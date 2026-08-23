#include "NoteSelection.hpp"
#include "gui/clip/NoteManager.hpp"

namespace gsr::gui {

void NoteSelection::HandleKeyboardShortcuts(gsr::App& /*app*/, Model::Clip& clip) {
    ImGuiIO& io = ImGui::GetIO();

    // Select All (Ctrl + A)
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A)) {
        for (auto& note : clip.notes) {
            note.selected = true;
        }
    }
}

} // namespace gsr::gui