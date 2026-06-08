#pragma once

#include "gui/main/GuiWindow.h"
#include <string>

namespace picsel {

    struct VirtualFolder; // Forward Declaration

    class ImageBrowser : public app::GuiWindow {
    public:
        ImageBrowser() = default;
        std::string GetWindowId() override { return "Image Browser##Picsel"; }

    protected:
        void DrawSelf() override;

    private:
        void DrawTopActionBar();
        void DrawBreadcrumbs();
        void DrawTreeSide(VirtualFolder* folder, const std::string& current_node_path);
        void DrawContentArea();

        float m_thumbnail_size = 72.0f;
        char m_search_filter[128] = "";
    };
}