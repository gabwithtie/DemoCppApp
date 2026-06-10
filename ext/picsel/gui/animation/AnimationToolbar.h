#pragma once
#include "AnimationWindow.h"
#include "picsel/animation/AnimationManager.h"
#include "imgui.h"

namespace picsel {

    class AnimationToolbar {
    public:
        static inline void Draw(AnimationWindow* window) {
            auto active_clip = AnimationManager::GetActiveClip();
            if (!active_clip) {
                ImGui::TextUnformatted("No active animation clip loaded.");
                return;
            }

            // Playback state modification loops
            if (active_clip->IsPlaying()) {
                if (ImGui::Button("⏸ Pause")) active_clip->Pause();
            }
            else {
                if (ImGui::Button("▶ Play")) active_clip->Play();
            }

            ImGui::SameLine();
            if (ImGui::Button("■ Stop")) {
                active_clip->Pause();
                active_clip->SetCurrentFrame(0);
                window->m_playback_acc = 0.0f;
            }

            ImGui::SameLine();
            ImGui::Separator();
            ImGui::SameLine();

            // Active track index parameters
            int cur_frame = active_clip->GetCurrentFrameIndex();
            int total_frames = active_clip->GetFrameCount();
            ImGui::Text("Frame: %d / %d", cur_frame + 1, total_frames > 0 ? total_frames : 1);

            ImGui::SameLine();
            ImGui::Separator();
            ImGui::SameLine();

            // Playback velocity modifiers
            int fps = active_clip->GetFPS();
            ImGui::SetNextItemWidth(120.0f);
            if (ImGui::SliderInt("FPS", &fps, 1, 60)) {
                active_clip->SetFPS(fps);
            }

            ImGui::SameLine();
            ImGui::Separator();
            ImGui::SameLine();
        }
    };
}