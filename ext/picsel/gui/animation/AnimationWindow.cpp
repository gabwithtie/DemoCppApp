#include "AnimationWindow.h"
#include "AnimationToolbar.h"
#include "AnimationViewport.h"
#include "AnimationTimeline.h"

#include "picsel/gui/tools/BrushTool.h"

#include "picsel/animation/AnimationManager.h"

namespace picsel {

    AnimationWindow::AnimationWindow() {
        // Register default interactive tool implementations
        m_tools.push_back(std::make_unique<BrushTool>());
    }

    void AnimationWindow::Tick(float delta_seconds) {
        auto active_clip = AnimationManager::GetActiveClip();
        if (!active_clip || !active_clip->IsPlaying()) {
            return;
        }

        // Advance playback simulation time
        m_playback_acc += delta_seconds;
        float frame_duration = active_clip->GetFrameDurationSeconds();

        while (m_playback_acc >= frame_duration) {
            m_playback_acc -= frame_duration;

            int next_frame = active_clip->GetCurrentFrameIndex() + 1;
            if (next_frame >= active_clip->GetFrameCount()) {
                next_frame = 0; // Wrap back around to form seamless loop cycles
            }
            active_clip->SetCurrentFrame(next_frame);   
        }
    }

    void AnimationWindow::DrawSelf() {
        // Orchestrate layout sequences by invoking modular sub-components
        AnimationToolbar::Draw(this);
        ImGui::Separator();

        AnimationViewport::Draw(this);
        ImGui::Separator();

        AnimationTimeline::Draw(this);
    }
}