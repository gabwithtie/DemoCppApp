#include "BrushTool.h"
#include "graphics/loaders/TextureLoader.h"
#include "gui/features/input/InputMap.h"
#include <cmath>
#include <iostream>

#include "picsel/viewport/Viewport.h"
#include "picsel/animation/AnimationManager.h"

namespace picsel {

    void BrushTool::DrawSettings() {
        ImGui::Text("Brush Settings");
        ImGui::Separator();

        ImGui::SliderFloat("Radius", &m_radius, 0.5, 50);

        // Color picker with alpha (Alpha 0 = Eraser due to overwrite logic)
        ImGui::ColorEdit4("Color", m_color);

        if (m_color[3] == 0.0f) {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Mode: ERASER");
        }
        else {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Mode: PAINT");
        }
    }

    void BrushTool::ScreenToPixel(ImVec2 screen_pos, ImVec2 canvas_min, float zoom, int& out_px, int& out_py) {
        out_px = static_cast<int>(std::floor((screen_pos.x - canvas_min.x) / zoom));
        out_py = static_cast<int>(std::floor((screen_pos.y - canvas_min.y) / zoom));
    }

    void BrushTool::ProcessInteraction(Viewport* vp, ImVec2 canvas_min, ImVec2 canvas_max, float zoom) {
        if (!vp->HasActiveCanvas()) return;

        app::InputMap& input = app::InputMap::Get();
        ImVec2 mouse_pos     = ImGui::GetMousePos(); // positional data, not a binding

        // Optional: Draw a preview circle of the brush under the cursor
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddCircle(mouse_pos, m_radius * zoom, IM_COL32(255, 255, 255, 150), 0, 1.0f);

        // Only paint if the primary tool button is held down
        if (input.IsHeld(app::InputAction::Tool_PrimaryUse)) {

            // Get the asset data (Assuming this returns a reference to the mutable CPU buffer)
            std::string asset_id = vp->GetActiveCanvas();
            auto tex_data = app::TextureLoader::GetAssetData(asset_id);

            int width = tex_data->dimensions.x;
            int height = tex_data->dimensions.y;
            int channels = tex_data->colorChannels; // Assuming 4 for RGBA

            int center_x, center_y;
            ScreenToPixel(mouse_pos, canvas_min, zoom, center_x, center_y);

            bool pixels_modified = false;

            // Iterate over the bounding box of the brush radius
            for (int y = center_y - m_radius; y <= center_y + m_radius; ++y) {
                for (int x = center_x - m_radius; x <= center_x + m_radius; ++x) {

                    // Bounds check
                    if (x < 0 || x >= width || y < 0 || y >= height) continue;

                    // Circular brush check (Euclidean distance)
                    int dx = x - center_x;
                    int dy = y - center_y;
                    if ((dx * dx + dy * dy) <= (m_radius * m_radius)) {

                        // Calculate 1D array index
                        int pixel_idx = (y * width + x) * channels;

                        // OVERWRITE logic (Direct replacement, no blending)
                        tex_data->data[pixel_idx + 0] = static_cast<uint8_t>(m_color[0] * 255.0f); // R
                        tex_data->data[pixel_idx + 1] = static_cast<uint8_t>(m_color[1] * 255.0f); // G
                        tex_data->data[pixel_idx + 2] = static_cast<uint8_t>(m_color[2] * 255.0f); // B

                        if (channels == 4) {
                            tex_data->data[pixel_idx + 3] = static_cast<uint8_t>(m_color[3] * 255.0f); // A
                        }

                        pixels_modified = true;
                    }
                }
            }

            if (pixels_modified) {
                app::TextureLoader::UpdateGPU(asset_id);
            }
        }
    }
    void BrushTool::ProcessInteractionAnim(AnimationManager* vp, ImVec2 canvas_min, ImVec2 canvas_max, float zoom)
    {
    }
}