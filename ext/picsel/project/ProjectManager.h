#pragma once

#include "ProjectData.h"
#include <glaze/glaze.hpp>
#include <filesystem>
#include <optional>

namespace picsel {

    class ProjectManager {
    public:
        ProjectManager();
        ~ProjectManager(); // Master Lifecycle Engine Hook for Auto-Saving

        static ProjectManager* Get() { return s_instance; }

        // --- Context State Interrogation Accessors ---
        std::optional<ProjectData>& GetActiveProject() { return m_active_project; }
        const std::filesystem::path& GetActiveProjectPath() const { return m_active_project_path; }
        bool HasActiveProject() const { return m_active_project.has_value(); }

        // --- Active Workspace Operations Pipeline ---
        bool OpenProject(const std::filesystem::path& project_file_path);
        void UnloadProject();
        bool SaveActiveProject();

        bool CreateNewProject(const std::filesystem::path& root_dir, const std::string& project_name, int width, int height);

    private:
        static ProjectManager* s_instance;

        std::optional<ProjectData> m_active_project = std::nullopt;
        std::filesystem::path m_active_project_path;
    };
}