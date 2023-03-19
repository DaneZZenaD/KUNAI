#include "KUNAI/APK/apk.hpp"

namespace KUNAI
{
    namespace APK
    {

        APK::APK(std::string path_to_apk_file, bool create_xrefs) : path_to_apk_file(path_to_apk_file), create_xrefs(create_xrefs)
        {
            // get a temporal path where to dump files
            // temporal_path = std::tmpnam(nullptr);
            temporal_path = std::filesystem::temp_directory_path().c_str();
            temporal_path += std::filesystem::path::preferred_separator;
            temporal_path += std::filesystem::path(path_to_apk_file).stem();
            global_disassembler = nullptr;

            std::filesystem::create_directory(temporal_path.c_str());
        }

        APK::~APK()
        {
            // we will remove files from the temporal path
            for (const auto &file : std::filesystem::directory_iterator(temporal_path))
                remove(file.path().c_str());
            remove(temporal_path.c_str());
        }

        void APK::analyze_apk_file()
        {
            auto logger = LOGGER::logger();

            // try opening the file with library
            struct zip_t *zip = zip_open(path_to_apk_file.c_str(), 0, 'r');

            if (!zip)
            {
                logger->error("There was an error opening apk file.");
                throw exceptions::ApkUnzipException("Exception opening apk file");
            }

            // go over each entry
            for (int i = 0, n = zip_entries_total(zip); i < n; i++)
            {
                zip_entry_openbyindex(zip, i);
                std::string file_name = zip_entry_name(zip);

                // in case it contains .dex, work with it
                if (file_name.find(".dex") != std::string::npos && file_name.find("classes") != std::string::npos)
                {
                    auto dex = manage_dex_files_from_zip_entry(zip);

                    if (dex == nullptr)
                        throw exceptions::ApkUnzipException("Exception extracting dex file");

                    dex_files[file_name] = std::move(dex);
                }
                zip_entry_close(zip);
            }

            zip_close(zip);

            logger->info("Adding disassemblers to global disassembler");

            // create a disassembler with all the objects
            for (auto &dex_file : dex_files)
            {
                if (global_disassembler == nullptr)
                    // initialize the global disassembler with the first disassembler.
                    global_disassembler = dex_file.second->get_dex_disassembler();
                else
                    // then add the others
                    global_disassembler->add_disassembly(dex_file.second->get_dex_disassembler());
            }

            logger->info("Finished adding disassemblers to global disassembler");

            global_analysis = std::make_unique<DEX::Analysis>(nullptr,
                                                              dex_files.begin()->second->get_dalvik_opcode_object(),
                                                              global_disassembler, this->create_xrefs);

            logger->info("Adding DEX files to analysis object");

            for (auto &dex_file : dex_files)
                global_analysis->add(dex_file.second->get_parser());

            logger->info("Finished adding DEX files to analysis object");

            logger->info("Starting xrefs analysis");

            global_analysis->create_xref();

            logger->info("Finished creating analysis xrefs");
        }

        std::unique_ptr<DEX::DEX> APK::manage_dex_files_from_zip_entry(struct zip_t *dex_file)
        {
            // get logger object
            auto logger = LOGGER::logger();

            std::string temporal_file_path;
            std::ifstream file;
            const char *name = zip_entry_name(dex_file);

            // create the path to it
            temporal_file_path = temporal_path +
                                 std::filesystem::path::preferred_separator +
                                 std::string(name);

            // first of all, let's try to unzip the file
            logger->info("Extracting {} DEX file", name);

            auto error = zip_entry_fread(dex_file, temporal_file_path.c_str());

            if (error)
            {
                logger->error("There was a problem extracting '{}', error message: {}", name, zip_strerror(error));
                return nullptr;
            }

            // get file size
            auto fsize = zip_entry_size(dex_file);

            logger->info("Extracted DEX file with size {}", fsize);

            file.open(temporal_file_path, std::ios::binary);

            // Create DEX object
            auto dex = KUNAI::DEX::get_unique_dex_object(file, fsize);

            if (!dex->get_parsing_correct())
            {
                logger->error("There was a problem creating DEX object, parsing was not correct");
                return nullptr;
            }

            // Now disassemble the DEX file
            logger->info("Disassembling DEX file");
            auto disassembler = dex->get_dex_disassembler();
            disassembler->disassembly_analysis();
            logger->info("Finished disassembly of DEX file");

            if (!disassembler->get_disassembly_correct())
            {
                logger->error("There was a problem creating DEX object, disassembly was not correct");
                return nullptr;
            }

            logger->info("Finished extracting {} DEX file", name);

            return dex;
        }

        std::unique_ptr<APK> get_unique_apk_object(std::string path_to_apk_file, bool create_xrefs)
        {
            return std::make_unique<APK>(path_to_apk_file, create_xrefs);
        }

        apk_t get_shared_apk_object(std::string path_to_apk_file, bool create_xrefs)
        {
            return std::make_shared<APK>(path_to_apk_file, create_xrefs);
        }
    }
}