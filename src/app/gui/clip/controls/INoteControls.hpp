// INoteControls.hpp
#pragma once

#include "model/Track.hpp"
#include <vector>
#include <cstdint>

namespace gsr {
class App;
}

namespace gsr::gui {

enum class SnapResolution {
    Note4th,
    Note4thTriplet,
    Note8th,
    Note8thTriplet,
    Note16th,
    Note16thTriplet,
    Note32nd
};

struct SnapOption {
    SnapResolution resolution;
    const char* label;
};

inline const SnapOption G_SNAP_OPTIONS[] = {
    { SnapResolution::Note4th,        "1/4 Note" },
    { SnapResolution::Note4thTriplet, "1/4 Triplet" },
    { SnapResolution::Note8th,        "1/8 Note" },
    { SnapResolution::Note8thTriplet, "1/8 Triplet" },
    { SnapResolution::Note16th,       "1/16 Note" },
    { SnapResolution::Note16thTriplet,"1/16 Triplet" },
    { SnapResolution::Note32nd,       "1/32 Note" }
};

inline constexpr int G_SNAP_OPTION_COUNT = sizeof(G_SNAP_OPTIONS) / sizeof(G_SNAP_OPTIONS[0]);

inline uint32_t GetTicksForSnap(SnapResolution res, uint32_t ppq) {
    switch (res) {
        case SnapResolution::Note4th:        return ppq;
        case SnapResolution::Note4thTriplet: return (ppq * 2) / 3;
        case SnapResolution::Note8th:        return ppq / 2;
        case SnapResolution::Note8thTriplet: return ppq / 3;
        case SnapResolution::Note16th:       return ppq / 4;
        case SnapResolution::Note16thTriplet:return ppq / 6;
        case SnapResolution::Note32nd:       return ppq / 8;
        default:                             return ppq / 4;
    }
}

// Lightweight context object aggregating frame dependencies
struct NoteEditorContext {
    gsr::App& app;
    Model::Clip& clip;
    std::vector<Model::Note>& clipboard;
    uint32_t& grid_snap_ticks; // Changed to uint32_t&
};

template <typename T>
struct ModalSession {
    bool active = false;
    bool just_started = false;
    T data{};

    bool Begin(bool is_held) {
        just_started = false;
        if (is_held) {
            if (!active) {
                active = true;
                just_started = true;
                data = T{};
            }
        } else if (active) {
            Cancel();
        }
        return active;
    }

    void Cancel() {
        active = false;
        data = T{};
    }
};

class INoteControls {
public:
    virtual ~INoteControls() = default;

    // Process keyboard shortcuts for this control module
    virtual void HandleKeyboardShortcuts(NoteEditorContext& ctx) = 0;
    
    // Optional context menu items rendered on right-click
    virtual void DrawContextMenu(NoteEditorContext& ctx) {}
};

} // namespace gsr::gui