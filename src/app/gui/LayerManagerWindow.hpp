#pragma once

#include "../../gui/main/GuiWindow.h"

namespace app {

class LayerManagerWindow : public app::GuiWindow {
public:
    LayerManagerWindow();

    std::string GetWindowId() override { return "Layer Manager"; }

protected:
    void DrawSelf() override;
};

} // namespace app
