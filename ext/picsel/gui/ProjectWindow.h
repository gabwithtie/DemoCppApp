#pragma once

#include "gui/main/GuiWindow.h"
#include <filesystem>

namespace picsel {

    class ProjectWindow : public app::GuiWindow {
    public:
        ProjectWindow();
        std::string GetWindowId() override { return "Project Loader##Picsel"; }

    protected:
        void DrawSelf() override;

    private:
        void DrawLandingPage();
        void DrawCreateProjectForm();
        void DrawActiveProjectDashboard();

        // Ephemeral View States used strictly by ImGui layout loops
        enum class ViewState { Landing, Creating, ProjectDashboard };
        ViewState m_view_state = ViewState::Landing;

        // Front-end transient form context storage layout buffers
        char m_buf_name[128] = "MyAnimationProject";
        std::filesystem::path m_selected_folder;
    };
}