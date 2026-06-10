#include "IllustrationWindow.h"
#include "picsel/illustration/IllustrationManager.h"

#include "tools/BrushTool.h"

#include "graphics/loaders/TextureLoader.h"
#include "gui/features/input/InputMap.h"
#include "imgui.h"

#include <algorithm>

namespace picsel {

    IllustrationWindow::IllustrationWindow() {
        m_tools.push_back(std::make_unique<BrushTool>());
    }

    IllustrationWindow::~IllustrationWindow() {
        // Explicitly flush active illustration changes to disk when window closes
        IllustrationManager::saveActiveIllustration();
    }

    void IllustrationWindow::DrawSelf() {
        IllustrationData* activeIllus = IllustrationManager::getActiveIllustration();

        // If no illustration is active, present a clean fallback state
        if (!activeIllus) {
            ImGui::TextUnformatted("No illustration loaded. Open one from the Illustration Manager.");
            return;
        }

        // Enforce parallel visibility tracking initialization safety
        if (activeIllus->layerVisibility.size() != activeIllus->layerFilenames.size()) {
            activeIllus->layerVisibility.resize(activeIllus->layerFilenames.size(), true);
        }

        ImGui::Columns(2, "ViewportColumns", false);
        ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() - 260.0f);

        int ali = activeIllus->activeLayerIdx;
        if (ali < 0 || ali >= static_cast<int>(activeIllus->layerFilenames.size())) {
            ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "No valid active layer selected.");
            ImGui::NextColumn(); ImGui::Columns(1); return;
        }

        // Retrieve texture using clean asset strings directly from our configuration data
        auto active_tex = app::TextureLoader::GetAssetData(activeIllus->layerFilenames[ali]);
        if (!active_tex || active_tex->dimensions.x <= 0 || active_tex->dimensions.y <= 0) {
            ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Processing asset payload...");
            ImGui::NextColumn(); ImGui::Columns(1); return;
        }

        float  canvas_w = static_cast<float>(activeIllus->width);
        float  canvas_h = static_cast<float>(activeIllus->height);
        ImVec2 avail = ImGui::GetContentRegionAvail();

        // Zoom-to-fit calculation triggers beautifully via the Viewport state synchronization latch
        if (needs_view_reset && avail.x > 40.0f && avail.y > 40.0f) {
            float ideal_zoom = std::min((avail.x * 0.85f) / canvas_w,
                (avail.y * 0.85f) / canvas_h);
            this->m_zoom = ideal_zoom;
            this->m_pan_x = (avail.x - canvas_w * ideal_zoom) * 0.5f;
            this->m_pan_y = (avail.y - canvas_h * ideal_zoom) * 0.5f;

            needs_view_reset = false;
        }

        ImVec2 origin = ImGui::GetCursorScreenPos();
        app::InputMap& input = app::InputMap::Get();

        // Handle Interactive Navigation Transformations
        if (ImGui::IsWindowHovered()) {
            float scroll = input.GetScrollAxis(app::InputAction::Viewport_Zoom);
            if (scroll != 0.0f) {
                float old_zoom = m_zoom;
                float speed = (old_zoom < 1.0f) ? 0.05f : 0.15f;
                m_zoom += (scroll * speed * old_zoom);
                float new_zoom = m_zoom;
                if (new_zoom != old_zoom) {
                    ImVec2 mp = ImGui::GetMousePos();
                    float  rx = mp.x - (origin.x + m_pan_x);
                    float  ry = mp.y - (origin.y + m_pan_y);
                    float  rat = new_zoom / old_zoom;
                    m_pan_x = (m_pan_x - (rx * rat - rx));
                    m_pan_y = m_pan_y - (ry * rat - ry);
                }
            }

            ImVec2 pan_delta = input.GetDragDelta(app::InputAction::Viewport_Pan);
            if (pan_delta.x != 0.0f || pan_delta.y != 0.0f) {
                m_pan_x += pan_delta.x;
                m_pan_y += pan_delta.y;
            }
        }

        float  rw = canvas_w * m_zoom;
        float  rh = canvas_h * m_zoom;
        ImVec2 canvas_min = ImVec2(origin.x + m_pan_x, origin.y + m_pan_y);
        ImVec2 canvas_max = ImVec2(canvas_min.x + rw, canvas_min.y + rh);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(canvas_min, canvas_max, IM_COL32(24, 24, 24, 255));

        // Composite: Render all layer textures from bottom to top safely
        int layer_count = static_cast<int>(activeIllus->layerFilenames.size());
        for (int li = 0; li < layer_count; ++li) {
            if (!activeIllus->layerVisibility[li]) continue;

            auto tex = app::TextureLoader::GetAssetData(activeIllus->layerFilenames[li]);
            if (!tex || tex->dimensions.x <= 0) continue;

            ImTextureID tid = static_cast<ImTextureID>(static_cast<uintptr_t>(tex->textureHandle));
            dl->AddImage(tid, canvas_min, canvas_max, ImVec2(0, 0), ImVec2(1, 1));
        }

        // Framework borders
        dl->AddRect(canvas_min, canvas_max, IM_COL32(75, 75, 75, 255), 0.0f, 0, 1.5f);
        dl->AddRect(ImVec2(canvas_min.x - 1, canvas_min.y - 1),
            ImVec2(canvas_max.x + 1, canvas_max.y + 1),
            IM_COL32(80, 160, 255, 100), 0.0f, 0, 1.0f);

        // Tool Interaction handling 
        
        bool over_canvas = ImGui::IsMouseHoveringRect(canvas_min, canvas_max);
        if (over_canvas && m_active_tool_idx >= 0 && m_active_tool_idx < (int)m_tools.size()) {
            m_tools[m_active_tool_idx]->ProcessInteraction(activeIllus->layerFilenames[activeIllus->activeLayerIdx], canvas_min, canvas_max, m_zoom);
        }

        // ---- RIGHT COLUMN: Control Operations ----
        ImGui::NextColumn();

        if (ImGui::Button("Save Composition State", ImVec2(-1, 0))) {
            IllustrationManager::saveActiveIllustration();
        }

        ImGui::Spacing();
        ImGui::TextUnformatted("Tools");
        ImGui::Separator();

        for (int i = 0; i < (int)m_tools.size(); ++i) {
            bool sel = (m_active_tool_idx == i);
            if (ImGui::Selectable(m_tools[i]->GetName().c_str(), sel))
                m_active_tool_idx = i;
        }

        if (input.IsPressed(app::InputAction::Tool_CyclePrev))
            m_active_tool_idx = (m_active_tool_idx - 1 + (int)m_tools.size()) % (int)m_tools.size();
        if (input.IsPressed(app::InputAction::Tool_CycleNext))
            m_active_tool_idx = (m_active_tool_idx + 1) % (int)m_tools.size();

        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        if (m_active_tool_idx >= 0 && m_active_tool_idx < (int)m_tools.size())
            m_tools[m_active_tool_idx]->DrawSettings();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("Layers");
        ImGui::Separator();

        // Render stack tracking lists (highest index displays on top visual layer)
        for (int li = layer_count - 1; li >= 0; --li) {
            bool is_active = (activeIllus->activeLayerIdx == li);

            // Extract a neat display label from the full filename paths safely
            std::filesystem::path p(activeIllus->layerFilenames[li]);
            std::string labelName = p.stem().string();

            ImGui::PushID(li);

            // Cast visibility directly back into our application state array
            bool is_visible = activeIllus->layerVisibility[li];
            if (ImGui::Checkbox("##v", &is_visible)) {
                activeIllus->layerVisibility[li] = is_visible;
            }

            ImGui::SameLine();
            if (ImGui::Selectable(labelName.c_str(), is_active)) {
                activeIllus->activeLayerIdx = li;
            }

            ImGui::PopID();
        }

        ImGui::Columns(1);
    }

} // namespace picsel