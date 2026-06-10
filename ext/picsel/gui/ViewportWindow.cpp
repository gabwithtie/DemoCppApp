#include "ViewportWindow.h"
#include "picsel/viewport/Viewport.h"

#include "tools/BrushTool.h"

#include "graphics/loaders/TextureLoader.h"
#include "gui/features/input/InputMap.h"   // <-- replaces direct ImGuiIO input reads
#include "imgui.h"

namespace picsel {

    ViewportWindow::ViewportWindow() {
        m_tools.push_back(std::make_unique<BrushTool>());
    }

    ViewportWindow::~ViewportWindow() {
        if (!m_last_active_asset_id.empty()) {
            SaveCanvas(m_last_active_asset_id);
        }
    }

    void ViewportWindow::SaveCanvas(const std::string& asset_id) {
        app::Texture* tex = app::Texture::GetAssetById(asset_id);
        if (tex) app::TextureLoader::ReSave(tex);
    }

    void ViewportWindow::DrawSelf() {
        Viewport* vp = Viewport::Get();
        if (!vp || !vp->HasActiveCanvas()) {
            ImGui::TextUnformatted("No canvas loaded.");
            return;
        }

        std::string current_asset_id = vp->GetActiveCanvas();

        // Auto-save on context switch
        if (m_last_active_asset_id != current_asset_id) {
            if (!m_last_active_asset_id.empty()) {
                SaveCanvas(m_last_active_asset_id);
            }
            m_last_active_asset_id = current_asset_id;
        }

        ImGui::Columns(2, "ViewportColumns", false);
        ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() - 250.0f);

        auto tex_data = app::TextureLoader::GetAssetData(current_asset_id);

        if (!tex_data || tex_data->dimensions.x <= 0 || tex_data->dimensions.y <= 0) {
            ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Processing Asset payload...");
            ImGui::NextColumn();
            ImGui::Columns(1);
            return;
        }

        float canvas_w = static_cast<float>(tex_data->dimensions.x);
        float canvas_h = static_cast<float>(tex_data->dimensions.y);

        ImVec2 available_window_space = ImGui::GetContentRegionAvail();

        // 1. Initial Zoom to Fit
        if (vp->PullNeedsViewReset() && available_window_space.x > 40.0f && available_window_space.y > 40.0f) {
            float scale_factor_x = (available_window_space.x * 0.85f) / canvas_w;
            float scale_factor_y = (available_window_space.y * 0.85f) / canvas_h;
            float ideal_zoom     = std::min(scale_factor_x, scale_factor_y);
            vp->SetZoom(ideal_zoom);

            float center_pan_x = (available_window_space.x - (canvas_w * ideal_zoom)) * 0.5f;
            float center_pan_y = (available_window_space.y - (canvas_h * ideal_zoom)) * 0.5f;
            vp->SetPan(center_pan_x, center_pan_y);
        }

        // 2. Fetch current spatial state
        float pan_x, pan_y;
        vp->GetPan(pan_x, pan_y);
        float current_zoom = vp->GetZoom();

        // 3. Navigation Capture (Zoom & Pan)
        ImVec2 origin_screen_pos = ImGui::GetCursorScreenPos();
        app::InputMap& input     = app::InputMap::Get();

        if (ImGui::IsWindowHovered()) {

            // --- Zoom to Cursor (via InputMap scroll axis) ---
            float scroll = input.GetScrollAxis(app::InputAction::Viewport_Zoom);
            if (scroll != 0.0f) {
                float old_zoom      = vp->GetZoom();
                float speed_modifier = (old_zoom < 1.0f) ? 0.05f : 0.15f;
                vp->AddZoom(scroll * speed_modifier * old_zoom);
                float new_zoom = vp->GetZoom();

                if (new_zoom != old_zoom) {
                    ImVec2 mouse_pos = ImGui::GetMousePos();

                    float mouse_rel_x = mouse_pos.x - (origin_screen_pos.x + pan_x);
                    float mouse_rel_y = mouse_pos.y - (origin_screen_pos.y + pan_y);

                    float ratio    = new_zoom / old_zoom;
                    float new_pan_x = pan_x - (mouse_rel_x * ratio - mouse_rel_x);
                    float new_pan_y = pan_y - (mouse_rel_y * ratio - mouse_rel_y);

                    vp->SetPan(new_pan_x, new_pan_y);
                    vp->GetPan(pan_x, pan_y);
                    current_zoom = vp->GetZoom();
                }
            }

            // --- Pan (via InputMap drag delta) ---
            ImVec2 pan_delta = input.GetDragDelta(app::InputAction::Viewport_Pan);
            if (pan_delta.x != 0.0f || pan_delta.y != 0.0f) {
                vp->AddPan(pan_delta.x, pan_delta.y);
                vp->GetPan(pan_x, pan_y);
            }
        }

        // 4. Render bounds setup
        float rendered_w = canvas_w * current_zoom;
        float rendered_h = canvas_h * current_zoom;

        ImVec2 canvas_min_bounds = ImVec2(origin_screen_pos.x + pan_x, origin_screen_pos.y + pan_y);
        ImVec2 canvas_max_bounds = ImVec2(canvas_min_bounds.x + rendered_w, canvas_min_bounds.y + rendered_h);

        // 5. GPU Rendering
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(canvas_min_bounds, canvas_max_bounds, IM_COL32(24, 24, 24, 255));

        ImTextureID texture_binding_id = static_cast<ImTextureID>(static_cast<uintptr_t>(tex_data->textureHandle));
        draw_list->AddImage(texture_binding_id, canvas_min_bounds, canvas_max_bounds, ImVec2(0, 0), ImVec2(1, 1));
        draw_list->AddRect(canvas_min_bounds, canvas_max_bounds, IM_COL32(75, 75, 75, 255), 0.0f, 0, 1.5f);

        // 6. Tool dispatch — restricted to canvas area
        bool is_mouse_over_canvas = ImGui::IsMouseHoveringRect(canvas_min_bounds, canvas_max_bounds);

        if (is_mouse_over_canvas && m_active_tool_idx >= 0 && m_active_tool_idx < (int)m_tools.size()) {
            m_tools[m_active_tool_idx]->ProcessInteraction(vp, canvas_min_bounds, canvas_max_bounds, current_zoom);
        }

        ImGui::NextColumn();

        // --- RIGHT COLUMN: TOOLBAR & SETTINGS ---
        ImGui::Text("Tools");
        ImGui::Separator();

        for (int i = 0; i < (int)m_tools.size(); ++i) {
            bool is_selected = (m_active_tool_idx == i);
            if (ImGui::Selectable(m_tools[i]->GetName().c_str(), is_selected)) {
                m_active_tool_idx = i;
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Tool cycle hotkeys
        if (input.IsPressed(app::InputAction::Tool_CyclePrev)) {
            m_active_tool_idx = (m_active_tool_idx - 1 + (int)m_tools.size()) % (int)m_tools.size();
        }
        if (input.IsPressed(app::InputAction::Tool_CycleNext)) {
            m_active_tool_idx = (m_active_tool_idx + 1) % (int)m_tools.size();
        }

        if (m_active_tool_idx >= 0 && m_active_tool_idx < (int)m_tools.size()) {
            m_tools[m_active_tool_idx]->DrawSettings();
        }

        ImGui::Columns(1);
    }
}
