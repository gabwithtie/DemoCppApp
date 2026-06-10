#pragma once

#include "gui/main/GuiWindow.h"
#include <string>

namespace picsel {

    class IllustrationBrowser : public app::GuiWindow {
    public:
        IllustrationBrowser();
        std::string GetWindowId() override { return "Illustration Manager##Picsel"; }

    protected:
        void DrawSelf() override;

    private:
        // Structural sub-renders to keep ImGui layout code clean
        void DrawSidebarList();
        void DrawActiveIllustrationWorkspace();
        void DrawCreateIllustrationModal();

        // Front-end transient layout buffers
        char m_buf_new_name[128] = "NewLayerComposition";
        bool m_open_create_modal = false;
    };

} // namespace picsel