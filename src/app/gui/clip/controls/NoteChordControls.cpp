// NoteChordControls.cpp
#include "NoteChordControls.hpp"
#include "App.hpp"
#include <imgui.h>
#include <algorithm>

namespace gsr::gui {

void NoteChordControls::BuildChords(NoteEditorContext& ctx, const std::vector<int>& semitone_offsets) {
    auto& clip = ctx.clip;
    bool has_selection = std::any_of(clip.notes.begin(), clip.notes.end(), [](const auto& n) { return n.selected; });
    if (!has_selection) return;

    ctx.app.SaveUndoPoint();

    std::vector<Model::Note> selected_roots;
    for (const auto& note : clip.notes) {
        if (note.selected) {
            selected_roots.push_back(note);
        }
    }

    for (const auto& root : selected_roots) {
        for (int offset : semitone_offsets) {
            int target_pitch = static_cast<int>(root.pitch) + offset;
            if (target_pitch >= 0 && target_pitch <= 127) {
                Model::Note chord_note = root;
                chord_note.pitch = static_cast<uint8_t>(target_pitch);
                chord_note.selected = true;
                clip.notes.push_back(chord_note);
            }
        }
    }
}

void NoteChordControls::DrawContextMenu(NoteEditorContext& ctx) {
    bool has_selection = std::any_of(ctx.clip.notes.begin(), ctx.clip.notes.end(), [](const auto& n) { return n.selected; });

    if (ImGui::BeginMenu("Build Chord", has_selection)) {
        if (ImGui::BeginMenu("Atonal")) {
            if (ImGui::MenuItem("5 / Power")) BuildChords(ctx, {7});
            if (ImGui::MenuItem("Sus 2"))     BuildChords(ctx, {2, 7});
            if (ImGui::MenuItem("Sus 4"))     BuildChords(ctx, {5, 7});
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Major")) {
            if (ImGui::MenuItem("Normal (Triad)")) BuildChords(ctx, {4, 7});
            if (ImGui::MenuItem("7 (Maj7)"))        BuildChords(ctx, {4, 7, 11});
            if (ImGui::MenuItem("9 (Maj9)"))        BuildChords(ctx, {4, 7, 11, 14});
            if (ImGui::MenuItem("11 (Maj11)"))      BuildChords(ctx, {4, 7, 11, 14, 17});
            if (ImGui::MenuItem("13 (Maj13)"))      BuildChords(ctx, {4, 7, 11, 14, 17, 21});
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Minor")) {
            if (ImGui::MenuItem("Normal (Triad)")) BuildChords(ctx, {3, 7});
            if (ImGui::MenuItem("7 (m7)"))          BuildChords(ctx, {3, 7, 10});
            if (ImGui::MenuItem("9 (m9)"))          BuildChords(ctx, {3, 7, 10, 14});
            if (ImGui::MenuItem("11 (m11)"))        BuildChords(ctx, {3, 7, 10, 14, 17});
            if (ImGui::MenuItem("13 (m13)"))        BuildChords(ctx, {3, 7, 10, 14, 17, 21});
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Other")) {
            if (ImGui::MenuItem("Dim (Triad)"))     BuildChords(ctx, {3, 6});
            if (ImGui::MenuItem("Half Dim (m7b5)")) BuildChords(ctx, {3, 6, 10});
            if (ImGui::MenuItem("Aug (Triad)"))      BuildChords(ctx, {4, 8});
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    ImGui::Separator();
}

} // namespace gsr::gui