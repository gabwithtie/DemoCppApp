#pragma once

#include "../../gui/main/GuiWindow.h"

#include <filesystem>
#include <vector>

namespace app {

class SampleWindow : public app::GuiWindow {
public:
    SampleWindow();

    std::string GetWindowId() override { return "SampleWindow"; }

protected:
    void DrawSelf() override;
};
}