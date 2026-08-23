#include "TimelineInteraction.hpp"

namespace gsr::gui {

TimelineInteraction::TimelineInteraction()
    : m_controls{
        &m_selection_controls,
        &m_clipboard_controls,
        &m_clip_edit_controls
      } {}

void TimelineInteraction::HandleKeyboardShortcuts(TimelineEditorContext& ctx) {
    // Only process timeline hotkeys if the Timeline window (or any of its child widgets) is currently focused
    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        return;
    }

    for (auto* control : m_controls) {
        control->HandleKeyboardShortcuts(ctx);
    }
}

void TimelineInteraction::DrawContextMenu(TimelineEditorContext& ctx) {
    for (auto* control : m_controls) {
        control->DrawContextMenu(ctx);
    }
}

} // namespace gsr::gui