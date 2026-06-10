#pragma once

#include "AnimationLayer.h"
#include "AnimationFrame.h"

#include <string>
#include <vector>
#include <memory>

namespace picsel {

    // ---------------------------------------------------------------------------
    // AnimationClip
    // Top-level data object for one animation.
    //
    // Owns:
    //   - The ordered layer stack (index = Z-order, 0 = bottom)
    //   - The ordered frame list
    //   - FPS and playback state
    //
    // All mutation goes through this class so invariants stay consistent.
    // The GUI reads from it; tools write drawing data directly into layer frames.
    // ---------------------------------------------------------------------------
    class AnimationClip {
    public:
        explicit AnimationClip(const std::string& name, int canvas_width, int canvas_height);

        // --- Identity ---
        const std::string& GetName()   const { return m_name; }
        void               SetName(const std::string& n) { m_name = n; }
        int                GetWidth()  const { return m_canvas_width; }
        int                GetHeight() const { return m_canvas_height; }

        // --- FPS / timing ---
        int  GetFPS()        const { return m_fps; }
        void SetFPS(int fps)       { m_fps = (fps > 0) ? fps : 1; }
        float GetFrameDurationSeconds() const { return 1.0f / static_cast<float>(m_fps); }

        // --- Playback state ---
        bool IsPlaying()          const { return m_playing; }
        void Play()                     { m_playing = true; }
        void Pause()                    { m_playing = false; }
        void TogglePlayback()           { m_playing = !m_playing; }
        int  GetCurrentFrameIndex()const{ return m_current_frame; }
        void SetCurrentFrame(int idx);

        // Advances playback by delta_seconds. Returns true if frame changed.
        bool TickPlayback(float delta_seconds);

        // --- Frame management ---
        int                    GetFrameCount() const { return static_cast<int>(m_frames.size()); }
        AnimationFrame&        GetFrame(int idx)       { return m_frames[idx]; }
        const AnimationFrame&  GetFrame(int idx) const { return m_frames[idx]; }

        // Appends a new frame. Drawing layers clone pixel data from the last frame.
        void AddFrame();

        // Inserts a frame at position, shifting later frames. Clones from previous.
        void InsertFrame(int at_idx);

        // Removes the frame at idx, re-indexes drawing data.
        void RemoveFrame(int idx);

        // --- Layer management ---
        int                    GetLayerCount() const { return static_cast<int>(m_layers.size()); }
        AnimationLayer&        GetLayer(int idx)       { return *m_layers[idx]; }
        const AnimationLayer&  GetLayer(int idx) const { return *m_layers[idx]; }

        // Returns the index of the currently selected/active layer (-1 if none).
        int  GetActiveLayerIndex()     const { return m_active_layer_idx; }
        void SetActiveLayerIndex(int idx)    { m_active_layer_idx = idx; }

        // Appends a new Object layer. Returns its index.
        int AddObjectLayer(const std::string& name, const std::string& asset_id);

        // Appends a new Drawing layer. Allocates blank frames for all existing frames.
        // Returns its index.
        int AddDrawingLayer(const std::string& name);

        // Removes the layer at idx.
        void RemoveLayer(int idx);

        // Moves layer at from_idx to to_idx (shifts others). Updates active layer.
        void MoveLayer(int from_idx, int to_idx);

    private:
        std::string m_name;
        int         m_canvas_width;
        int         m_canvas_height;

        int         m_fps           = 12;
        bool        m_playing       = false;
        int         m_current_frame = 0;
        float       m_playback_acc  = 0.0f; // accumulated time since last frame advance

        std::vector<AnimationFrame>                  m_frames;
        std::vector<std::unique_ptr<AnimationLayer>> m_layers;
        int                                          m_active_layer_idx = -1;
    };

} // namespace picsel
