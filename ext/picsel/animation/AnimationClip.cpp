#include "AnimationClip.h"
#include <algorithm>
#include <stdexcept>

namespace picsel {

AnimationClip::AnimationClip(const std::string& name, int canvas_width, int canvas_height)
    : m_name(name)
    , m_canvas_width(canvas_width)
    , m_canvas_height(canvas_height)
{
    // Every clip starts with one blank frame.
    AnimationFrame f;
    f.frame_index = 0;
    m_frames.push_back(std::move(f));
}

// --- Playback ------------------------------------------------------------------

void AnimationClip::SetCurrentFrame(int idx) {
    if (m_frames.empty()) return;
    m_current_frame = std::clamp(idx, 0, GetFrameCount() - 1);
    m_playback_acc  = 0.0f;
}

bool AnimationClip::TickPlayback(float delta_seconds) {
    if (!m_playing || m_frames.size() <= 1) return false;

    m_playback_acc += delta_seconds;
    float frame_dur = GetFrameDurationSeconds();

    if (m_playback_acc >= frame_dur) {
        m_playback_acc -= frame_dur;
        m_current_frame = (m_current_frame + 1) % GetFrameCount();
        return true;
    }
    return false;
}

// --- Frame management ----------------------------------------------------------

void AnimationClip::AddFrame() {
    int new_idx = GetFrameCount();
    int src_idx = new_idx - 1; // clone from last frame

    AnimationFrame f;
    f.frame_index = new_idx;
    m_frames.push_back(std::move(f));

    // Clone drawing data from the previous frame into the new one.
    for (auto& layer_ptr : m_layers) {
        if (layer_ptr->GetType() == LayerType::Drawing) {
            layer_ptr->CloneFrame(src_idx, new_idx, m_canvas_width, m_canvas_height);
        }
    }
}

void AnimationClip::InsertFrame(int at_idx) {
    if (at_idx < 0 || at_idx > GetFrameCount()) return;

    // Shift all frames at or after at_idx up by one.
    // We do this in the frame list first, then in each drawing layer.
    for (int i = GetFrameCount() - 1; i >= at_idx; --i) {
        // Shift drawing data: old index i → new index i+1.
        for (auto& layer_ptr : m_layers) {
            if (layer_ptr->GetType() == LayerType::Drawing) {
                // Rename existing frame data upward (copy to i+1, remove i).
                auto* src = layer_ptr->GetFrameData(i);
                if (src) {
                    layer_ptr->CloneFrame(i, i + 1, m_canvas_width, m_canvas_height);
                    layer_ptr->RemoveFrame(i);
                }
            }
        }
    }

    // Insert the new frame slot.
    AnimationFrame f;
    f.frame_index = at_idx;
    m_frames.insert(m_frames.begin() + at_idx, std::move(f));

    // Re-stamp frame_index for all frames after insertion.
    for (int i = at_idx; i < GetFrameCount(); ++i) {
        m_frames[i].frame_index = i;
    }

    // Clone pixel data into the new frame from at_idx - 1 (or blank if first).
    int src_idx = at_idx - 1;
    for (auto& layer_ptr : m_layers) {
        if (layer_ptr->GetType() == LayerType::Drawing) {
            if (src_idx >= 0) {
                layer_ptr->CloneFrame(src_idx, at_idx, m_canvas_width, m_canvas_height);
            } else {
                layer_ptr->AllocateFrame(at_idx, m_canvas_width, m_canvas_height);
            }
        }
    }

    // Keep current frame valid.
    if (m_current_frame >= at_idx) m_current_frame++;
    m_current_frame = std::clamp(m_current_frame, 0, GetFrameCount() - 1);
}

void AnimationClip::RemoveFrame(int idx) {
    if (GetFrameCount() <= 1 || idx < 0 || idx >= GetFrameCount()) return;

    m_frames.erase(m_frames.begin() + idx);

    // Re-stamp frame indices.
    for (int i = 0; i < GetFrameCount(); ++i) {
        m_frames[i].frame_index = i;
    }

    // Shift drawing layer data downward.
    for (auto& layer_ptr : m_layers) {
        if (layer_ptr->GetType() == LayerType::Drawing) {
            layer_ptr->RemoveFrame(idx);
            layer_ptr->ShiftFramesAfterRemoval(idx);
        }
    }

    m_current_frame = std::clamp(m_current_frame, 0, GetFrameCount() - 1);
}

// --- Layer management ----------------------------------------------------------

int AnimationClip::AddObjectLayer(const std::string& name, const std::string& asset_id) {
    auto layer = std::make_unique<AnimationLayer>(LayerType::Object, name);
    layer->SetAssetId(asset_id);
    m_layers.push_back(std::move(layer));
    int new_idx = GetLayerCount() - 1;
    if (m_active_layer_idx < 0) m_active_layer_idx = new_idx;
    return new_idx;
}

int AnimationClip::AddDrawingLayer(const std::string& name) {
    auto layer = std::make_unique<AnimationLayer>(LayerType::Drawing, name);

    // Pre-allocate a cloned (or blank) frame for every existing frame.
    for (int i = 0; i < GetFrameCount(); ++i) {
        layer->AllocateFrame(i, m_canvas_width, m_canvas_height);
    }

    m_layers.push_back(std::move(layer));
    int new_idx = GetLayerCount() - 1;
    if (m_active_layer_idx < 0) m_active_layer_idx = new_idx;
    return new_idx;
}

void AnimationClip::RemoveLayer(int idx) {
    if (idx < 0 || idx >= GetLayerCount()) return;
    m_layers.erase(m_layers.begin() + idx);

    // Fix active layer index.
    if (m_active_layer_idx >= GetLayerCount()) {
        m_active_layer_idx = GetLayerCount() - 1;
    }
}

void AnimationClip::MoveLayer(int from_idx, int to_idx) {
    if (from_idx == to_idx) return;
    if (from_idx < 0 || from_idx >= GetLayerCount()) return;
    if (to_idx   < 0 || to_idx   >= GetLayerCount()) return;

    // Rotate the unique_ptr into its new position.
    auto layer = std::move(m_layers[from_idx]);
    m_layers.erase(m_layers.begin() + from_idx);
    m_layers.insert(m_layers.begin() + to_idx, std::move(layer));

    // Keep active layer tracking correct.
    if (m_active_layer_idx == from_idx) {
        m_active_layer_idx = to_idx;
    } else {
        // Shift active index if it was displaced by the move.
        if (from_idx < to_idx) {
            if (m_active_layer_idx > from_idx && m_active_layer_idx <= to_idx)
                m_active_layer_idx--;
        } else {
            if (m_active_layer_idx >= to_idx && m_active_layer_idx < from_idx)
                m_active_layer_idx++;
        }
    }
}

} // namespace picsel
