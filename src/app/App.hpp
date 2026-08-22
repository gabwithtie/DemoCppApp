#pragma once

#include "model/Project.hpp"
#include "ISerializable.hpp"
#include "File/Parser.hpp"
#include <string>
#include <memory>

namespace gsr {

enum class PlaybackState {
    Stopped,
    Playing,
    Paused
};

struct Transport {
    PlaybackState state{PlaybackState::Stopped};
    uint64_t current_tick{0};        // Master playhead position in PPQ ticks
    double current_time_sec{0.0};    // Playhead position in real seconds
    
    // Looping bounds
    bool loop_enabled{false};
    uint64_t loop_start_tick{0};
    uint64_t loop_end_tick{3840};    // Default: 4 bars at 960 PPQ (4/4 time)
};

// Add / update BarSelection inside App.hpp
struct BarSelection {
    int track_index{-1};
    uint32_t start_bar{0};
    uint32_t num_bars{0};
    uint32_t drag_anchor_bar{0};
    bool active{false};
};

// Add to ViewState inside App.hpp:
struct ViewState {
    int active_track_index{0};
    BarSelection cell_selection;
    
    // Clip Clipboard
    Model::Clip clip_clipboard;
    bool has_copied_clip{false};

    // Panel Visibility
    bool show_piano_roll{true};
    bool show_score_view{true};
    bool show_inspector{true};
    bool show_timeline{true};

    // Piano Roll View Metrics
    float px_per_tick{0.05f};
    float row_height{18.0f};
    uint64_t scroll_tick{0};
    uint8_t scroll_pitch{72};
    
    // Grid & Quantization Settings
    uint32_t quantize_ticks{240};
    bool snap_to_grid{true};
};

class App : public gbe::ISerializable {
public:
    App();
    ~App();

    inline static App& GetInstance(){
        return *s_instance;
    }

    // Enforce single-instance lifetime ownership
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    // Primary Lifecycle Steps
    bool init();
    void process_input();
    void update(float delta_time);
    void render_ui();
    void shutdown();

    // Manual serialization using raw Glaze via Parser helpers
    gbe::SerializedData Serialize() override {
        gbe::SerializedData data = gbe::ISerializable::Serialize();
        data.serialized_variables["project"] = gbe::Parser::ExportClassStr(project);
        return data;
    }

    void Deserialize(gbe::SerializedData& data) override {
        gbe::ISerializable::Deserialize(data);
        auto it = data.serialized_variables.find("project");
        if (it != data.serialized_variables.end()) {
            gbe::Parser::PopulateClassStr(project, it->second);
        }
    }

public:
    Model::Project project;
    Transport transport;
    ViewState view;

    bool is_running{true};
    std::string current_filepath;

private:
    inline static App* s_instance{nullptr};
};

} // namespace gsr