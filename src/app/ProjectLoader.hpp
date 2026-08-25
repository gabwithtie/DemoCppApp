#pragma once

#include <filesystem>
#include <algorithm>
#include <cstdlib>
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

    static inline std::filesystem::path GetRecentProjectsFile() {
        if (const char* appData = std::getenv("APPDATA"); appData != nullptr && *appData != '\0') {
            return std::filesystem::path(appData) / "GabApp" / "recent-projects.txt";
        }
        return std::filesystem::current_path() / "recent-projects.txt";
    }

    static inline void AddRecentProject(const std::filesystem::path& path) {
        std::error_code error;
        const auto absolutePath = std::filesystem::weakly_canonical(path, error);
        const auto normalizedPath = error ? std::filesystem::absolute(path) : absolutePath;

        auto recentProjects = GetRecentProjects();
        recentProjects.erase(
            std::remove(recentProjects.begin(), recentProjects.end(), normalizedPath),
            recentProjects.end());
        recentProjects.insert(recentProjects.begin(), normalizedPath);
        if (recentProjects.size() > 10) {
            recentProjects.resize(10);
        }

        const auto recentFile = GetRecentProjectsFile();
        std::filesystem::create_directories(recentFile.parent_path(), error);
        std::ofstream output(recentFile);
        for (const auto& recentProject : recentProjects) {
            output << recentProject.string() << '\n';
        }
    }

public:
    inline static std::filesystem::path GetCurrentProjectDir() { return currentProjectDir; }
    inline static std::filesystem::path GetCurrentSceneFile() { return currentSceneFile; }
    inline static std::filesystem::path GetCurrentProjectFile() { return currentProjectFile; }

    inline static std::filesystem::path GetAbsolutePath(const std::filesystem::path& relativePath) {
        return std::filesystem::absolute(currentProjectDir / relativePath);
    }

    static inline std::vector<std::filesystem::path> GetRecentProjects() {
        std::vector<std::filesystem::path> recentProjects;
        std::ifstream input(GetRecentProjectsFile());
        std::string line;
        while (std::getline(input, line)) {
            if (!line.empty()) {
                recentProjects.emplace_back(line);
            }
        }
        return recentProjects;
    }

    static inline void StartNewProject() {
        currentProjectDir.clear();
        currentSceneFile.clear();
        currentProjectFile.clear();
        App::GetInstance().project = Model::Project{};
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

        AddRecentProject(path);
    }

    static inline void SaveProject(const std::filesystem::path& path) {
        currentProjectFile = path;
        currentProjectDir = path.parent_path();

        App::GetInstance().SerializeToFile(path);
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