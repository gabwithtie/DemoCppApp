#include "TimelineClipEditControls.hpp"
#include "../util/MidiFileImporter.hpp"
#include "FileDialogue.hpp"
#include <algorithm>
#include <cmath>
#include <string>

namespace gsr::gui {

bool TimelineClipEditControls::ImportMidiAtSelection(TimelineEditorContext& ctx) {
    const std::string path = gbe::FileDialogue::GetFilePath(gbe::FileDialogue::OPEN);
    if (path.empty()) return false;

    ImportedMidi imported;
    if (!ImportMidiFile(path, imported)) return false;

    const auto& selection = ctx.app.view.cell_selection;
    ctx.app.SaveUndoPoint();
    Model::Clip clip;
    clip.name = "MIDI " + std::to_string(ctx.track.clips.size() + 1);
    clip.start_tick = static_cast<uint64_t>(selection.start_bar) * ctx.ticks_per_bar;
    clip.type = Model::ClipType::Standard;

    uint64_t imported_duration = 0;
    const double scale = static_cast<double>(ctx.app.project.ppq) / imported.ppq;
    for (auto note : imported.notes) {
        note.start_tick = static_cast<uint64_t>(std::llround(note.start_tick * scale));
        note.duration = std::max<uint64_t>(1, static_cast<uint64_t>(std::llround(note.duration * scale)));
        imported_duration = std::max(imported_duration, note.start_tick + note.duration);
        clip.notes.push_back(note);
    }
    clip.duration = std::max<uint64_t>(static_cast<uint64_t>(selection.num_bars) * ctx.ticks_per_bar, imported_duration);
    ctx.track.clips.push_back(std::move(clip));
    return true;
}

void TimelineClipEditControls::HandleKeyboardShortcuts(TimelineEditorContext& ctx) {
    auto& sel = ctx.app.view.cell_selection;
    if (!sel.active || sel.track_index != static_cast<int>(ctx.track_index)) return;

    // 'S' Key shortcut for Split
    if (ImGui::IsKeyPressed(ImGuiKey_S) && !ImGui::GetIO().KeyCtrl) {
        uint64_t target_start = sel.start_bar * ctx.ticks_per_bar;
        for (size_t c_idx = 0; c_idx < ctx.track.clips.size(); ++c_idx) {
            auto& orig = ctx.track.clips[c_idx];
            if (target_start > orig.start_tick && target_start < orig.start_tick + orig.duration) {
                uint64_t first_dur = target_start - orig.start_tick;
                uint64_t second_dur = orig.duration - first_dur;

                Model::Clip second_clip = orig;
                second_clip.start_tick = target_start;
                second_clip.duration = second_dur;
                second_clip.notes.clear();

                auto it = orig.notes.begin();
                while (it != orig.notes.end()) {
                    if (it->start_tick >= first_dur) {
                        Model::Note n = *it;
                        n.start_tick -= first_dur;
                        second_clip.notes.push_back(n);
                        it = orig.notes.erase(it);
                    } else {
                        ++it;
                    }
                }

                orig.duration = first_dur;
                ctx.track.clips.push_back(second_clip);
                break;
            }
        }
    }
}

void TimelineClipEditControls::DrawContextMenu(TimelineEditorContext& ctx) {
    auto& sel = ctx.app.view.cell_selection;
    if (!sel.active || sel.track_index != static_cast<int>(ctx.track_index)) return;
    uint64_t target_start = sel.start_bar * ctx.ticks_per_bar;
    uint64_t target_dur = sel.num_bars * ctx.ticks_per_bar;

    int clip_to_split = -1;
    bool has_overlap = false;

    for (size_t c_idx = 0; c_idx < ctx.track.clips.size(); ++c_idx) {
        const auto& clip = ctx.track.clips[c_idx];
        if ((target_start < clip.start_tick + clip.duration) && (target_start + target_dur > clip.start_tick)) {
            has_overlap = true;
        }
        if (target_start > clip.start_tick && target_start < clip.start_tick + clip.duration) {
            clip_to_split = static_cast<int>(c_idx);
        }
    }

    if (clip_to_split >= 0) {
        if (ImGui::MenuItem(("Split Clip at Bar " + std::to_string(sel.start_bar + 1) + " (S)").c_str())) {
            auto& orig = ctx.track.clips[clip_to_split];
            uint64_t first_dur = target_start - orig.start_tick;
            uint64_t second_dur = orig.duration - first_dur;

            Model::Clip second_clip = orig;
            second_clip.start_tick = target_start;
            second_clip.duration = second_dur;
            second_clip.notes.clear();

            auto it = orig.notes.begin();
            while (it != orig.notes.end()) {
                if (it->start_tick >= first_dur) {
                    Model::Note n = *it;
                    n.start_tick -= first_dur;
                    second_clip.notes.push_back(n);
                    it = orig.notes.erase(it);
                } else {
                    ++it;
                }
            }

            orig.duration = first_dur;
            ctx.track.clips.push_back(second_clip);
        }
        ImGui::Separator();
    }

    if (has_overlap) ImGui::BeginDisabled();
    if (ImGui::MenuItem(("Add Clip (" + std::to_string(sel.num_bars) + " Bars)").c_str())) {
        Model::Clip new_clip;
        new_clip.start_tick = target_start;
        new_clip.duration = target_dur;
        new_clip.name = "Clip " + std::to_string(ctx.track.clips.size() + 1);
        new_clip.type = Model::ClipType::Standard;
        ctx.track.clips.push_back(new_clip);
    }

    bool can_repeat = (target_start >= target_dur);
    if (!can_repeat) ImGui::BeginDisabled();
    if (ImGui::MenuItem(("Add Repeat Clip (" + std::to_string(sel.num_bars) + " Bars)").c_str())) {
        Model::Clip rep_clip;
        rep_clip.start_tick = target_start;
        rep_clip.duration = target_dur;
        rep_clip.name = "Repeat " + std::to_string(ctx.track.clips.size() + 1);
        rep_clip.type = Model::ClipType::Repeat;
        ctx.track.clips.push_back(rep_clip);
    }
    if (!can_repeat) ImGui::EndDisabled();
    if (has_overlap) ImGui::EndDisabled();

    ImGui::Separator();

    if (ImGui::MenuItem("Import MIDI Here...")) {
        if (!ImportMidiAtSelection(ctx)) {
            ImGui::OpenPopup("MIDI Import Failed");
        }
    }
    if (ImGui::BeginPopup("MIDI Import Failed")) {
        ImGui::TextUnformatted("Could not load a MIDI file with note events.");
        if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

} // namespace gsr::gui