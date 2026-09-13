#pragma once

#include "../../gui/main/GuiWindow.h"
#include "IllustratorWindow.hpp"
#include "tools/Tool.hpp"

#include <memory>

#include <vector>

namespace app {

class ToolRegistryWindow : public app::GuiWindow {
public:
    explicit ToolRegistryWindow(IllustratorWindow& illustratorWindow);

    std::string GetWindowId() override { return "Tool Registry"; }
    const std::vector<std::unique_ptr<Tool>>& GetTools() const { return ownedTools; }

protected:
    void DrawSelf() override;

private:
    IllustratorWindow& illustrator;
    std::vector<std::unique_ptr<Tool>> ownedTools;
};

} // namespace app
