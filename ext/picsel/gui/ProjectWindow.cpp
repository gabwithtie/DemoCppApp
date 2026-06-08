#include "ProjectWindow.h"
#include "picsel/project/ProjectManager.h" // Dispatched actions and data source routing
#include "system/FileDialogue.h"
#include "gui/features/directory/DirectoryBrowser.h" 
#include "imgui.h"

namespace picsel {

    ProjectWindow::ProjectWindow() {
        // Presentation component configuration initializations
    }

    void ProjectWindow::DrawSelf() {
        switch (m_view_state) {
        case ViewState::Landing:
            DrawLandingPage();
            break;
        case ViewState::Creating:
            DrawCreateProjectForm();
            break;
        case ViewState::ProjectDashboard:
            DrawActiveProjectDashboard();
            break;
        }
    }

    void ProjectWindow::DrawLandingPage() {
        ImVec2 center = ImGui::GetContentRegionAvail();

        ImGui::Text("Welcome to Picsel Animator!");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Create New Project Structure...", ImVec2(center.x, 40))) {
            m_view_state = ViewState::Creating;
        }

        ImGui::Spacing();

        if (ImGui::Button("Open Existing .picsel Project File...", ImVec2(center.x, 40))) {
            std::filesystem::path selected_file = app::FileDialogue::GetFilePath(app::FileDialogue::OpType::OPEN);

            if (!selected_file.empty()) {
                // UI forwards user interaction requests directly to the state manager
                if (ProjectManager::Get()->OpenProject(selected_file)) {
                    m_view_state = ViewState::ProjectDashboard;

                    // Automatically direct the workspace layout inspector context path
                    app::DirectoryBrowser::SetProjectDirectory(selected_file.parent_path());
                }
            }
        }
    }

    void ProjectWindow::DrawCreateProjectForm() {
        ImGui::Text("Setup New Workspace Directory");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::InputText("Project / Folder Name", m_buf_name, IM_ARRAYSIZE(m_buf_name));

        ImGui::Spacing();
        ImGui::Text("Target Base Location:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), m_selected_folder.empty() ? "[Select Root Location]" : m_selected_folder.string().c_str());

        if (ImGui::Button("Browse Directories...")) {
            std::filesystem::path folder = app::FileDialogue::GetFilePath(app::FileDialogue::OpType::FOLDER);
            if (!folder.empty()) {
                m_selected_folder = folder;
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        bool can_create = !m_selected_folder.empty() && (strlen(m_buf_name) > 0);

        if (!can_create) ImGui::BeginDisabled();
        if (ImGui::Button("Initialize Structure", ImVec2(160, 35))) {
            bool success = ProjectManager::Get()->Get()->CreateNewProject(m_selected_folder, m_buf_name, 32, 32);
            if (success) {
                std::filesystem::path expected_file = m_selected_folder / m_buf_name / "project.picsel";

                if (ProjectManager::Get()->OpenProject(expected_file)) {
                    m_view_state = ViewState::ProjectDashboard;
                    app::DirectoryBrowser::SetProjectDirectory(expected_file.parent_path());
                }
            }
        }
        if (!can_create) ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 35))) {
            m_view_state = ViewState::Landing;
        }
    }

    void ProjectWindow::DrawActiveProjectDashboard() {
        // Interrogate the logical core layer to determine current configuration safety states
        if (!ProjectManager::Get()->HasActiveProject()) {
            m_view_state = ViewState::Landing;
            return;
        }

        auto& project = ProjectManager::Get()->GetActiveProject().value();
        auto& project_path = ProjectManager::Get()->GetActiveProjectPath();

        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "● Active Workspace: %s", project.project_name.c_str());
        ImGui::TextDisabled("Manifest Location: %s", project_path.string().c_str());
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Refresh Browser Alignment", ImVec2(220, 30))) {
            app::DirectoryBrowser::SetProjectDirectory(project_path.parent_path());
        }

        ImGui::Spacing();

        if (ImGui::Button("Unload Workspace Profile", ImVec2(220, 30))) {
            ProjectManager::Get()->UnloadProject();
            m_view_state = ViewState::Landing;
        }
    }
}