#pragma once

#include <filesystem>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include "File/Parser.hpp"
#include "App.hpp"

namespace gsr {

class ProjectLoader {
    struct ProjectInfo {
        std::string entryscene;
    };

    inline static std::filesystem::path currentProjectDir;
    inline static std::filesystem::path currentSceneFile;
    inline static std::filesystem::path currentProjectFile;
    inline static bool projectOpen = false;

public:
    inline static std::filesystem::path GetCurrentProjectDir() { return currentProjectDir; }
    inline static std::filesystem::path GetCurrentSceneFile() { return currentSceneFile; }
    inline static std::filesystem::path GetCurrentProjectFile() { return currentProjectFile; }
    inline static bool IsProjectOpen() { return projectOpen; }

    inline static std::filesystem::path GetAbsolutePath(const std::filesystem::path& relativePath) {
        return std::filesystem::absolute(currentProjectDir / relativePath);
    }

    static inline void LoadProject(const std::filesystem::path& path) {
        ProjectInfo newinfo;

        if (gbe::Parser::PopulateClass(newinfo, path) && !newinfo.entryscene.empty()) {
            currentProjectDir = path.parent_path();
            currentSceneFile = currentProjectDir / newinfo.entryscene;
            currentProjectFile = path;

            App::GetInstance().DeserializeFromFile(currentSceneFile);
        } else {
            currentProjectDir = path.parent_path();
            currentSceneFile = path;
            currentProjectFile = path;

            App::GetInstance().DeserializeFromFile(path);
        }
        projectOpen = true;
    }

    static inline void StartNewProject() {
        currentProjectDir.clear();
        currentSceneFile.clear();
        currentProjectFile.clear();
        App::GetInstance().project = Model::Project{};
        projectOpen = true;
    }

    static inline void SaveProject(const std::filesystem::path& path) {
        currentProjectFile = path;
        currentProjectDir = path.parent_path();

        App::GetInstance().SerializeToFile(path);
        projectOpen = true;
    }

    static inline bool QuickSave() {
        if (!currentSceneFile.empty()) {
            SaveProject(currentSceneFile);
            return true;
        }
        if (!currentProjectFile.empty()) {
            SaveProject(currentProjectFile);
            return true;
        }
        return false;
    }
};

} // namespace gsr