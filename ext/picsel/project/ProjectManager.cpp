#include "ProjectManager.h"
#include <fstream>
#include <iostream>

#include "asset/assetloading/BatchLoader.h"

#include "picsel/illustration/IllustrationManager.h"

namespace picsel {

    ProjectManager* ProjectManager::s_instance = nullptr;

    ProjectManager::ProjectManager() {
        if (!s_instance) s_instance = this;
    }

    ProjectManager::~ProjectManager() {
        std::cout << "[Picsel] Application context shutting down. Initiating project auto-save layer...\n";
        if (HasActiveProject()) {
            bool saved = SaveActiveProject();
            if (saved) {
                std::cout << "[Picsel] Auto-save flushed successfully to: " << m_active_project_path.string() << "\n";
            }
            else {
                std::cerr << "[Picsel] Critical Error: Auto-save failed during destruction pass.\n";
            }
        }
        if (s_instance == this) s_instance = nullptr;
    }

    bool ProjectManager::OpenProject(const std::filesystem::path& project_file_path) {
        if (!std::filesystem::exists(project_file_path) || project_file_path.extension() != ".picsel") {
            std::cerr << "[Picsel] Target manifest file does not exist or has an invalid format.\n";
            return false;
        }

        // Auto-save any currently active project context before moving to the new one
        if (HasActiveProject()) {
            SaveActiveProject();
        }

        ProjectData loaded_data;
        std::string file_buffer;

        auto ec = glz::read_file_json < glz::opts{ .error_on_unknown_keys = false } > (
            loaded_data,
            project_file_path.string(),
            file_buffer
        );

        if (ec) {
            std::cerr << "[Picsel] Glaze parse exception occurred during project loading phase.\n";
            return false;
        }

        m_active_project = std::move(loaded_data);
        m_active_project_path = project_file_path;

        auto project_root = m_active_project_path.parent_path();
        std::filesystem::path illustrations_dir = project_root / "illustrations";
        std::filesystem::create_directories(illustrations_dir);

        // Other directory initializers
        app::BatchLoader::ReloadDirectory(project_root);
        IllustrationManager::initialize(illustrations_dir);

        return true;
    }

    void ProjectManager::UnloadProject() {
        if (HasActiveProject()) {
            SaveActiveProject(); // Save changes before unbinding state context references
        }
        m_active_project = std::nullopt;
        m_active_project_path.clear();
    }

    bool ProjectManager::SaveActiveProject() {
        if (!HasActiveProject()) return false;

        std::string json_buffer;
        glz::write_json(m_active_project.value(), json_buffer);

        std::ofstream file(m_active_project_path);
        if (!file.is_open()) return false;

        file << json_buffer;
        file.close();
        return true;
    }

    bool ProjectManager::CreateNewProject(const std::filesystem::path& root_dir, const std::string& project_name, int width, int height) {
        try {
            std::filesystem::path project_root = root_dir / project_name;
            
            if (!std::filesystem::exists(project_root)) {
                std::cerr << "[Picsel] System execution mapping error: Directory could not be mounted.\n";
                return false;
            }

            ProjectData new_project;
            new_project.project_name = project_name;

            std::filesystem::path config_file = project_root / "project.picsel";
            std::string json_buffer;
            glz::write_json(new_project, json_buffer);

            std::ofstream file(config_file);
            if (!file.is_open()) return false;

            file << json_buffer;
            file.close();
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "[Picsel] Exception occurred while initializing project structure: " << e.what() << "\n";
            return false;
        }
    }
}