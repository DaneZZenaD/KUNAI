#include "KUNAI/DEX/parser/dex_protos.hpp"

namespace KUNAI
{
    namespace DEX
    {

        /***
         * ProtoID class
         */
        ProtoID::ProtoID(std::uint32_t shorty_idx,
                         std::uint32_t return_type_idx,
                         std::uint32_t parameters_off,
                         std::ifstream &input_file,
                         DexStrings *dex_strings,
                         DexTypes *dex_types) : parameters_off(parameters_off)
        {
            this->shorty_idx = dex_strings->get_string_from_order(shorty_idx);
            this->return_type_idx = dex_types->get_type_from_order(return_type_idx);
            if (!parse_parameters(input_file, dex_strings, dex_types))
                throw exceptions::ParserReadingException("Error reading ProtoID");
        }

        bool ProtoID::parse_parameters(std::ifstream &input_file,
                                       DexStrings *dex_strings,
                                       DexTypes *dex_types)
        {
            auto logger = LOGGER::logger();

            auto current_offset = input_file.tellg();
            std::uint32_t size;
            std::uint16_t type_id;
            Type *type;
            size_t i = 0;

            if (parameters_off == 0)
                return true;

            // move to parameters offset
            input_file.seekg(parameters_off);

            if (!KUNAI::read_data_file<std::uint32_t>(size, sizeof(std::uint32_t), input_file))
                return false;

#ifdef DEBUG
            logger->debug("Reading ProtoID from offset {} and size {}", parameters_off, size);
#endif

            auto num_types = dex_types->get_number_of_types();

            for (i = 0; i < size; i++)
            {
                if (!KUNAI::read_data_file<std::uint16_t>(type_id, sizeof(std::uint16_t), input_file))
                    return false;

                if (type_id > num_types)
                {
                    logger->error("Error reading ProtoID parameters type_id out of type bound ({} > {})", type_id, num_types);
                    throw exceptions::IncorrectTypeId("Error reading ProtoID parameters type_id out of type bound");
                }

                type = dex_types->get_type_from_order(type_id);

                parameters.push_back(type);

#ifdef DEBUG
                logger->debug("Parsed type number {}", i);
#endif
            }

            input_file.seekg(current_offset);
            return true;
        }

        Type *ProtoID::get_parameter_type_by_order(size_t pos)
        {
            if (pos >= parameters.size())
                return nullptr;

            return parameters[pos];
        }

        std::string ProtoID::get_proto_str()
        {
            std::string proto = "";

            proto += "(";

            for (size_t i = 0; i < parameters.size(); i++)
            {
                proto += parameters[i]->get_raw();
            }

            proto += ")" + return_type_idx->get_raw();

            return proto;
        }

        /***
         * DexProtos class
         */
        DexProtos::DexProtos(std::ifstream &input_file,
                             std::uint64_t file_size,
                             std::uint32_t number_of_protos,
                             std::uint32_t offset,
                             DexStrings *dex_strings,
                             DexTypes *dex_types) : number_of_protos(number_of_protos),
                                                    offset(offset),
                                                    dex_strings(dex_strings),
                                                    dex_types(dex_types)
        {
            if (!parse_protos(input_file, file_size))
                throw exceptions::ParserReadingException("Error reading DEX protos");
        }

        bool DexProtos::parse_protos(std::ifstream &input_file, std::uint64_t file_size)
        {
            auto logger = LOGGER::logger();

            protoid_t proto_id;
            auto current_offset = input_file.tellg();
            size_t i = 0;
            std::uint32_t shorty_idx = 0, return_type_idx = 0, parameters_off = 0;

            // set to current offset
            input_file.seekg(offset);

#ifdef DEBUG
            logger->debug("DexProtos start parsing in offset {} with size {}", offset, number_of_protos);
#endif

            auto number_of_strings = dex_strings->get_number_of_strings();
            auto number_of_types = dex_types->get_number_of_types();

            for (i = 0; i < number_of_protos; i++)
            {
                if (!KUNAI::read_data_file<std::uint32_t>(shorty_idx, sizeof(std::uint32_t), input_file))
                    return false;

                if (shorty_idx >= number_of_strings)
                {
                    logger->error("Error reading protos short_idx out of string bound ({} >= {})", shorty_idx, number_of_strings);
                    throw exceptions::IncorrectStringId("Error reading protos short_idx out of string bound");
                }

                if (!KUNAI::read_data_file<std::uint32_t>(return_type_idx, sizeof(std::uint32_t), input_file))
                    return false;

                if (return_type_idx >= number_of_types)
                {
                    logger->error("Error reading protos return_type_idx out of type bound ({} >= {})", return_type_idx, number_of_types);
                    throw exceptions::IncorrectTypeId("Error reading protos return_type_idx out of type bound");
                }

                if (!KUNAI::read_data_file<std::uint32_t>(parameters_off, sizeof(std::uint32_t), input_file))
                    return false;

                if (parameters_off > file_size)
                {
                    logger->error("Error reading protos parameters_off out of file bound ({} > {})", parameters_off, file_size);
                    throw exceptions::OutOfBoundException("Error reading protos parameters_off out of file bound");
                }

                proto_id = std::make_unique<ProtoID>(shorty_idx, return_type_idx, parameters_off, input_file, dex_strings, dex_types);

                proto_ids.push_back(std::move(proto_id));

#ifdef DEBUG
                logger->debug("parsed proto id {}", i);
#endif
            }

            // set to previous offset
            input_file.seekg(current_offset);

            logger->info("DexProtos parsing correct");

            return true;
        }

        ProtoID* DexProtos::get_proto_by_order(size_t pos)
        {
            if (pos >= proto_ids.size())
                return nullptr;

            return proto_ids[pos].get();
        }

        std::ostream &operator<<(std::ostream &os, const DexProtos &entry)
        {
            size_t i = 0;
            os << std::hex;
            os << std::setw(30) << std::left << std::setfill(' ') << "=========== DEX Protos ==========="
               << "\n";

            for (const auto & proto_id : entry.proto_ids)
            {
                os << std::left << std::setfill(' ') << "Proto (" << std::dec << i++ << std::hex << "): ";
                os << "(";
                for (size_t j = 0; j < proto_id->get_number_of_parameters(); j++)
                {
                    os << proto_id->get_parameter_type_by_order(j)->get_raw();
                    if (j != proto_id->get_number_of_parameters() - 1)
                        os << ", ";
                }
                os << ")" << proto_id->get_return_idx()->get_raw();
                os << "\n";
            }

            return os;
        }

        std::fstream &operator<<(std::fstream &fos, const DexProtos &entry)
        {
            std::stringstream stream;

            stream << std::hex;
            stream << std::setw(30) << std::left << std::setfill(' ') << "<protos>"
                   << "\n";

            for (const auto & proto_id : entry.proto_ids)
            {
                stream << std::left << std::setfill(' ') << "\t<proto>"
                       << "\n";
                stream << "\t\t<arguments>"
                       << "\n";
                for (size_t j = 0; j < proto_id->get_number_of_parameters(); j++)
                {
                    stream << "\t\t\t<argument>" << proto_id->get_parameter_type_by_order(j)->get_raw() << "</argument>"
                           << "\n";
                }
                stream << "\t\t</arguments>"
                       << "\n";
                stream << "\t\t<return>" << proto_id->get_return_idx()->get_raw() << "</return>"
                       << "\n";
                stream << std::left << std::setfill(' ') << "\t</proto>"
                       << "\n";
            }

            stream << std::setw(30) << std::left << std::setfill(' ') << "</protos>"
                   << "\n";

            fos.write(stream.str().c_str(), stream.str().size());

            return fos;
        }
    }
}