#pragma once
#include "AnimationWindow.h"
#include "picsel/animation/AnimationManager.h"
#include "graphics/loaders/TextureLoader.h"
#include "imgui.h"
#include <string>

namespace picsel {

    namespace ViewportStyle {
        static constexpr ImU32 CanvasBg = IM_COL32(24, 24, 24, 255);
        static constexpr ImU32 ErrorMagenta = IM_COL32(255, 0, 255, 255);
        static constexpr ImU32 ShadowBg = IM_COL32(0, 0, 0, 180);
    }

    class AnimationViewport {
    public:
        static inline void DrawCompositeFrame(ImDrawList* dl, ImVec2 canvas_min, ImVec2 canvas_max) {
            auto active_clip = AnimationManager::GetActiveClip();
            if (!active_clip) return;

            // Paint default base layer background bounds
            dl->AddRectFilled(canvas_min, canvas_max, ViewportStyle::CanvasBg);

            // Iterate layers sequentially matching asset Z-depth configurations
            for (int i = 0; i < active_clip->GetLayerCount(); ++i) {
                auto& layer = active_clip->GetLayer(i);
                if (!layer.IsVisible()) continue;

                if (layer.GetType() == LayerType::Object) {
                    std::string asset_id = layer.GetAssetId();
                    auto tex_data = app::TextureLoader::GetAssetData(asset_id);

                    if (tex_data && tex_data->textureHandle != 0) {
                        ImTextureID tex_id = static_cast<ImTextureID>(static_cast<uintptr_t>(tex_data->textureHandle));
                        dl->AddImage(tex_id, canvas_min, canvas_max, ImVec2(0, 0), ImVec2(1, 1));
                    }
                    else {
                        // Diagnostic Fallback routine alerts if dropped assets are null/unregistered keys
                        dl->AddRect(canvas_min, canvas_max, ViewportStyle::ErrorMagenta, 0.0f, 0, 2.0f);
                        dl->AddLine(canvas_min, canvas_max, ViewportStyle::ErrorMagenta, 1.0f);
                        dl->AddLine(ImVec2(canvas_max.x, canvas_min.y), ImVec2(canvas_min.x, canvas_max.y), ViewportStyle::ErrorMagenta, 1.0f);

                        std::string error_msg = "Asset Error: [ " + (asset_id.empty() ? "NULL" : asset_id) + " ] not found in TextureLoader registry.";
                        ImVec2 text_pos = ImVec2(canvas_min.x + 10.0f, canvas_min.y + 10.0f + (i * 22.0f));
                        ImVec2 text_size = ImGui::CalcTextSize(error_msg.c_str());

                        dl->AddRectFilled(ImVec2(text_pos.x - 4, text_pos.y - 2),
                            ImVec2(text_pos.x + text_size.x + 4, text_pos.y + text_size.y + 2),
                            ViewportStyle::ShadowBg);
                        dl->AddText(text_pos, ViewportStyle::ErrorMagenta, error_msg.c_str());
                    }
                }
            }
        }

        static inline void Draw(AnimationWindow* window) {
            auto active_clip = AnimationManager::GetActiveClip();
            if (!active_clip) return;

            ImGui::BeginChild("ViewportRegion", ImVec2(0, -window->k_timeline_height), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            ImVec2 view_pos = ImGui::GetCursorScreenPos();
            ImVec2 view_size = ImGui::GetContentRegionAvail();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 window_size = ImGui::GetWindowSize();

            // Track workspace navigation modifications
            if (ImGui::IsWindowHovered()) {
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
                    ImVec2 delta = ImGui::GetIO().MouseDelta;
                    window->m_preview_pan_x += delta.x;
                    window->m_preview_pan_y += delta.y;
                }
                float wheel = ImGui::GetIO().MouseWheel;
                if (wheel != 0.0f) {
                    window->m_preview_zoom += wheel * 0.1f;
                    if (window->m_preview_zoom < 0.1f) window->m_preview_zoom = 0.1f;
                }
            }

            // Reposition viewport window coordinates on project reloads
            if (window->m_needs_view_reset) {
                window->m_preview_pan_x = (view_size.x * 0.5f) - (active_clip->GetWidth() * 0.5f);
                window->m_preview_pan_y = (view_size.y * 0.5f) - (active_clip->GetHeight() * 0.5f);
                window->m_preview_zoom = 1.0f;
                window->m_needs_view_reset = false;
            }

            ImVec2 canvas_min = ImVec2(view_pos.x + window->m_preview_pan_x, view_pos.y + window->m_preview_pan_y);
            ImVec2 canvas_max = ImVec2(canvas_min.x + (active_clip->GetWidth() * window->m_preview_zoom),
                canvas_min.y + (active_clip->GetHeight() * window->m_preview_zoom));

            DrawCompositeFrame(dl, canvas_min, canvas_max);

            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::InvisibleButton("##ViewportInputCatcher", window_size);

            // Handle asset payloads dragged from file browsers into the view workspace
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PROJECT_ASSET_ITEM")) {
                    const char* dropped_asset_id = (const char*)payload->Data;
                    active_clip->AddObjectLayer("Object Layer: " + std::string(dropped_asset_id), dropped_asset_id);
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::EndChild();
        }
    };
}