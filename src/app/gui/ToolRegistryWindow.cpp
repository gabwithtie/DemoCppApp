#include "ToolRegistryWindow.hpp"

#include "tools/ToolRegistry.hpp"

#include <algorithm>
#include <imgui.h>

namespace app {

ToolRegistryWindow::ToolRegistryWindow(IllustratorWindow& illustratorWindow)
    : illustrator(illustratorWindow) {
    ownedTools = ToolRegistry::CreateAll();

    if (!ownedTools.empty() && ownedTools.front()) {
        illustrator.SetActiveTool(ownedTools.front().get());
    }
}

void ToolRegistryWindow::DrawSelf() {
    if (ownedTools.empty()) {
        ImGui::TextUnformatted("No tools registered.");
        return;
    }

    Tool* currentTool = illustrator.GetActiveTool();
    ImGui::Text("Registered Tools: %d", static_cast<int>(ownedTools.size()));

    for (auto& toolPtr : ownedTools) {
        Tool* tool = toolPtr.get();
        if (tool == nullptr) {
            continue;
        }
        ImGui::PushID(tool);

        const bool isActive = (tool == currentTool);
        if (ImGui::Selectable(tool->GetToolName().c_str(), isActive)) {
            illustrator.SetActiveTool(tool);
            currentTool = tool;
        }

        ImGui::PopID();
    }
}

} // namespace app
