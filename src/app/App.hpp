#pragma once

#include "model/Project.hpp"
#include "audio/AudioEngine.hpp"

#include "../gui/features/propertydrawers/PropertyDrawers.hpp"
#include "SerializationIncludes.hpp"

#include "File/Parser.hpp"
#include <string>
#include <memory>
#include <vector>

namespace gsr {

enum class PlaybackState { Stopped, Playing, Paused };

struct Transport {
    PlaybackState state{PlaybackState::Stopped};
    std::atomic<uint64_t> current_tick{0};
    double bpm{120.0};
    uint32_t ppq{960};
};

struct BarSelection {
    int track_index{-1};
    uint32_t start_bar{0};
    uint32_t num_bars{0};
    uint32_t drag_anchor_bar{0};
    bool active{false};
};

struct ViewState {
    int active_track_index{0};
    BarSelection cell_selection;
    
    Model::Clip clip_clipboard;
    bool has_copied_clip{false};

    bool show_piano_roll{true};
    bool show_score_view{true};
    bool show_inspector{true};
    bool show_timeline{true};

    float px_per_tick{0.05f};
    float row_height{48.0f};
    uint64_t scroll_tick{0};
    uint8_t scroll_pitch{72};

    bool show_mute_solo_columns{true};
    bool show_volume_column{true};
    bool show_pan_column{true};
    
    uint32_t quantize_ticks{240};
    bool snap_to_grid{true};
};

class App : public gbe::ISerializable {
public:
    App();
    ~App();

    inline static App& GetInstance() {
        return *s_instance;
    }

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    bool init();
    void process_input();
    void update(float delta_time);
    void render_ui();
    void shutdown();

    // Undo / Redo System
    void SaveUndoPoint() {
        m_undo_stack.push_back(Serialize());
        m_redo_stack.clear();

        if (m_undo_stack.size() > m_max_undo_levels) {
            m_undo_stack.erase(m_undo_stack.begin());
        }
    }

    void Undo() {
        if (m_undo_stack.empty()) return;

        m_redo_stack.push_back(Serialize());

        auto state = m_undo_stack.back();
        m_undo_stack.pop_back();
        Deserialize(state);
    }

    void Redo() {
        if (m_redo_stack.empty()) return;

        m_undo_stack.push_back(Serialize());

        auto state = m_redo_stack.back();
        m_redo_stack.pop_back();
        Deserialize(state);
    }

    gbe::SerializedData Serialize() override {
        gbe::SerializedData data = gbe::ISerializable::Serialize();
        data.serialized_variables["project"] = gbe::Parser::ExportClassStr(project);
        data.serialized_variables["view"] = gbe::Parser::ExportClassStr(view);
        return data;
    }

    void Deserialize(gbe::SerializedData& data) override {
        gbe::ISerializable::Deserialize(data);
        auto it = data.serialized_variables.find("project");
        if (it != data.serialized_variables.end()) {
            gbe::Parser::PopulateClassStr(project, it->second);
        }

        auto view_it = data.serialized_variables.find("view");
        if (view_it != data.serialized_variables.end()) {
            gbe::Parser::PopulateClassStr(view, view_it->second);
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
    audio::AudioEngine audio_engine{*this};

    std::vector<gbe::SerializedData> m_undo_stack;
    std::vector<gbe::SerializedData> m_redo_stack;
    size_t m_max_undo_levels{50};
};

} // namespace gsr