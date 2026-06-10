#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

#include "IllustrationData.h"

namespace picsel {

    class IllustrationManager {
    public:

        // Core API
        static void initialize(const std::filesystem::path& folderPath);
        static void shutdown(); // Call this or let RAII handle it on program exit via static destruction

        static const std::vector<IllustrationData>& getAllIllustrations();

        static IllustrationData* getActiveIllustration();
        static void setActiveIllustration(size_t index);
        static void setActiveIllustration(const std::string _name);

        static void createNewIllustration(const std::string& filenameWithoutExtension);
        static void createNewLayerForActive(const std::string& layername);
        static void saveActiveIllustration();

    private:
        static void loadAllFromFolder();
        static void saveIllustration(const IllustrationData& illus);

        // Static State
        static std::filesystem::path m_activeFolder;
        static std::filesystem::path m_layerDirectory;
        static std::vector<IllustrationData> m_illustrations;
        static IllustrationData* m_activeIllustration;

        // Helper class to guarantee saving on destruction via RAII
        struct DestructorGuard {
            ~DestructorGuard() { IllustrationManager::shutdown(); }
        };
        static DestructorGuard m_guard;
    };

} // namespace picsel