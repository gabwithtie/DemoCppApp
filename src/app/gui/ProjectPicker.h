#pragma once

#include "../../gui/main/GuiWindow.h"

#include <filesystem>
#include <vector>

namespace gsr {

class ProjectPicker : public app::GuiWindow {
public:
    ProjectPicker();

    std::string GetWindowId() override { return "Open Project"; }

protected:
    void DrawSelf() override;

private:
    std::vector<std::filesystem::path> recentProjects;

    static std::filesystem::path RecentProjectsFile();
    void LoadRecentProjects();
    void SaveRecentProject(const std::filesystem::path& path);
    void OpenProject(const std::filesystem::path& path);
};

}