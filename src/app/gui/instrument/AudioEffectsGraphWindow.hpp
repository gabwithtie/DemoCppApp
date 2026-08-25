#pragma once

#include "../gui/main/GuiWindow.h"
#include "App.hpp"
#include "../gui/features/nodeeditor/GenericNodeEditor.hpp"
#include <imgui.h>
#include <vector>
#include <string>
#include <unordered_map>

namespace gsr::gui {

enum class AudioNodeType {
    Instrument,
    Effect,
    Combiner,
    TrackOutput
};

struct AudioNodeData {
    AudioNodeType type{AudioNodeType::Effect};
    std::string fx_type_name{"Reverb"};
    float parameter_1{0.5f};
    float parameter_2{0.5f};
    bool enabled{true};
};

class AudioEffectsGraphWindow : public app::GuiWindow {
public:
    explicit AudioEffectsGraphWindow(gsr::App& app);
    ~AudioEffectsGraphWindow() = default;

    std::string GetWindowId() override { return "Audio FX Graph"; }
    void DrawSelf() override;

private:
    gsr::App& m_app;

    app::NodeEditorContext m_editor_ctx;
    std::vector<app::Node> m_nodes;
    std::vector<app::Link> m_links;
    std::unordered_map<app::PinId, app::Pin> m_pin_lookup;
    std::unordered_map<app::NodeId, AudioNodeData> m_audio_data;
    int m_next_id{1000};

    Model::Track* GetSelectedTrack();
    void BuildDefaultGraph();
    void AddEffectNode(const std::string& name);
    void AddCombinerNode();
    void RenderNodeCustomControls(app::Node& node);
};

} // namespace gsr::gui