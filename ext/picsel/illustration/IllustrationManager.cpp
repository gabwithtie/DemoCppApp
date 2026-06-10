#include "IllustrationManager.h"
#include <iostream>
#include <fstream>

#include <glaze/glaze.hpp>

#include "picsel/utility/PngWriter.h"
#include "asset/assetloading/BatchLoader.h"

namespace picsel {

    // Define static members
    std::filesystem::path IllustrationManager::m_activeFolder = "";
    std::filesystem::path IllustrationManager::m_layerDirectory = "";
    std::vector<IllustrationData> IllustrationManager::m_illustrations;
    IllustrationData* IllustrationManager::m_activeIllustration = nullptr;
    IllustrationManager::DestructorGuard IllustrationManager::m_guard;

    void IllustrationManager::initialize(const std::filesystem::path& folderPath) {
        // If changing folders, safely save old active work first
        if (m_activeIllustration) {
            saveActiveIllustration();
        }

        m_activeFolder = folderPath;
        m_layerDirectory = m_activeFolder / "layers";

        if (!std::filesystem::create_directories(m_layerDirectory)) {

        }

        m_illustrations.clear();
        m_activeIllustration = nullptr;

        if (std::filesystem::exists(m_activeFolder) && std::filesystem::is_directory(m_activeFolder)) {
            loadAllFromFolder();
        }
    }

    void IllustrationManager::shutdown() {
        if (m_activeIllustration) {
            std::cout << "[IllustrationManager] Shutdown triggered. Saving active illustration...\n";
            saveActiveIllustration();
            m_activeIllustration = nullptr;
        }
    }

    const std::vector<IllustrationData>& IllustrationManager::getAllIllustrations() {
        return m_illustrations;
    }

    IllustrationData* IllustrationManager::getActiveIllustration() {
        return m_activeIllustration;
    }

    void IllustrationManager::setActiveIllustration(size_t index) {
        if (index >= m_illustrations.size()) return;

        // Save current asset before swapping
        saveActiveIllustration();

        m_activeIllustration = &m_illustrations[index];
        std::cout << "[IllustrationManager] Active illustration changed to: " << m_activeIllustration->name << "\n";
    }

    void IllustrationManager::setActiveIllustration(const std::string _name) {
        for (size_t i = 0; i < m_illustrations.size(); ++i) {
            if (m_illustrations[i].name == _name) {
                setActiveIllustration(i);
                return;
            }
        }
    }

    void IllustrationManager::createNewIllustration(const std::string& filenameWithoutExtension) {
        if (m_activeFolder.empty()) return;

        std::filesystem::path newPath = m_activeFolder / (filenameWithoutExtension + ".illus");

        // Avoid overwriting existing files blindly
        if (std::filesystem::exists(newPath)) {
            std::cout << "[IllustrationManager] File already exists: " << newPath << "\n";
            setActiveIllustration(filenameWithoutExtension);
            return;
        }

        IllustrationData newIllus;
        newIllus.name = filenameWithoutExtension;
        // Optionally populate default values: newIllus.layerFilenames.push_back("background.png");

        // Write it out immediately to establish the file
        saveIllustration(newIllus);

        // Push into our static vector
        m_illustrations.push_back(newIllus);

        // Set as active (This will auto-save any prior active illustration)
        setActiveIllustration(m_illustrations.size() - 1);
    }

    void IllustrationManager::createNewLayerForActive(const std::string& layername)
    {
        getActiveIllustration()->layerFilenames.push_back(layername); //create data link
        auto layer_folder = m_layerDirectory / getActiveIllustration()->name;
        std::filesystem::create_directories(layer_folder);

        auto layerfilename = layer_folder / (layername + ".png");
        PngWriter::WriteEmptyRGBA(layerfilename, getActiveIllustration()->width, getActiveIllustration()->height);
        app::BatchLoader::ReloadDirectory(m_layerDirectory);
    }

    void IllustrationManager::saveActiveIllustration() {
        if (!m_activeIllustration) return;
        saveIllustration(*m_activeIllustration);
    }

    void IllustrationManager::loadAllFromFolder() {
        for (const auto& entry : std::filesystem::directory_iterator(m_activeFolder)) {
            if (entry.is_regular_file() && entry.path().extension() == ".illus") {

                std::string buffer;
                IllustrationData illus;
                // Glaze accepts a string buffer natively for incredibly fast parsing
                glz::read_file_json(illus, entry.path().string(), buffer);
                if (buffer.size() > 0) {
                    m_illustrations.push_back(illus);
                }
                else {
                    std::cerr << "[IllustrationManager] Failed to parse: " << entry.path() << "\n";
                }
            }
        }
        std::cout << "[IllustrationManager] Loaded " << m_illustrations.size() << " files from " << m_activeFolder << "\n";
    }

    void IllustrationManager::saveIllustration(const IllustrationData& illus) {
        std::string buffer;
        // Serialize structure to beautiful, readable JSON format
        glz::write_json(illus, buffer);

        // Write string buffer out to disk
        auto targetpath = m_activeFolder / (illus.name + ".illus");
        std::ofstream out(targetpath, std::ios::out | std::ios::trunc);
        if (out.is_open()) {
            out << buffer;
            std::cout << "[IllustrationManager] Saved: " << targetpath << "\n";
        }
    }

} // namespace picsel