#include "AnimationBrowser.h"
#include "picsel/animation/AnimationManager.h"
#include "picsel/project/ProjectManager.h" // Fetch active project state
#include "imgui.h"

namespace picsel {

    void AnimationBrowser::DrawSelf() {
        // Tie the browser to the active project's file structure
        auto active_proj_path = ProjectManager::Get()->GetActiveProjectPath();
        if (active_proj_path.empty()) {
            ImGui::TextDisabled("No active project loaded.");
            return;
        }

        m_current_directory = active_proj_path.parent_path() / "animations";

        // Ensure the directory exists
        if (!std::filesystem::exists(m_current_directory)) {
            std::filesystem::create_directories(m_current_directory);
        }

        // Toggle button for the creation form
        if (ImGui::Button(m_show_creator ? "Cancel Creation" : "Create New Clip", ImVec2(-1, 30))) {
            m_show_creator = !m_show_creator;
        }

        ImGui::Separator();
        ImGui::Spacing();

        if (m_show_creator) {
            DrawCreatorForm();
        }
        else {
            DrawFileList();
        }
    }

    void AnimationBrowser::DrawCreatorForm() {
        ImGui::Text("New Animation Setup");
        ImGui::Spacing();

        ImGui::InputText("Clip Name", m_buf_name, IM_ARRAYSIZE(m_buf_name));
        ImGui::InputInt2("Size (W x H)", m_buf_size);

        // Clamp sizes to valid minimums
        m_buf_size[0] = std::max(1, m_buf_size[0]);
        m_buf_size[1] = std::max(1, m_buf_size[1]);

        ImGui::Spacing();
        if (ImGui::Button("Confirm Creation", ImVec2(-1, 30))) {
            if (strlen(m_buf_name) > 0) {
                AnimationManager::CreateNewClip(m_current_directory, m_buf_name, m_buf_size[0], m_buf_size[1]);
                m_show_creator = false; // Close form and return to the list view
            }
        }
    }

    void AnimationBrowser::DrawFileList() {
        ImGui::Text("Available Clips:");
        ImGui::Spacing();

        std::string active_path = AnimationManager::GetActiveClipPath();

        // Iterate through all .anim files in the directory
        for (const auto& entry : std::filesystem::directory_iterator(m_current_directory)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".anim") continue;

            std::string filename = entry.path().filename().string();
            bool is_selected = (entry.path().string() == active_path);

            if (ImGui::Selectable(filename.c_str(), is_selected)) {
                AnimationManager::LoadClip(entry.path());
            }

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Click to load into the Animation Window");
            }
        }
    }
}