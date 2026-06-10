#include "ViewportWindow.h"
#include "picsel/viewport/Viewport.h"
#include "imgui.h"

// Assuming this header exposes your TextureLoader context and TextureData layout declaration
#include "graphics/loaders/TextureLoader.h" 

namespace picsel {

    void ViewportWindow::DrawSelf() {
        Viewport* vp = Viewport::Get();
        if (!vp || !vp->HasActiveCanvas()) {
            ImGui::TextUnformatted("No canvas loaded. Select a file asset node from the browser layout.");
            return;
        }

        std::string current_asset_id = vp->GetActiveCanvas();

        // --- 1. HARVEST HARDWARE METADATA VIA TEXTURELOADER PIPELINE ---
        // Accessing your static asset tracking structure
        auto tex_data = app::TextureLoader::GetAssetData(current_asset_id);

        if (tex_data == nullptr)
            return;

        float canvas_w = static_cast<float>(tex_data->dimensions.x);
        float canvas_h = static_cast<float>(tex_data->dimensions.y);

        // Fail-safe protection layer for empty initialization allocations
        if (canvas_w <= 0.0f || canvas_h <= 0.0f) {
            ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Processing Asset payload: '%s'...", current_asset_id.c_str());
            return;
        }

        // Target pane spatial size constraints mapping evaluation
        ImVec2 available_window_space = ImGui::GetContentRegionAvail();

        // --- 2. DYNAMIC BASE ZOOM INITIALIZATION Sequence ---
        // If a new file is loaded, dynamically scale the view to occupy up to 85% of the viewport container window
        if (vp->PullNeedsViewReset() && available_window_space.x > 40.0f && available_window_space.y > 40.0f) {
            float scale_factor_x = (available_window_space.x * 0.85f) / canvas_w;
            float scale_factor_y = (available_window_space.y * 0.85f) / canvas_h;

            // Choose the restrictive axis limit to avoid layout overflowing boundaries
            float ideal_zoom = std::min(scale_factor_x, scale_factor_y);
            vp->SetZoom(ideal_zoom);

            // Center the initial pan alignment based on computed scale factors
            float center_pan_x = (available_window_space.x - (canvas_w * ideal_zoom)) * 0.5f;
            float center_pan_y = (available_window_space.y - (canvas_h * ideal_zoom)) * 0.5f;
            vp->SetPan(center_pan_x, center_pan_y);
        }

        // --- 3. HARBOARD NAVIGATION CAPTURE LOOPS ---
        if (ImGui::IsWindowHovered()) {
            ImGuiIO& io = ImGui::GetIO();

            // Zoom Step handling (Relative to current position scale focus)
            if (io.MouseWheel != 0.0f) {
                // Modulate scale rate steps dynamically based on current proximity depth 
                float speed_modifier = (vp->GetZoom() < 1.0f) ? 0.05f : 0.15f;
                vp->AddZoom(io.MouseWheel * speed_modifier * vp->GetZoom());
            }

            // Pan Processing Layout routing via Middle Mouse button bindings
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
                vp->AddPan(io.MouseDelta.x, io.MouseDelta.y);
            }
        }

        // --- 4. DATA MATRIX CALCULATION AND VECTOR DISPATCH ---
        float pan_x, pan_y;
        vp->GetPan(pan_x, pan_y);
        float current_zoom = vp->GetZoom();

        float rendered_w = canvas_w * current_zoom;
        float rendered_h = canvas_h * current_zoom;

        // Establish structural target coordinates relative to frame render regions
        ImVec2 origin_screen_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_min_bounds = ImVec2(origin_screen_pos.x + pan_x, origin_screen_pos.y + pan_y);
        ImVec2 canvas_max_bounds = ImVec2(canvas_min_bounds.x + rendered_w, canvas_min_bounds.y + rendered_h);

        // --- 5. GPU RENDERING DRAW LIST EXECUTION PIPELINE ---
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Background backing pass to identify boundary limits clearly
        draw_list->AddRectFilled(canvas_min_bounds, canvas_max_bounds, IM_COL32(24, 24, 24, 255));

        // Binding OpenGL context address fields securely through type casting matrices
        ImTextureID texture_binding_id = static_cast<ImTextureID>(static_cast<uintptr_t>(tex_data->textureHandle));

        // Execute primary hardware fragment shader mapping sequence pass
        draw_list->AddImage(texture_binding_id, canvas_min_bounds, canvas_max_bounds, ImVec2(0, 0), ImVec2(1, 1));

        // High contrast asset highlighting boundaries structure contouring lines
        draw_list->AddRect(canvas_min_bounds, canvas_max_bounds, IM_COL32(75, 75, 75, 255), 0.0f, 0, 1.5f);

        // --- 6. OVERLAY METADATA VISUALIZATION PANELS ---
        ImGui::SetCursorScreenPos(ImVec2(origin_screen_pos.x + 10, origin_screen_pos.y + 10));
        ImGui::TextColored(ImVec4(0.3f, 0.9f, 1.0f, 1.0f), "Canvas Resolution: %dx%d px", tex_data->dimensions.x, tex_data->dimensions.y);

        ImGui::SetCursorScreenPos(ImVec2(origin_screen_pos.x + 10, origin_screen_pos.y + 28));
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "Effective Scaling Factor: %.1f%%", current_zoom * 100.0f);
    }
}