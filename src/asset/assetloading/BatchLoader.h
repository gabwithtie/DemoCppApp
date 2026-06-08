#pragma once

#include "../BaseAsset.h"

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "../types/Texture.h"
#include "system/Parser.h"

namespace fs = std::filesystem;

namespace app {
    class BatchLoader {
    private:
        // Recursively finds all file paths within a given directory and its subdirectories.
        inline static void get_all_filepaths(const fs::path& directory_path, std::vector<fs::path>& filepaths) {
            try {
                // Iterate through all entries (files and subdirectories) in the given directory.
                for (const auto& entry : fs::directory_iterator(directory_path)) {
                    // Check if the current entry is a regular file.
                    if (fs::is_regular_file(entry.status())) {
                        // If it's a file, add its path to our vector.
                        filepaths.push_back(entry.path());
                    }
                    // Check if the current entry is a directory.
                    else if (fs::is_directory(entry.status())) {
                        // If it's a directory, recursively call the function on it.
                        get_all_filepaths(entry.path(), filepaths);
                    }
                }
            }
            catch (const fs::filesystem_error& e) {
                // Handle potential errors, such as permission denied.
                std::cerr << "Error accessing directory " << directory_path << ": " << e.what() << std::endl;
            }
        }
        inline static bool is_file_extension(const std::string& filename, const std::string& extension) {
            // If the filename is shorter than the extension, it can't possibly match.
            if (filename.length() < extension.length()) {
                return false;
            }

            // Compare the end of the filename with the extension.
            return filename.compare(filename.length() - extension.length(), extension.length(), extension) == 0;
        }
    public:
        inline static void GenerateMetafiles(std::filesystem::path _directory) {
            std::vector<fs::path> filepaths;
            get_all_filepaths(_directory, filepaths);

            for (size_t i = 0; i < filepaths.size(); i++)
            {
                const auto& filepath = filepaths[i];
                const auto& directory = filepath.parent_path();
                const auto& filename_ext = filepath.filename().string();
                const auto& filename_only = filepath.stem().string();

                if (is_file_extension(filename_ext, ".obj.gbe")) {
                    std::filesystem::remove_all(filepath);
                }
                if (is_file_extension(filename_ext, ".img.gbe")) {
                    std::filesystem::remove_all(filepath);
                }
            }

            filepaths.clear();
            get_all_filepaths(_directory, filepaths);

            for (size_t i = 0; i < filepaths.size(); i++)
            {
                const auto& filepath = filepaths[i];
                const auto& directory = filepath.parent_path();
                const auto& filename_ext = filepath.filename().string();
                const auto& filename_only = filepath.stem().string();

                if (is_file_extension(filename_ext, ".png") || is_file_extension(filename_ext, ".jpg")) {
                    auto newdata = app::TextureImportData{
                        .path = filename_ext
                    };
                    const auto& meta_filename = filename_only + ".img.gbe";

                    app::Parser::ExportClass(newdata, directory / meta_filename);
                }
            }
        }

        ///returns: Loaded Filenames
        inline static std::vector<std::string> LoadAssetsFromDirectory(std::filesystem::path directory) {
            std::vector<fs::path> filepaths;
            get_all_filepaths(directory, filepaths);

            std::vector<fs::path> filepaths_material;

            std::vector<std::string> filenames;

            for (size_t i = 0; i < filepaths.size(); i++)
            {
                const auto& filepath = filepaths[i];
                const auto& filename = filepath.filename().string();

                if (is_file_extension(filename, ".img.gbe")) {
                    std::cout << "[BATCHLOADER] Loading Texture: \"" << filepath << "\"" << std::endl;
                    auto newtex = new Texture(filepath);
                    filenames.push_back(newtex->Get_assetId());
                }
                else if (is_file_extension(filename, ".gbe")) {
                    std::cout << "[BATCHLOADER] Unknown Asset Type in: \"" << filepath << "\"" << std::endl;
                }
            }

            //Wait here for all async tasks to finish
            bool batchload_done = false;
            while (!batchload_done)
            {
                batchload_done = true;

                for (const auto& lpair : app::all_asset_loaders)
                {
                    const auto& loader = lpair.second;

                    if (loader->CheckAsynchrounousTasks() > 0) {
                        batchload_done = false;
                    }
                }
            }

            return filenames;
        }

        static void ReloadDirectory(std::filesystem::path directory);
    };
}