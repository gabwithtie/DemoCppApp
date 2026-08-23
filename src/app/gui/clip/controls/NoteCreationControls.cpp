#include "NoteCreationControls.hpp"
#include "gui/clip/NoteManager.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

namespace gsr::gui {

void NoteCreationControls::HandleKeyboardShortcuts(gsr::App& app, Model::Clip& clip) {
    ImGuiIO& io = ImGui::GetIO();
    auto& note_clipboard = m_note_manager->GetNoteClipboard();

    if (ImGui::IsKeyPressed(ImGuiKey_Backspace) || ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        app.SaveUndoPoint();

        std::erase_if(clip.notes, [](const auto& note) {
            return note.selected;
        });
    }

    // 1. Copy selected notes (Ctrl + C)
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C)) {
        note_clipboard.clear();
        for (const auto& note : clip.notes) {
            if (note.selected) {
                note_clipboard.push_back(note);
            }
        }
    }

    // 2. Paste notes at Playhead position (Ctrl + V)
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_V) && !note_clipboard.empty()) {
        app.SaveUndoPoint();

        int64_t rel_playhead = static_cast<int64_t>(app.transport.current_tick) - static_cast<int64_t>(clip.start_tick);
        uint64_t paste_base_tick = std::max<int64_t>(0, rel_playhead);

        uint64_t min_clip_tick = note_clipboard.front().start_tick;
        for (const auto& n : note_clipboard) {
            min_clip_tick = std::min(min_clip_tick, n.start_tick);
        }

        for (auto& n : clip.notes) n.selected = false;

        for (auto note : note_clipboard) {
            note.selected = true;
            int64_t offset = note.start_tick - min_clip_tick;
            note.start_tick = std::min(clip.duration - 1, paste_base_tick + offset);
            clip.notes.push_back(note);
        }

    }

    // 3. Duplicate to End (Ctrl + D)
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D)) {
        uint64_t min_start = UINT64_MAX;
        uint64_t max_end = 0;
        std::vector<size_t> selected_indices;

        for (size_t i = 0; i < clip.notes.size(); ++i) {
            if (clip.notes[i].selected) {
                selected_indices.push_back(i);
                min_start = std::min(min_start, clip.notes[i].start_tick);
                max_end = std::max(max_end, clip.notes[i].start_tick + clip.notes[i].duration);
            }
        }

        if (!selected_indices.empty()) {
            app.SaveUndoPoint();
            
            uint64_t offset = max_end - min_start;

            for (auto& note : clip.notes) {
                note.selected = false;
            }

            std::vector<Model::Note> duplicated_notes;
            duplicated_notes.reserve(selected_indices.size());

            for (size_t idx : selected_indices) {
                Model::Note dup = clip.notes[idx];
                dup.start_tick += offset;
                dup.selected = true;
                if (dup.start_tick < clip.duration) {
                    duplicated_notes.push_back(dup);
                }
            }
            clip.notes.insert(clip.notes.end(), duplicated_notes.begin(), duplicated_notes.end());
            
        }

    }
}

} // namespace gsr::gui