#include "TimelineClipboardControls.hpp"
#include <algorithm>

namespace gsr::gui {

bool TimelineClipboardControls::ClipsOverlap(uint64_t start1, uint64_t dur1, uint64_t start2, uint64_t dur2) {
    return (start1 < start2 + dur2) && (start1 + dur1 > start2);
}

void TimelineClipboardControls::HandleKeyboardShortcuts(TimelineEditorContext& ctx) {
    ImGuiIO& io = ImGui::GetIO();
    auto& sel = ctx.app.view.cell_selection;

    if (!sel.active || sel.track_index != static_cast<int>(ctx.track_index)) return;

    uint64_t target_start = sel.start_bar * ctx.ticks_per_bar;
    uint64_t target_dur = sel.num_bars * ctx.ticks_per_bar;

    std::vector<size_t> overlapping_indices;
    for (size_t c_idx = 0; c_idx < ctx.track.clips.size(); ++c_idx) {
        if (ClipsOverlap(target_start, target_dur, ctx.track.clips[c_idx].start_tick, ctx.track.clips[c_idx].duration)) {
            overlapping_indices.push_back(c_idx);
        }
    }

    bool has_overlap = !overlapping_indices.empty();

    // Ctrl + C (Copy)
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C)) {
        if (has_overlap) {
            ctx.app.view.clip_clipboard = ctx.track.clips[overlapping_indices[0]];
            ctx.app.view.has_copied_clip = true;
        }
    }

    // Ctrl + V (Paste)
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_V)) {
        if (ctx.app.view.has_copied_clip && !has_overlap) {
            Model::Clip pasted_clip = ctx.app.view.clip_clipboard;
            pasted_clip.start_tick = target_start;
            ctx.track.clips.push_back(pasted_clip);
        }
    }

    // Delete / Backspace
    if (ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
        if (has_overlap) {
            std::sort(overlapping_indices.begin(), overlapping_indices.end(), std::greater<size_t>());
            for (size_t idx : overlapping_indices) {
                ctx.track.clips.erase(ctx.track.clips.begin() + idx);
            }
        }
    }
}

void TimelineClipboardControls::DrawContextMenu(TimelineEditorContext& ctx) {
    auto& sel = ctx.app.view.cell_selection;
    uint64_t target_start = sel.start_bar * ctx.ticks_per_bar;
    uint64_t target_dur = sel.num_bars * ctx.ticks_per_bar;

    std::vector<size_t> overlapping_indices;
    for (size_t c_idx = 0; c_idx < ctx.track.clips.size(); ++c_idx) {
        if (ClipsOverlap(target_start, target_dur, ctx.track.clips[c_idx].start_tick, ctx.track.clips[c_idx].duration)) {
            overlapping_indices.push_back(c_idx);
        }
    }

    bool has_overlap = !overlapping_indices.empty();

    if (has_overlap && overlapping_indices.size() == 1) {
        if (ImGui::MenuItem("Copy Clip (Ctrl+C)")) {
            ctx.app.view.clip_clipboard = ctx.track.clips[overlapping_indices[0]];
            ctx.app.view.has_copied_clip = true;
        }
    }

    bool can_paste = ctx.app.view.has_copied_clip && !has_overlap;
    if (!can_paste) ImGui::BeginDisabled();
    if (ImGui::MenuItem("Paste Clip (Ctrl+V)")) {
        Model::Clip pasted_clip = ctx.app.view.clip_clipboard;
        pasted_clip.start_tick = target_start;
        ctx.track.clips.push_back(pasted_clip);
    }
    if (!can_paste) ImGui::EndDisabled();

    ImGui::Separator();

    if (!has_overlap) ImGui::BeginDisabled();
    if (ImGui::MenuItem("Delete Selected Clip(s) (Del)")) {
        std::sort(overlapping_indices.begin(), overlapping_indices.end(), std::greater<size_t>());
        for (size_t idx : overlapping_indices) {
            ctx.track.clips.erase(ctx.track.clips.begin() + idx);
        }
    }
    if (!has_overlap) ImGui::EndDisabled();
}

} // namespace gsr::gui