#pragma once
#include "gui/main/GuiWindow.h"
#include <filesystem>
#include <string>

namespace picsel {
    class AnimationBrowser : public app::GuiWindow {
    public:
        AnimationBrowser() = default;
        std::string GetWindowId() override { return "Animation Browser##Picsel"; }

    protected:
        void DrawSelf() override;

    private:
        void DrawCreatorForm();
        void DrawFileList();

        // State variables
        std::filesystem::path m_current_directory;
        bool m_show_creator = false;

        // ImGui input buffers
        char m_buf_name[128] = "NewAnimation";
        int m_buf_size[2] = { 64, 64 };
    };
}