#pragma once
#include "AnimationWindow.h"
#include "picsel/animation/AnimationManager.h"
#include "imgui.h"
#include <algorithm>

namespace picsel {

    namespace TimelineStyle {
        static constexpr ImU32 BgDark = IM_COL32(28, 28, 28, 255);
        static constexpr ImU32 CellEmpty = IM_COL32(50, 50, 50, 255);
        static constexpr ImU32 CellActive = IM_COL32(70, 130, 180, 200);
        static constexpr ImU32 CellDrawing = IM_COL32(80, 160, 80, 200);
        static constexpr ImU32 CellHover = IM_COL32(255, 255, 255, 30);
        static constexpr ImU32 FrameCursor = IM_COL32(255, 200, 0, 200);
        static constexpr ImU32 GridLine = IM_COL32(16, 16, 16, 255);
    }

    class AnimationTimeline {
    public:
        static inline void Draw(AnimationWindow* window) {
            auto active_clip = AnimationManager::GetActiveClip();
            if (!active_clip) return;

            ImGui::BeginChild("TimelineContainer", ImVec2(0, window->k_timeline_height), true, ImGuiWindowFlags_NoScrollbar);
            ImDrawList* dl = ImGui::GetWindowDrawList();

            float row_h = window->k_row_height;
            float col_w = window->k_col_width;
            float layer_w = window->k_layer_col_width;
            int layer_count = active_clip->GetLayerCount();
            int frame_count = active_clip->GetFrameCount();
            int cur_frame = active_clip->GetCurrentFrameIndex();
            int active_layer_idx = active_clip->GetActiveLayerIndex();

            // --- LEFT TRACK: LAYER REORDER PANEL ---
            ImGui::BeginGroup();
            for (int li = layer_count - 1; li >= 0; --li) {
                auto& layer = active_clip->GetLayer(li);
                bool is_selected = (active_layer_idx == li);

                ImGui::PushID(li);
                if (ImGui::Selectable(layer.GetName().c_str(), is_selected, ImGuiSelectableFlags_None, ImVec2(layer_w, row_h))) {
                    active_clip->SetActiveLayerIndex(li);
                }

                // Drag handles updating depth configuration vectors
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    window->m_drag_layer_from = li;
                    ImGui::SetDragDropPayload("TIMELINE_LAYER_ROW", &window->m_drag_layer_from, sizeof(int));
                    ImGui::Text("Moving layer: %s", layer.GetName().c_str());
                    ImGui::EndDragDropSource();
                }
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TIMELINE_LAYER_ROW")) {
                        int source_idx = *(const int*)payload->Data;
                        active_clip->MoveLayer(source_idx, li);
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::PopID();
            }
            ImGui::EndGroup();

            ImGui::SameLine();

            // --- RIGHT TRACK: FRAME CELL MATRIX ---
            ImGui::BeginChild("GridScrollArea", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
            ImVec2 grid_origin = ImGui::GetCursorScreenPos();
            ImVec2 grid_size = ImVec2(std::max(frame_count * col_w, ImGui::GetContentRegionAvail().x), layer_count * row_h);

            dl->AddRectFilled(grid_origin, ImVec2(grid_origin.x + grid_size.x, grid_origin.y + grid_size.y), TimelineStyle::BgDark);

            int current_row_idx = 0;
            for (int li = layer_count - 1; li >= 0; --li) {
                auto& layer = active_clip->GetLayer(li);
                float y_min = grid_origin.y + (current_row_idx * row_h);
                float y_max = y_min + row_h;

                for (int fi = 0; fi < frame_count; ++fi) {
                    float x_min = grid_origin.x + (fi * col_w);
                    float x_max = x_min + col_w;

                    ImVec2 cell_min(x_min, y_min);
                    ImVec2 cell_max(x_max, y_max);

                    ImU32 cell_col = TimelineStyle::CellEmpty;
                    if (active_layer_idx == li && cur_frame == fi) {
                        cell_col = TimelineStyle::CellActive;
                    }
                    else if (layer.GetType() == LayerType::Drawing) {
                        cell_col = TimelineStyle::CellDrawing;
                    }

                    dl->AddRectFilled(cell_min, cell_max, cell_col);
                    dl->AddRect(cell_min, cell_max, TimelineStyle::GridLine);

                    ImGui::SetCursorScreenPos(cell_min);
                    ImGui::PushID((li * 5000) + fi);
                    if (ImGui::InvisibleButton("##scrub_btn", ImVec2(col_w, row_h))) {
                        active_clip->Pause();
                        active_clip->SetCurrentFrame(fi);
                        active_clip->SetActiveLayerIndex(li);
                    }
                    if (ImGui::IsItemHovered()) {
                        dl->AddRectFilled(cell_min, cell_max, TimelineStyle::CellHover);
                    }
                    ImGui::PopID();
                }
                current_row_idx++;
            }

            // Draw yellow timeline scrubber playhead cursor on top layer
            if (cur_frame >= 0 && cur_frame < frame_count) {
                float playhead_pos_x = grid_origin.x + (cur_frame * col_w) + (col_w * 0.5f);
                dl->AddLine(ImVec2(playhead_pos_x, grid_origin.y),
                    ImVec2(playhead_pos_x, grid_origin.y + grid_size.y),
                    TimelineStyle::FrameCursor, 2.5f);
            }

            ImGui::EndChild();
            ImGui::EndChild();
        }
    };
}