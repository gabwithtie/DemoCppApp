#include "ProjectPicker.hpp"

#include "App.hpp"
#include "ProjectLoader.hpp"
#include "FileDialogue.hpp"

#include <filesystem>
#include <imgui.h>

namespace gsr::gui {

bool ProjectPicker::Draw() {
    if (finished) {
        return true;
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(520.0f, 360.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin("Open Recent Projects", nullptr, ImGuiWindowFlags_NoCollapse);

    if (ImGui::Button("Start New Project", ImVec2(-1.0f, 0.0f))) {
        ProjectLoader::StartNewProject();
        finished = true;
    }

    if (ImGui::Button("Open Project File...", ImVec2(-1.0f, 0.0f))) {
        const auto path = gbe::FileDialogue::GetFilePath(gbe::FileDialogue::OPEN, "gsrproj");
        if (!path.empty()) {
            ProjectLoader::LoadProject(path);
            finished = true;
        }
    }

    ImGui::SeparatorText("Recent Projects");
    const auto recentProjects = ProjectLoader::GetRecentProjects();
    if (recentProjects.empty()) {
        ImGui::TextDisabled("No recent projects");
    } else {
        for (const auto& path : recentProjects) {
            if (!std::filesystem::exists(path)) {
                continue;
            }
            if (ImGui::Selectable(path.string().c_str())) {
                ProjectLoader::LoadProject(path);
                finished = true;
            }
        }
    }

    ImGui::End();
    return finished;
}

} // namespace gsr::gui