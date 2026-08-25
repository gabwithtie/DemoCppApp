// NoteSubdivideControls.cpp
#include "NoteSubdivideControls.hpp"
#include "App.hpp"
#include <imgui.h>
#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace gsr::gui {

void NoteSubdivideControls::SubdivideNotes(NoteEditorContext& ctx, int divisions) {
    if (divisions <= 1) return;

    auto& clip = ctx.clip;
    bool has_selection = std::any_of(clip.notes.begin(), clip.notes.end(), [](const auto& n) { return n.selected; });
    if (!has_selection) return;

    ctx.app.SaveUndoPoint();

    std::vector<Model::Note> updated_notes;
    updated_notes.reserve(clip.notes.size() * divisions);

    for (const auto& note : clip.notes) {
        if (note.selected && note.duration >= static_cast<uint64_t>(divisions)) {
            uint64_t base_sub_dur = note.duration / divisions;

            for (int i = 0; i < divisions; ++i) {
                Model::Note sub_note = note;
                sub_note.start_tick = note.start_tick + (i * base_sub_dur);
                
                // Assign remaining tick remainder to the final piece to prevent timing gaps
                sub_note.duration = (i == divisions - 1) 
                    ? (note.duration - (i * base_sub_dur)) 
                    : base_sub_dur;
                
                sub_note.selected = true;
                updated_notes.push_back(sub_note);
            }
        } else {
            updated_notes.push_back(note);
        }
    }

    clip.notes = std::move(updated_notes);
}

void NoteSubdivideControls::UnsubdivideNotes(NoteEditorContext& ctx, UnsubdivideMode mode) {
    auto& clip = ctx.clip;
    bool has_selection = std::any_of(clip.notes.begin(), clip.notes.end(), [](const auto& n) { return n.selected; });
    if (!has_selection) return;

    ctx.app.SaveUndoPoint();

    std::vector<Model::Note> remaining_notes;
    std::map<uint8_t, std::vector<Model::Note>> selected_by_pitch;

    // Separate selected notes by pitch while keeping non-selected notes untouched
    for (const auto& note : clip.notes) {
        if (note.selected) {
            selected_by_pitch[note.pitch].push_back(note);
        } else {
            remaining_notes.push_back(note);
        }
    }

    // Process each pitch group independently
    for (auto& [pitch, notes] : selected_by_pitch) {
        // Sort chronologically by start tick
        std::sort(notes.begin(), notes.end(), [](const Model::Note& a, const Model::Note& b) {
            return a.start_tick < b.start_tick;
        });

        if (mode == UnsubdivideMode::Full) {
            if (notes.empty()) continue;

            uint64_t min_start = notes.front().start_tick;
            uint64_t max_end = min_start;
            for (const auto& n : notes) {
                max_end = std::max(max_end, n.start_tick + n.duration);
            }

            Model::Note merged_note = notes.front();
            merged_note.start_tick = min_start;
            merged_note.duration = max_end - min_start;
            merged_note.selected = true;

            remaining_notes.push_back(merged_note);
        } 
        else if (mode == UnsubdivideMode::Once) {
            size_t i = 0;
            while (i < notes.size()) {
                if (i + 1 < notes.size()) {
                    const auto& n1 = notes[i];
                    const auto& n2 = notes[i + 1];

                    uint64_t min_start = std::min(n1.start_tick, n2.start_tick);
                    uint64_t max_end = std::max(n1.start_tick + n1.duration, n2.start_tick + n2.duration);

                    Model::Note merged_note = n1;
                    merged_note.start_tick = min_start;
                    merged_note.duration = max_end - min_start;
                    merged_note.selected = true;

                    remaining_notes.push_back(merged_note);
                    i += 2; // Move past the merged pair
                } else {
                    // Leftover odd note with no pair stays untouched
                    remaining_notes.push_back(notes[i]);
                    i += 1;
                }
            }
        }
    }

    clip.notes = std::move(remaining_notes);
}

void NoteSubdivideControls::DrawContextMenu(NoteEditorContext& ctx) {
    bool has_selection = std::any_of(ctx.clip.notes.begin(), ctx.clip.notes.end(), [](const auto& n) { return n.selected; });

    if (ImGui::BeginMenu("Subdivide", has_selection)) {
        for (int d = 2; d <= 7; ++d) {
            std::string label = std::to_string(d) + " Divisions";
            if (ImGui::MenuItem(label.c_str())) {
                SubdivideNotes(ctx, d);
            }
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Unsubdivide", has_selection)) {
        if (ImGui::MenuItem("Unsubdivide Once")) {
            UnsubdivideNotes(ctx, UnsubdivideMode::Once);
        }
        if (ImGui::MenuItem("Unsubdivide Full")) {
            UnsubdivideNotes(ctx, UnsubdivideMode::Full);
        }
        ImGui::EndMenu();
    }

    ImGui::Separator();
}

} // namespace gsr::gui