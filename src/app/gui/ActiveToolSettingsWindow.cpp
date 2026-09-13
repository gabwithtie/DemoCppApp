#include "ActiveToolSettingsWindow.hpp"

#include "tools/Tool.hpp"

#include <imgui.h>

namespace app {

ActiveToolSettingsWindow::ActiveToolSettingsWindow(IllustratorWindow& illustratorWindow)
    : illustrator(illustratorWindow) {}

void ActiveToolSettingsWindow::DrawSelf() {
    Tool* activeTool = illustrator.GetActiveTool();
    if (activeTool == nullptr) {
        ImGui::TextUnformatted("No active tool selected.");
        return;
    }

    ImGui::Text("Tool: %s", activeTool->GetToolName().c_str());
    ImGui::Separator();
    activeTool->DrawSettingsUi();
}

} // namespace app
