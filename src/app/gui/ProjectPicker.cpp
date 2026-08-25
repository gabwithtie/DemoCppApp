#include "ProjectPicker.h"

#include "../ProjectLoader.hpp"
#include "FileDialogue.hpp"

#include <cstdlib>
#include <fstream>
#include <imgui.h>

namespace gsr {

ProjectPicker::ProjectPicker() {
    LoadRecentProjects();
}

std::filesystem::path ProjectPicker::RecentProjectsFile() {
    const char* appData = std::getenv("APPDATA");
    const auto base = appData ? std::filesystem::path(appData) : std::filesystem::current_path();
    return base / "DemoCppApp" / "recent-projects.txt";
}

void ProjectPicker::LoadRecentProjects() {
    std::ifstream file(RecentProjectsFile());
    std::string line;
    while (std::getline(file, line)) {
        std::filesystem::path path(line);
        if (!path.empty() && std::filesystem::exists(path))
            recentProjects.push_back(path);
    }
}

void ProjectPicker::SaveRecentProject(const std::filesystem::path& path) {
    recentProjects.erase(std::remove(recentProjects.begin(), recentProjects.end(), path), recentProjects.end());
    recentProjects.insert(recentProjects.begin(), path);
    if (recentProjects.size() > 8)
        recentProjects.resize(8);

    const auto filePath = RecentProjectsFile();
    std::filesystem::create_directories(filePath.parent_path());
    std::ofstream file(filePath, std::ios::trunc);
    for (const auto& recent : recentProjects)
        file << recent.string() << '\n';
}

void ProjectPicker::OpenProject(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path))
        return;
    ProjectLoader::LoadProject(path);
    SaveRecentProject(path);
    SetOpen(false);
}

void ProjectPicker::DrawSelf() {
    ImGui::TextUnformatted("Choose a project to begin");
    ImGui::Spacing();

    if (ImGui::Button("Open Project...")) {
        const auto path = gbe::FileDialogue::GetFilePath(gbe::FileDialogue::OPEN, "gsrproj");
        if (!path.empty())
            OpenProject(path);
    }
    ImGui::SameLine();
    if (ImGui::Button("New Empty Project")) {
        ProjectLoader::StartNewProject();
        SetOpen(false);
    }

    if (!recentProjects.empty()) {
        ImGui::SeparatorText("Recent projects");
        for (const auto& path : recentProjects) {
            ImGui::PushID(path.string().c_str());
            if (ImGui::Selectable(path.filename().string().c_str()))
                OpenProject(path);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", path.string().c_str());
            ImGui::PopID();
        }
    }
}

}