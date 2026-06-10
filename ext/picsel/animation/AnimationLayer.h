#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace picsel {

    enum class LayerType {
        Object,   // References a single external PNG asset — same across all frames
        Drawing,  // Per-frame pixel data owned by this layer
    };

    // ---------------------------------------------------------------------------
    // DrawingFrameData
    // Owned by a Drawing layer, keyed by frame index.
    // Stores raw RGBA pixel data for one frame on this layer.
    // ---------------------------------------------------------------------------
    struct DrawingFrameData {
        std::vector<uint8_t> pixels; // RGBA, width * height * 4 bytes
        int width  = 0;
        int height = 0;

        bool IsEmpty() const { return pixels.empty(); }

        void Allocate(int w, int h) {
            width  = w;
            height = h;
            pixels.assign(w * h * 4, 0); // transparent black
        }

        void CloneFrom(const DrawingFrameData& src) {
            width  = src.width;
            height = src.height;
            pixels = src.pixels;
        }
    };

    // ---------------------------------------------------------------------------
    // AnimationLayer
    // A single layer slot in an AnimationClip.
    //
    // Object layers:   m_asset_id points to a TextureLoader asset; no pixel data.
    // Drawing layers:  m_frames holds per-frame pixel buffers; no asset id.
    // ---------------------------------------------------------------------------
    class AnimationLayer {
    public:
        explicit AnimationLayer(LayerType type, const std::string& name)
            : m_type(type), m_name(name) {}

        // --- Identity ---
        LayerType          GetType()    const { return m_type; }
        const std::string& GetName()    const { return m_name; }
        void               SetName(const std::string& name) { m_name = name; }

        // --- Visibility ---
        bool IsVisible()                const { return m_visible; }
        void SetVisible(bool v)               { m_visible = v; }

        // --- Object layer: asset binding ---
        // Only meaningful when type == Object.
        const std::string& GetAssetId()                    const { return m_asset_id; }
        void               SetAssetId(const std::string& id)     { m_asset_id = id; }

        // --- Drawing layer: per-frame pixel data ---
        // Returns nullptr if no data exists for this frame.
        DrawingFrameData* GetFrameData(int frame_idx) {
            auto it = m_frames.find(frame_idx);
            return (it != m_frames.end()) ? &it->second : nullptr;
        }
        const DrawingFrameData* GetFrameData(int frame_idx) const {
            auto it = m_frames.find(frame_idx);
            return (it != m_frames.end()) ? &it->second : nullptr;
        }

        // Allocates a blank frame at frame_idx (width x height, transparent).
        DrawingFrameData& AllocateFrame(int frame_idx, int width, int height) {
            auto& fd = m_frames[frame_idx];
            fd.Allocate(width, height);
            return fd;
        }

        // Clones pixel data from src_frame into dst_frame.
        // If src_frame has no data, dst_frame gets a blank allocation.
        void CloneFrame(int src_frame_idx, int dst_frame_idx, int width, int height) {
            const DrawingFrameData* src = GetFrameData(src_frame_idx);
            auto& dst = m_frames[dst_frame_idx];
            if (src && !src->IsEmpty()) {
                dst.CloneFrom(*src);
            } else {
                dst.Allocate(width, height);
            }
        }

        // Removes pixel data for a frame (e.g. when a frame is deleted).
        void RemoveFrame(int frame_idx) {
            m_frames.erase(frame_idx);
        }

        // Re-indexes all frames >= removed_idx downward by one.
        // Call after removing a frame from the clip.
        void ShiftFramesAfterRemoval(int removed_idx) {
            std::unordered_map<int, DrawingFrameData> shifted;
            for (auto& [idx, data] : m_frames) {
                int new_idx = (idx > removed_idx) ? idx - 1 : idx;
                shifted[new_idx] = std::move(data);
            }
            m_frames = std::move(shifted);
        }

    private:
        LayerType   m_type;
        std::string m_name;
        bool        m_visible  = true;

        // Object layer
        std::string m_asset_id;

        // Drawing layer — keyed by frame index
        std::unordered_map<int, DrawingFrameData> m_frames;
    };

} // namespace picsel
