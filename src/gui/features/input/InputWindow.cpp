#include "InputWindow.h"
#include "InputMap.h"

#include <imgui.h>

namespace app {

// ---------------------------------------------------------------------------
// Display names shown in the binding table
// ---------------------------------------------------------------------------
const char* InputWindow::ActionDisplayName(InputAction action) {
    using A = InputAction;
    switch (action) {
        case A::Viewport_Zoom:        return "Viewport — Zoom";
        case A::Viewport_Pan:         return "Viewport — Pan";
        case A::Tool_PrimaryUse:      return "Tool — Primary Use";
        case A::Tool_SecondaryUse:    return "Tool — Secondary Use";
        case A::Tool_CyclePrev:       return "Tool — Cycle Previous";
        case A::Tool_CycleNext:       return "Tool — Cycle Next";
        case A::Canvas_Save:          return "Canvas — Save";
        case A::Canvas_Undo:          return "Canvas — Undo";
        case A::Canvas_Redo:          return "Canvas — Redo";
        case A::UI_ToggleInputWindow: return "UI — Toggle This Window";
        default:                      return "Unknown";
    }
}

// ---------------------------------------------------------------------------
InputWindow::InputWindow(std::string config_path)
    : m_config_path(std::move(config_path))
{
    // Bindings are already loaded by the time this window is created,
    // but we could reload here if needed.
}

void InputWindow::SetOpen(bool newstate) {
    bool was_open = is_open;
    GuiWindow::SetOpen(newstate);

    // Auto-save whenever the window transitions from open → closed.
    if (was_open && !newstate) {
        ApplyAndSave();
    }
}

// ---------------------------------------------------------------------------
void InputWindow::DrawSelf() {
    InputMap& im = InputMap::Get();

    ImGui::TextUnformatted("Click a binding button, then press the desired key or mouse button.");
    ImGui::Spacing();

    // Table: Action | Binding | Clear
    if (ImGui::BeginTable("InputBindings", 4,
        ImGuiTableFlags_Borders      |
        ImGuiTableFlags_RowBg        |
        ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Action",  ImGuiTableColumnFlags_WidthStretch, 2.0f);
        ImGui::TableSetupColumn("Binding", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("Modifiers", ImGuiTableColumnFlags_WidthFixed,  130.0f);
        ImGui::TableSetupColumn("",          ImGuiTableColumnFlags_WidthFixed,   40.0f);
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(InputAction::_Count); ++i) {
            DrawActionRow(static_cast<InputAction>(i), i);
        }

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Poll capture result if we're waiting for input.
    if (m_capturing_action_idx >= 0) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Waiting for input...");
        ImGui::SameLine();
        InputBinding captured;
        if (ImGui::SmallButton("Cancel")) {
            im.PollCapture(captured); // discard
            m_capturing_action_idx = -1;
            // Reset capture state without consuming result.
            // (BeginCapture was called; just stop listening.)
            // A cleaner approach: add InputMap::CancelCapture() — see note below.
        }

        if (im.PollCapture(captured)) {
            im.SetBinding(static_cast<InputAction>(m_capturing_action_idx), captured);
            m_capturing_action_idx = -1;
        }
    } else {
        if (ImGui::Button("Apply & Save")) {
            ApplyAndSave();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Defaults")) {
            im.ResetToDefaults();
            m_capturing_action_idx = -1;
        }
    }
}

// ---------------------------------------------------------------------------
void InputWindow::DrawActionRow(InputAction action, int idx) {
    InputMap& im = InputMap::Get();
    ImGui::PushID(idx);

    ImGui::TableNextRow();

    // Column 0 — action name
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted(ActionDisplayName(action));

    // Column 1 — binding button
    ImGui::TableSetColumnIndex(1);
    bool is_capturing_this = (m_capturing_action_idx == idx);

    if (is_capturing_this) {
        // Flash the button to show it's listening.
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.8f, 0.5f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.6f, 0.1f, 1.0f));
        ImGui::Button("[ press any key ]", ImVec2(-1, 0));
        ImGui::PopStyleColor(2);
    } else {
        const std::string& label = im.GetBinding(action).label;
        std::string btn_label    = label.empty() ? "(unbound)" : label;
        if (ImGui::Button(btn_label.c_str(), ImVec2(-1, 0))) {
            // Cancel any previous capture first.
            m_capturing_action_idx = idx;
            im.BeginCapture();
        }
    }

    // Column 2 — modifier checkboxes (Ctrl / Shift / Alt)
    ImGui::TableSetColumnIndex(2);
    {
        InputBinding binding = im.GetBinding(action);
        bool changed = false;

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));

        bool c = binding.requireCtrl;
        if (ImGui::Checkbox("C##c", &c)) { binding.requireCtrl  = c; changed = true; }
        ImGui::SameLine();
        bool s = binding.requireShift;
        if (ImGui::Checkbox("S##s", &s)) { binding.requireShift = s; changed = true; }
        ImGui::SameLine();
        bool a = binding.requireAlt;
        if (ImGui::Checkbox("A##a", &a)) { binding.requireAlt   = a; changed = true; }

        ImGui::PopStyleVar();

        if (changed) {
            binding.BuildLabel();
            im.SetBinding(action, binding);
        }
    }

    // Column 3 — clear button
    ImGui::TableSetColumnIndex(3);
    if (ImGui::SmallButton("X")) {
        im.SetBinding(action, InputBinding::None());
        if (m_capturing_action_idx == idx) {
            m_capturing_action_idx = -1;
        }
    }

    ImGui::PopID();
}

// ---------------------------------------------------------------------------
void InputWindow::ApplyAndSave() {
    InputMap::Get().SaveToFile(m_config_path);
}

} // namespace app
