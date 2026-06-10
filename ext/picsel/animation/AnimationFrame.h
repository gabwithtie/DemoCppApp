#pragma once

#include <unordered_map>

namespace picsel {

    // ---------------------------------------------------------------------------
    // AnimationFrame
    // Represents one frame slot in a clip.
    //
    // The layer stack and Z-order live on AnimationClip.
    // AnimationFrame only stores per-frame overrides — currently per-layer
    // visibility, which lets a layer be hidden on specific frames without
    // changing its global visible flag.
    // ---------------------------------------------------------------------------
    struct AnimationFrame {
        int frame_index = 0;

        // layer_index → visible override.
        // If a layer index is absent here, fall back to AnimationLayer::IsVisible().
        std::unordered_map<int, bool> layer_visibility_overrides;

        // Returns the effective visibility for a layer on this frame,
        // given the layer's own global visibility as fallback.
        bool IsLayerVisible(int layer_idx, bool layer_global_visible) const {
            auto it = layer_visibility_overrides.find(layer_idx);
            if (it != layer_visibility_overrides.end()) return it->second;
            return layer_global_visible;
        }

        void SetLayerVisibilityOverride(int layer_idx, bool visible) {
            layer_visibility_overrides[layer_idx] = visible;
        }

        void ClearLayerVisibilityOverride(int layer_idx) {
            layer_visibility_overrides.erase(layer_idx);
        }
    };

} // namespace picsel
