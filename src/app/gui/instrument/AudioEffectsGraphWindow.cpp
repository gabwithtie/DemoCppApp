#include "AudioEffectsGraphWindow.hpp"

namespace gsr::gui {

AudioEffectsGraphWindow::AudioEffectsGraphWindow(gsr::App& app) 
    : m_app(app) 
{
    BuildDefaultGraph();
}

Model::Track* AudioEffectsGraphWindow::GetSelectedTrack() {
    int track_idx = m_app.view.active_track_index;
    if (m_app.view.cell_selection.active) {
        track_idx = m_app.view.cell_selection.track_index;
    }
    if (track_idx >= 0 && track_idx < static_cast<int>(m_app.project.tracks.size())) {
        return &m_app.project.tracks[track_idx];
    }
    return nullptr;
}

void AudioEffectsGraphWindow::BuildDefaultGraph() {
    m_nodes.clear();
    m_links.clear();
    m_audio_data.clear();

    // 1. Central Instrument Node (Outputs raw sound via 1 connector)
    app::Node inst_node{m_next_id++, "Instrument Engine", ImVec2(50, 100), ImVec2(0,0), {}, {}, ImColor(180, 50, 50)};
    inst_node.outputs.push_back({m_next_id++, inst_node.id, "Raw Sound Out", app::PinKind::Output});
    m_audio_data[inst_node.id] = {AudioNodeType::Instrument};
    m_nodes.push_back(inst_node);

    // 2. Track Processor Master Node (Final Sink)
    app::Node master_node{m_next_id++, "Track Processor", ImVec2(650, 100), ImVec2(0,0), {}, {}, ImColor(50, 180, 50)};
    master_node.inputs.push_back({m_next_id++, master_node.id, "Audio In", app::PinKind::Input});
    m_audio_data[master_node.id] = {AudioNodeType::TrackOutput};
    m_nodes.push_back(master_node);
}

void AudioEffectsGraphWindow::AddEffectNode(const std::string& name) {
    app::Node node{m_next_id++, name, ImVec2(300, 150), ImVec2(0,0), {}, {}, ImColor(50, 100, 180)};
    node.inputs.push_back({m_next_id++, node.id, "In", app::PinKind::Input});
    node.outputs.push_back({m_next_id++, node.id, "Out", app::PinKind::Output});
    
    m_audio_data[node.id] = {AudioNodeType::Effect, name};
    m_nodes.push_back(node);
}

void AudioEffectsGraphWindow::AddCombinerNode() {
    app::Node node{m_next_id++, "Combiner Node", ImVec2(480, 150), ImVec2(0,0), {}, {}, ImColor(140, 80, 180)};
    node.inputs.push_back({m_next_id++, node.id, "In 1", app::PinKind::Input});
    node.inputs.push_back({m_next_id++, node.id, "In 2", app::PinKind::Input});
    node.outputs.push_back({m_next_id++, node.id, "Mix Out", app::PinKind::Output});
    
    m_audio_data[node.id] = {AudioNodeType::Combiner};
    m_nodes.push_back(node);
}

void AudioEffectsGraphWindow::RenderNodeCustomControls(app::Node& node) {
    auto& data = m_audio_data[node.id];
    ImGui::PushItemWidth(90.0f);

    if (data.type == AudioNodeType::Effect) {
        ImGui::Checkbox("Enable", &data.enabled);
        ImGui::SliderFloat("Dry/Wet", &data.parameter_1, 0.0f, 1.0f);
        ImGui::SliderFloat("Param", &data.parameter_2, 0.0f, 1.0f);
    } else if (data.type == AudioNodeType::Combiner) {
        if (ImGui::Button("+ Pin")) {
            node.inputs.push_back({
                m_next_id++, 
                node.id, 
                ("In " + std::to_string(node.inputs.size() + 1)), 
                app::PinKind::Input
            });
        }
    } else if (data.type == AudioNodeType::Instrument) {
        ImGui::TextDisabled("Source Engine");
    } else if (data.type == AudioNodeType::TrackOutput) {
        ImGui::TextDisabled("Master Track Sink");
    }

    ImGui::PopItemWidth();
}

void AudioEffectsGraphWindow::DrawSelf() {
    Model::Track* track = GetSelectedTrack();
    
    // Top Bar Actions & Track Context
    ImGui::BeginGroup();
    if (!track) {
        ImGui::TextDisabled("No active track selected.");
    } else {
        ImGui::Text("Routing Graph for: %s", track->name.c_str());
    }
    
    ImGui::SameLine(0.0f, 20.0f);
    if (ImGui::Button("+ Delay")) AddEffectNode("Delay");
    ImGui::SameLine();
    if (ImGui::Button("+ Reverb")) AddEffectNode("Reverb");
    ImGui::SameLine();
    if (ImGui::Button("+ Combiner")) AddCombinerNode();
    ImGui::SameLine();
    if (ImGui::Button("Reset Graph")) BuildDefaultGraph();
    ImGui::EndGroup();

    ImGui::Separator();

    // Cache lookup pins for link rendering
    m_pin_lookup.clear();
    for (auto& node : m_nodes) {
        for (auto& p : node.inputs)  m_pin_lookup[p.id] = p;
        for (auto& p : node.outputs) m_pin_lookup[p.id] = p;
    }

    // Canvas occupies remaining window region
    app::NodeEditor::BeginCanvas("AudioGraphCanvas", m_editor_ctx, ImVec2(0, 0));
    
    app::NodeEditor::DrawLinks(m_links, m_pin_lookup);

    for (auto& node : m_nodes) {
        app::NodeEditor::DrawNode(m_editor_ctx, node, [this](app::Node& n) {
            RenderNodeCustomControls(n);
        });
    }

    app::NodeEditor::EndCanvas(m_editor_ctx, m_links, [this](app::PinId start, app::PinId end) {
        m_links.push_back({m_next_id++, start, end});
    });
}

} // namespace gsr::gui