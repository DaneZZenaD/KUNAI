#include "KUNAI/DEX/parser/dex_parser.hpp"

namespace KUNAI
{
    namespace DEX
    {
        DexParser::DexParser() : api_version(0)
        {
        }

        void DexParser::parse_dex_file(std::ifstream &input_file, std::uint64_t file_size)
        {
            auto logger = LOGGER::logger();

            logger->info("DexParser start parsing dex file with a size of {} bytes", file_size);

            std::uint8_t header[8];

            if (file_size < sizeof(DexHeader::dexheader_t))
                throw exceptions::IncorrectDexFile("File is not a DEX file");

            if (!KUNAI::read_data_file<std::uint8_t[8]>(header, sizeof(std::uint8_t[8]), input_file))
                throw exceptions::ParserReadingException("Error parsing header");

            // quick check
            // big check should be done in detector module
            if (!memcmp(header, dex_magic_035, 8) &&
                !memcmp(header, dex_magic_037, 8) &&
                !memcmp(header, dex_magic_038, 8) &&
                !memcmp(header, dex_magic_039, 8))
                throw exceptions::IncorrectDexFile("Error dex magic not recognized");

            input_file.seekg(0);

#ifdef DEBUG
            logger->debug("Checks correct");
#endif

            logger->info("Starting DEX headers parsing");

            dex_header = std::make_unique<DexHeader>(input_file, file_size);
            dex_strings = std::make_unique<DexStrings>(input_file, file_size, dex_header->get_dex_header().string_ids_size, dex_header->get_dex_header().string_ids_off);
            dex_types = std::make_unique<DexTypes>(input_file, dex_header->get_dex_header().type_ids_size, dex_header->get_dex_header().type_ids_off, dex_strings.get());
            dex_protos = std::make_unique<DexProtos>(input_file, file_size, dex_header->get_dex_header().proto_ids_size, dex_header->get_dex_header().proto_ids_off, dex_strings.get(), dex_types.get());
            dex_fields = std::make_unique<DexFields>(input_file, dex_header->get_dex_header().field_ids_size, dex_header->get_dex_header().field_ids_off, dex_strings.get(), dex_types.get());
            dex_methods = std::make_unique<DexMethods>(input_file, dex_header->get_dex_header().method_ids_size, dex_header->get_dex_header().method_ids_off, dex_strings.get(), dex_types.get(), dex_protos.get());
            dex_classes = std::make_unique<DexClasses>(input_file, file_size, dex_header->get_dex_header().class_defs_size, dex_header->get_dex_header().class_defs_off, dex_strings.get(), dex_types.get(), dex_fields.get(), dex_methods.get());

            retrieve_encoded_fields_from_classes();

            logger->info("Finished DEX headers parsing");
        }

        std::uint32_t DexParser::get_header_version()
        {
            if (memcmp(this->dex_header->get_dex_header().magic, dex_magic_035, 8))
                return 35;
            else if (memcmp(this->dex_header->get_dex_header().magic, dex_magic_037, 8))
                return 37;
            else if (memcmp(this->dex_header->get_dex_header().magic, dex_magic_038, 8))
                return 38;
            else if (memcmp(this->dex_header->get_dex_header().magic, dex_magic_039, 8))
                return 39;
            else
                return 0;
        }

        std::string DexParser::get_header_version_str()
        {
            std::string version = "";

            if (memcmp(this->dex_header->get_dex_header().magic, dex_magic_035, 8))
                version = "DEX_VERSION_35";
            else if (memcmp(this->dex_header->get_dex_header().magic, dex_magic_037, 8))
                version = "DEX_VERSION_37";
            else if (memcmp(this->dex_header->get_dex_header().magic, dex_magic_038, 8))
                version = "DEX_VERSION_38";
            else if (memcmp(this->dex_header->get_dex_header().magic, dex_magic_039, 8))
                version = "DEX_VERSION_39";

            return version;
        }

        std::vector<CodeItemStruct*> DexParser::get_codes_item()
        {
            std::vector<CodeItemStruct*> codes;

            for (size_t i = 0; i < dex_classes->get_number_of_classes(); i++)
            {
                auto class_def = dex_classes->get_class_by_pos(i);

                auto class_data_item = class_def->get_class_data();

                for (size_t j = 0; j < class_data_item->get_number_of_direct_methods(); j++)
                {
                    auto direct_method = class_data_item->get_direct_method_by_pos(j);

                    auto code_item_struct = direct_method->get_code_item();

                    codes.push_back(code_item_struct);
                }
            }

            return codes;
        }

        std::vector<std::string> DexParser::get_string_values()
        {
            std::vector<std::string> strings;

            for (size_t i = 0; i < dex_strings->get_number_of_strings(); i++)
            {
                strings.push_back(*dex_strings->get_string_from_order(i));
            }

            return strings;
        }

        void DexParser::retrieve_encoded_fields_from_classes()
        {
            auto & classes_def = dex_classes->get_classes();

            for (auto & c : classes_def)
            {
                if (!c->get_class_data())
                    continue;

                auto fields = c->get_class_data()->get_fields();

                if (fields.empty())
                    continue;

                for (auto f : fields)
                {
                    encoded_fields.push_back(f);
                }
            }
        }

        std::ostream &operator<<(std::ostream &os, const DexParser &entry)
        {
            if (entry.dex_header)
                os << *entry.dex_header;
            if (entry.dex_strings)
                os << *entry.dex_strings;
            if (entry.dex_types)
                os << *entry.dex_types;
            if (entry.dex_protos)
                os << *entry.dex_protos;
            if (entry.dex_fields)
                os << *entry.dex_fields;
            if (entry.dex_methods)
                os << *entry.dex_methods;
            if (entry.dex_classes)
                os << *entry.dex_classes;

            return os;
        }

    }
}