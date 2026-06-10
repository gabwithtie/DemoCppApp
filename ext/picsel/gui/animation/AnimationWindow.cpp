#include "AnimationWindow.h"
#include "tools/BrushTool.h"
#include "graphics/loaders/TextureLoader.h"
#include "gui/features/input/InputMap.h"
#include "imgui.h"
#include <algorithm>
#include <string>


#include "picsel/animation/AnimationManager.h"

namespace picsel {

// ---------------------------------------------------------------------------
// Colour palette used throughout the timeline UI
// ---------------------------------------------------------------------------
namespace TimelineStyle {
    static constexpr ImU32 BgDark          = IM_COL32( 28,  28,  28, 255);
    static constexpr ImU32 BgMid           = IM_COL32( 40,  40,  40, 255);
    static constexpr ImU32 CellEmpty       = IM_COL32( 50,  50,  50, 255);
    static constexpr ImU32 CellActive      = IM_COL32( 70, 130, 180, 200); // steel blue
    static constexpr ImU32 CellDrawing     = IM_COL32( 80, 160,  80, 200); // green
    static constexpr ImU32 CellHover       = IM_COL32(255, 255, 255,  30);
    static constexpr ImU32 FrameCursor     = IM_COL32(255, 200,   0, 200); // gold
    static constexpr ImU32 GridLine        = IM_COL32( 65,  65,  65, 255);
    static constexpr ImU32 TextNormal      = IM_COL32(220, 220, 220, 255);
    static constexpr ImU32 TextDim         = IM_COL32(140, 140, 140, 255);
    static constexpr ImU32 LayerDragHint   = IM_COL32(255, 180,   0, 120);
}

// ---------------------------------------------------------------------------
AnimationWindow::AnimationWindow() {
    m_tools.push_back(std::make_unique<BrushTool>());
}

void AnimationWindow::Tick(float delta_seconds) {
    if (!AnimationManager::GetActiveClip()) return;
    AnimationManager::GetActiveClip()->TickPlayback(delta_seconds);
}

// ---------------------------------------------------------------------------
// DrawSelf — top-level layout
// ---------------------------------------------------------------------------
void AnimationWindow::DrawSelf() {
    if (!AnimationManager::GetActiveClip()) {
        ImGui::TextUnformatted("No animation loaded. Create or open an AnimationClip.");
        return;
    }

    // Reserve the timeline at the bottom; everything else goes to the preview.
    float avail_h      = ImGui::GetContentRegionAvail().y;
    float preview_h    = avail_h - k_timeline_height - ImGui::GetStyle().ItemSpacing.y;

    DrawToolbar();

    // --- Preview region ---
    ImGui::BeginChild("##anim_preview", ImVec2(0, preview_h), false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    DrawPreview();
    ImGui::EndChild();

    // --- Timeline region ---
    ImGui::BeginChild("##anim_timeline", ImVec2(0, k_timeline_height), false,
                      ImGuiWindowFlags_NoScrollbar);
    DrawTimeline();
    ImGui::EndChild();

    float deltaTime = ImGui::GetIO().DeltaTime;
    Tick(deltaTime);
}

// ---------------------------------------------------------------------------
// Toolbar — playback controls, FPS, add/remove frame & layer buttons
// ---------------------------------------------------------------------------
void AnimationWindow::DrawToolbar() {
    if (!AnimationManager::GetActiveClip()) return;

    // Playback
    if (ImGui::Button(AnimationManager::GetActiveClip()->IsPlaying() ? "||" : " > ")) {
        AnimationManager::GetActiveClip()->TogglePlayback();
    }
    ImGui::SameLine();
    if (ImGui::Button("|<")) {
        AnimationManager::GetActiveClip()->Pause();
        AnimationManager::GetActiveClip()->SetCurrentFrame(0);
    }

    ImGui::SameLine();
    ImGui::TextUnformatted("FPS:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(48.0f);
    int fps = AnimationManager::GetActiveClip()->GetFPS();
    if (ImGui::InputInt("##fps", &fps, 1, 5)) {
        AnimationManager::GetActiveClip()->SetFPS(fps);
    }

    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();

    // Frame ops
    if (ImGui::Button("+ Frame")) {
        AnimationManager::GetActiveClip()->AddFrame();
    }
    ImGui::SameLine();
    if (ImGui::Button("- Frame")) {
        int cur = AnimationManager::GetActiveClip()->GetCurrentFrameIndex();
        AnimationManager::GetActiveClip()->RemoveFrame(cur);
    }

    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();

    // Layer ops
    if (ImGui::Button("+ Drawing Layer")) {
        int n = AnimationManager::GetActiveClip()->GetLayerCount();
        AnimationManager::GetActiveClip()->AddDrawingLayer("Drawing " + std::to_string(n + 1));
    }

    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();

    // Tool selector
    for (int i = 0; i < (int)m_tools.size(); ++i) {
        bool sel = (m_active_tool_idx == i);
        if (sel) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.8f));
        if (ImGui::Button(m_tools[i]->GetName().c_str())) m_active_tool_idx = i;
        if (sel) ImGui::PopStyleColor();
        ImGui::SameLine();
    }
    ImGui::NewLine();
}

// ---------------------------------------------------------------------------
// Preview — composite renderer + tool dispatch
// ---------------------------------------------------------------------------
void AnimationWindow::DrawPreview() {
    if (!AnimationManager::GetActiveClip()) return;

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float  cw    = static_cast<float>(AnimationManager::GetActiveClip()->GetWidth());
    float  ch    = static_cast<float>(AnimationManager::GetActiveClip()->GetHeight());

    // Initial zoom-to-fit
    if (m_needs_view_reset && avail.x > 40.0f && avail.y > 40.0f) {
        float sx = (avail.x * 0.85f) / cw;
        float sy = (avail.y * 0.85f) / ch;
        m_preview_zoom  = std::min(sx, sy);
        m_preview_pan_x = (avail.x - cw * m_preview_zoom) * 0.5f;
        m_preview_pan_y = (avail.y - ch * m_preview_zoom) * 0.5f;
        m_needs_view_reset = false;
    }

    ImVec2 origin     = ImGui::GetCursorScreenPos();
    app::InputMap& input = app::InputMap::Get();

    if (ImGui::IsWindowHovered()) {
        // Zoom to cursor
        float scroll = input.GetScrollAxis(app::InputAction::Viewport_Zoom);
        if (scroll != 0.0f) {
            float old_zoom = m_preview_zoom;
            float speed    = (old_zoom < 1.0f) ? 0.05f : 0.15f;
            m_preview_zoom = std::max(0.05f, m_preview_zoom + scroll * speed * old_zoom);
            float ratio    = m_preview_zoom / old_zoom;
            ImVec2 mp      = ImGui::GetMousePos();
            float  rx      = mp.x - (origin.x + m_preview_pan_x);
            float  ry      = mp.y - (origin.y + m_preview_pan_y);
            m_preview_pan_x -= rx * ratio - rx;
            m_preview_pan_y -= ry * ratio - ry;
        }

        // Pan
        ImVec2 pan_delta = input.GetDragDelta(app::InputAction::Viewport_Pan);
        m_preview_pan_x += pan_delta.x;
        m_preview_pan_y += pan_delta.y;
    }

    ImVec2 window_size = ImGui::GetWindowSize();
    float rw = cw * m_preview_zoom;
    float rh = ch * m_preview_zoom;
    ImVec2 canvas_min = ImVec2(origin.x + m_preview_pan_x, origin.y + m_preview_pan_y);
    ImVec2 canvas_max = ImVec2(canvas_min.x + rw, canvas_min.y + rh);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    // Checkerboard background to indicate transparency
    dl->AddRectFilled(canvas_min, canvas_max, IM_COL32(40, 40, 40, 255));

    DrawCompositeFrame(dl, AnimationManager::GetActiveClip()->GetCurrentFrameIndex(), canvas_min, canvas_max, m_preview_zoom);

    // 2. Submit an invisible element across the full canvas zone to capture user input/drops
    ImGui::SetCursorPos(ImVec2(0, 0));
    ImGui::InvisibleButton("##ViewportInputCatcher", window_size);

    // 3. HOOK UP THE ENTIRE VIEWPORT AS A DRAG & DROP TARGET
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("IMAGE_ASSET_ID")) {
            // Reconstruct the asset ID text value out of payload memory
            const char* dropped_asset_id = (const char*)payload->Data;

            // Derive a clean object layer presentation name from the file identifier
            std::filesystem::path path_parser(dropped_asset_id);
            std::string clean_layer_name = path_parser.stem().string();
            if (clean_layer_name.empty()) {
                clean_layer_name = "Object Layer";
            }

            // Fire structural instance injection into your clip configuration
            AnimationManager::GetActiveClip()->AddObjectLayer(clean_layer_name, dropped_asset_id);
        }
        ImGui::EndDragDropTarget();
    }

    // 4. Fall back to standard viewport transformation tool interactions if not dragging a file
    if (m_active_tool_idx >= 0 && m_active_tool_idx < m_tools.size()) {
        //m_tools[m_active_tool_idx]->ProcessInteraction(nullptr, canvas_min, canvas_max, m_preview_zoom);
    }

    dl->AddRect(canvas_min, canvas_max, IM_COL32(75, 75, 75, 255), 0.0f, 0, 1.5f);

    // Tool dispatch — only onto the active Drawing layer
    bool over_canvas = ImGui::IsMouseHoveringRect(canvas_min, canvas_max);
    int  ali         = AnimationManager::GetActiveClip()->GetActiveLayerIndex();
    bool can_draw    = over_canvas
                    && ali >= 0
                    && ali < AnimationManager::GetActiveClip()->GetLayerCount()
                    && AnimationManager::GetActiveClip()->GetLayer(ali).GetType() == LayerType::Drawing
                    && !AnimationManager::GetActiveClip()->IsPlaying();

    if (can_draw && m_active_tool_idx >= 0 && m_active_tool_idx < (int)m_tools.size()) {
        // We need a Viewport* for the tool interface — tools that only need
        // canvas_min/max/zoom work fine; tools reading vp->GetActiveCanvas()
        // need the active drawing layer's asset ID routed through the viewport.
        // For now we pass nullptr and handle inside BrushTool via the layer directly.
        // TODO: wire a lightweight AnimationViewportAdapter if tools need vp access.
        m_tools[m_active_tool_idx]->ProcessInteraction(nullptr, canvas_min, canvas_max, m_preview_zoom);
    }

    // Active layer indicator
    if (ali >= 0 && ali < AnimationManager::GetActiveClip()->GetLayerCount()) {
        const auto& layer = AnimationManager::GetActiveClip()->GetLayer(ali);
        std::string hint = "Active: [" + layer.GetName() + "]";
        if (layer.GetType() == LayerType::Object) hint += "  (Object — read only)";
        dl->AddText(ImVec2(canvas_min.x + 4, canvas_max.y + 4),
                    IM_COL32(180, 180, 180, 200), hint.c_str());
    }
}

// ---------------------------------------------------------------------------
// Composite renderer — blends all visible layers for one frame top-down
// ---------------------------------------------------------------------------
void AnimationWindow::DrawCompositeFrame(ImDrawList* dl, int frame_idx,
                                          ImVec2 canvas_min, ImVec2 canvas_max,
                                          float zoom) {
    if (!AnimationManager::GetActiveClip()) return;

    const AnimationFrame& frame = AnimationManager::GetActiveClip()->GetFrame(frame_idx);
    int layer_count = AnimationManager::GetActiveClip()->GetLayerCount();

    // Layers are stored bottom (index 0) to top (index n-1).
    // Draw bottom-up so higher indices appear on top.
    for (int li = 0; li < layer_count; ++li) {
        const AnimationLayer& layer = AnimationManager::GetActiveClip()->GetLayer(li);
        if (!frame.IsLayerVisible(li, layer.IsVisible())) continue;

        if (layer.GetType() == LayerType::Object) {
            std::string asset_id = layer.GetAssetId();
            auto tex_data = app::TextureLoader::GetAssetData(asset_id);

            if (tex_data && tex_data->textureHandle != 0) {
                // --- VALID IMAGE ROUTINE ---
                ImTextureID tex_id = static_cast<ImTextureID>(static_cast<uintptr_t>(tex_data->textureHandle));
                dl->AddImage(tex_id, canvas_min, canvas_max, ImVec2(0, 0), ImVec2(1, 1));
            }
            else {
                // --- BUG FIX & DIAGNOSTIC FALLBACK ROUTINE ---
                // If the texture failed to load, draw a diagnostic warning box on screen
                ImU32 error_magenta = IM_COL32(255, 0, 255, 255);
                ImU32 transparent_bg = IM_COL32(0, 0, 0, 180);

                // Draw bounding box with corner markers
                dl->AddRect(canvas_min, canvas_max, error_magenta, 0.0f, 0, 2.0f);
                dl->AddLine(canvas_min, canvas_max, error_magenta, 1.0f);
                dl->AddLine(ImVec2(canvas_max.x, canvas_min.y), ImVec2(canvas_min.x, canvas_max.y), error_magenta, 1.0f);

                // Print the exact missing asset reference string to determine what key failed
                std::string error_msg = "Asset Error: [ " + asset_id + " ] not found in TextureLoader registry.";
                ImVec2 text_pos = ImVec2(canvas_min.x + 10.0f, canvas_min.y + 10.0f + (li * 20.0f));

                // Text background drop shadow for legibility
                dl->AddRectFilled(ImVec2(text_pos.x - 4, text_pos.y - 2),
                    ImVec2(text_pos.x + ImGui::CalcTextSize(error_msg.c_str()).x + 4, text_pos.y + 15),
                    transparent_bg);
                dl->AddText(text_pos, error_magenta, error_msg.c_str());
            }
        } else {
            // Drawing layer — upload pixel data if present
            const DrawingFrameData* fd = layer.GetFrameData(frame_idx);
            if (!fd || fd->IsEmpty()) continue;

            // NOTE: For Drawing layers we need a GPU handle. The recommended approach
            // is to maintain a per-layer per-frame TextureLoader entry keyed by a
            // synthetic asset ID (e.g. "anim:<clip_name>:layer<li>:frame<frame_idx>")
            // and call TextureLoader::UpdateGPU after each brush stroke.
            // The lookup below follows that convention; the caller is responsible for
            // creating/updating the texture when pixel data changes.
            std::string draw_asset_id = "anim:" + AnimationManager::GetActiveClip()->GetName()
                                      + ":L" + std::to_string(li)
                                      + ":F" + std::to_string(frame_idx);

            auto tex_data = app::TextureLoader::GetAssetData(draw_asset_id);
            if (!tex_data || tex_data->dimensions.x <= 0) continue;

            ImTextureID tid = static_cast<ImTextureID>(
                static_cast<uintptr_t>(tex_data->textureHandle));
            dl->AddImage(tid, canvas_min, canvas_max, ImVec2(0, 0), ImVec2(1, 1));
        }
    }
}

// ---------------------------------------------------------------------------
// Timeline
// ---------------------------------------------------------------------------
void AnimationWindow::DrawTimeline() {
    if (!AnimationManager::GetActiveClip()) return;

    ImVec2 tl_origin = ImGui::GetCursorScreenPos();
    ImVec2 tl_size   = ImGui::GetContentRegionAvail();
    ImDrawList* dl   = ImGui::GetWindowDrawList();

    // Background
    dl->AddRectFilled(tl_origin,
                      ImVec2(tl_origin.x + tl_size.x, tl_origin.y + tl_size.y),
                      TimelineStyle::BgDark);

    // Invisible widget to capture scroll input over the timeline
    ImGui::InvisibleButton("##tl_area", tl_size);
    if (ImGui::IsItemHovered()) {
        m_timeline_scroll_x -= ImGui::GetIO().MouseWheelH * k_frame_col_width;
        m_timeline_scroll_x  = std::max(0.0f, m_timeline_scroll_x);
    }

    // --- Layer name column (fixed left) ---
    ImVec2 layer_col_min = tl_origin;
    ImVec2 layer_col_max = ImVec2(tl_origin.x + k_layer_col_width, tl_origin.y + tl_size.y);
    dl->AddRectFilled(layer_col_min, layer_col_max, TimelineStyle::BgMid);
    dl->AddLine(ImVec2(layer_col_max.x, layer_col_min.y),
                layer_col_max, TimelineStyle::GridLine);

    DrawTimelineLayerColumn(k_layer_col_width, k_row_height);

    // --- Frame grid (scrollable) ---
    ImVec2 grid_origin = ImVec2(tl_origin.x + k_layer_col_width, tl_origin.y);
    DrawTimelineFrameGrid(grid_origin, k_frame_col_width, k_row_height);

    // Apply deferred layer drag-reorder
    if (m_drag_layer_from >= 0 && m_drag_layer_to >= 0 &&
        m_drag_layer_from != m_drag_layer_to &&
        !ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        AnimationManager::GetActiveClip()->MoveLayer(m_drag_layer_from, m_drag_layer_to);
        m_drag_layer_from = m_drag_layer_to = -1;
    }
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        m_drag_layer_from = m_drag_layer_to = -1;
    }
}

// ---------------------------------------------------------------------------
void AnimationWindow::DrawTimelineLayerColumn(float col_width, float row_height) {
    if (!AnimationManager::GetActiveClip()) return;

    ImVec2      origin = ImGui::GetWindowPos();
    ImDrawList* dl     = ImGui::GetWindowDrawList();

    // Offset to account for the child window position within the outer window
    ImVec2 base = ImGui::GetCursorScreenPos();
    // We already moved to the timeline child, so base is correct.
    base = ImVec2(ImGui::GetWindowPos().x,
                  ImGui::GetWindowPos().y + ImGui::GetScrollY());

    // Re-anchor to the actual timeline child top-left
    base = ImGui::GetItemRectMin(); // last InvisibleButton = whole timeline area
    // Actually just use a known screen position
    ImVec2 tl_screen = ImGui::GetWindowPos();

    int layer_count = AnimationManager::GetActiveClip()->GetLayerCount();

    for (int li = layer_count - 1; li >= 0; --li) {
        // Draw from top (highest layer index = top of stack)
        int   row_idx   = (layer_count - 1) - li;
        float row_y     = tl_screen.y + static_cast<float>(row_idx) * row_height;
        ImVec2 row_min  = ImVec2(tl_screen.x,              row_y);
        ImVec2 row_max  = ImVec2(tl_screen.x + col_width,  row_y + row_height);

        AnimationLayer& layer     = AnimationManager::GetActiveClip()->GetLayer(li);
        bool            is_active = (AnimationManager::GetActiveClip()->GetActiveLayerIndex() == li);

        // Row background
        ImU32 row_bg = is_active ? IM_COL32(55, 80, 110, 255)
                                 : (row_idx % 2 == 0 ? TimelineStyle::BgMid
                                                      : TimelineStyle::BgDark);
        dl->AddRectFilled(row_min, row_max, row_bg);
        dl->AddLine(ImVec2(row_min.x, row_max.y), row_max, TimelineStyle::GridLine);

        // Drag-reorder hint line
        if (m_drag_layer_from == li) {
            dl->AddRectFilled(row_min, row_max, TimelineStyle::LayerDragHint);
        }

        // Visibility toggle (eye icon placeholder — small checkbox)
        ImGui::SetCursorScreenPos(ImVec2(row_min.x + 4, row_y + (row_height - 14) * 0.5f));
        ImGui::PushID(li * 100);
        bool vis = layer.IsVisible();
        if (ImGui::Checkbox("##vis", &vis)) layer.SetVisible(vis);
        ImGui::PopID();

        // Layer type badge
        ImU32 badge_col = (layer.GetType() == LayerType::Drawing)
                        ? IM_COL32(80, 160, 80, 255)
                        : IM_COL32(70, 130, 180, 255);
        dl->AddRectFilled(ImVec2(row_min.x + 24, row_y + 6),
                          ImVec2(row_min.x + 38, row_y + row_height - 6),
                          badge_col, 2.0f);

        // Layer name (click to select, drag to reorder)
        ImGui::SetCursorScreenPos(ImVec2(row_min.x + 42, row_y + (row_height - ImGui::GetTextLineHeight()) * 0.5f));
        ImGui::PushID(li * 100 + 1);
        bool clicked = ImGui::InvisibleButton("##lname",
            ImVec2(col_width - 46, row_height));
        dl->AddText(ImVec2(row_min.x + 44, row_y + (row_height - ImGui::GetTextLineHeight()) * 0.5f),
                    TimelineStyle::TextNormal, layer.GetName().c_str());

        if (clicked) AnimationManager::GetActiveClip()->SetActiveLayerIndex(li);

        // Drag initiation
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            if (m_drag_layer_from < 0) m_drag_layer_from = li;
            // Find target row under mouse
            float my = ImGui::GetMousePos().y;
            int   target_row = static_cast<int>((my - tl_screen.y) / row_height);
            target_row = std::clamp(target_row, 0, layer_count - 1);
            m_drag_layer_to = (layer_count - 1) - target_row;
        }
        ImGui::PopID();
    }
}

// ---------------------------------------------------------------------------
void AnimationWindow::DrawTimelineFrameGrid(ImVec2 grid_origin, float col_width, float row_height) {
    if (!AnimationManager::GetActiveClip()) return;

    ImDrawList* dl          = ImGui::GetWindowDrawList();
    int         frame_count = AnimationManager::GetActiveClip()->GetFrameCount();
    int         layer_count = AnimationManager::GetActiveClip()->GetLayerCount();
    int         cur_frame   = AnimationManager::GetActiveClip()->GetCurrentFrameIndex();
    float       scroll_x    = m_timeline_scroll_x;

    // Visible frame range
    float grid_w      = ImGui::GetContentRegionAvail().x; // remaining after layer col
    int   first_frame = static_cast<int>(scroll_x / col_width);
    int   visible_frames = static_cast<int>(grid_w / col_width) + 2;
    int   last_frame  = std::min(frame_count - 1, first_frame + visible_frames);

    // Frame header row
    for (int fi = first_frame; fi <= last_frame; ++fi) {
        float x = grid_origin.x + (fi - first_frame) * col_width;
        ImVec2 hdr_min = ImVec2(x,             grid_origin.y);
        ImVec2 hdr_max = ImVec2(x + col_width, grid_origin.y + row_height);

        bool is_cur = (fi == cur_frame);
        dl->AddRectFilled(hdr_min, hdr_max,
            is_cur ? TimelineStyle::CellActive : TimelineStyle::BgMid);
        dl->AddLine(ImVec2(hdr_max.x, hdr_min.y), hdr_max, TimelineStyle::GridLine);

        // Frame number
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", fi + 1);
        float tw = ImGui::CalcTextSize(buf).x;
        dl->AddText(ImVec2(x + (col_width - tw) * 0.5f,
                           grid_origin.y + (row_height - ImGui::GetTextLineHeight()) * 0.5f),
                    is_cur ? TimelineStyle::TextNormal : TimelineStyle::TextDim, buf);

        // Click header to scrub
        ImGui::SetCursorScreenPos(hdr_min);
        ImGui::PushID(fi + 10000);
        if (ImGui::InvisibleButton("##fhdr", ImVec2(col_width, row_height))) {
            AnimationManager::GetActiveClip()->Pause();
            AnimationManager::GetActiveClip()->SetCurrentFrame(fi);
        }
        ImGui::PopID();
    }

    // Layer rows
    for (int li = layer_count - 1; li >= 0; --li) {
        int   row_idx = (layer_count - 1) - li;
        float row_y   = grid_origin.y + static_cast<float>(row_idx + 1) * row_height; // +1 for header

        const AnimationLayer& layer = AnimationManager::GetActiveClip()->GetLayer(li);
        const AnimationFrame& cur_f = AnimationManager::GetActiveClip()->GetFrame(cur_frame);

        for (int fi = first_frame; fi <= last_frame; ++fi) {
            float x = grid_origin.x + (fi - first_frame) * col_width;

            ImVec2 cell_min = ImVec2(x,             row_y);
            ImVec2 cell_max = ImVec2(x + col_width, row_y + row_height);

            bool is_cur_col   = (fi == cur_frame);
            bool is_active_row= (AnimationManager::GetActiveClip()->GetActiveLayerIndex() == li);
            bool has_data     = false;

            if (layer.GetType() == LayerType::Drawing) {
                const DrawingFrameData* fd = layer.GetFrameData(fi);
                has_data = fd && !fd->IsEmpty();
            } else {
                has_data = !layer.GetAssetId().empty();
            }

            // Cell background
            ImU32 cell_col = TimelineStyle::CellEmpty;
            if (has_data)   cell_col = (layer.GetType() == LayerType::Drawing)
                                       ? TimelineStyle::CellDrawing
                                       : TimelineStyle::CellActive;
            if (is_cur_col) cell_col = IM_COL32(
                ((cell_col >> 0)  & 0xff),
                ((cell_col >> 8)  & 0xff),
                ((cell_col >> 16) & 0xff), 255); // full alpha on current column

            dl->AddRectFilled(cell_min, cell_max, cell_col);
            dl->AddRect(cell_min, cell_max, TimelineStyle::GridLine);

            // Hover highlight + click to select frame & layer
            ImGui::SetCursorScreenPos(cell_min);
            ImGui::PushID(li * 10000 + fi);
            if (ImGui::InvisibleButton("##cell", ImVec2(col_width, row_height))) {
                AnimationManager::GetActiveClip()->Pause();
                AnimationManager::GetActiveClip()->SetCurrentFrame(fi);
                AnimationManager::GetActiveClip()->SetActiveLayerIndex(li);
            }
            if (ImGui::IsItemHovered()) {
                dl->AddRectFilled(cell_min, cell_max, TimelineStyle::CellHover);
            }
            ImGui::PopID();
        }
    }

    // Current-frame cursor line (drawn on top of everything)
    if (cur_frame >= first_frame && cur_frame <= last_frame) {
        float cx = grid_origin.x + (cur_frame - first_frame) * col_width + col_width * 0.5f;
        float grid_bottom = grid_origin.y + static_cast<float>(layer_count + 1) * row_height;
        dl->AddLine(ImVec2(cx, grid_origin.y),
                    ImVec2(cx, grid_bottom),
                    TimelineStyle::FrameCursor, 2.0f);
    }
}

} // namespace picsel
