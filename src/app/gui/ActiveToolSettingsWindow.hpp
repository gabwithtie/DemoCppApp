#pragma once

#include "../../gui/main/GuiWindow.h"
#include "IllustratorWindow.hpp"

namespace app {

class ActiveToolSettingsWindow : public app::GuiWindow {
public:
    explicit ActiveToolSettingsWindow(IllustratorWindow& illustratorWindow);

    std::string GetWindowId() override { return "Active Tool Settings"; }

protected:
    void DrawSelf() override;

private:
    IllustratorWindow& illustrator;
};

} // namespace app
