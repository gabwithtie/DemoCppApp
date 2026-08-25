#include "AudioEffectsGraphWindow.hpp"

namespace gsr::gui {

AudioEffectsGraphWindow::AudioEffectsGraphWindow(gsr::App& app) 
    : m_app(app) 
{
    m_creation_entries = {
        {"Delay", [this](ImVec2 position) {
            AddEffectNode("Delay", ImVec2(position.x - m_editor_ctx.canvas_origin.x - m_editor_ctx.scrolling.x,
                                           position.y - m_editor_ctx.canvas_origin.y - m_editor_ctx.scrolling.y));
        }},
        {"Reverb", [this](ImVec2 position) {
            AddEffectNode("Reverb", ImVec2(position.x - m_editor_ctx.canvas_origin.x - m_editor_ctx.scrolling.x,
                                            position.y - m_editor_ctx.canvas_origin.y - m_editor_ctx.scrolling.y));
        }},
        {"Gain", [this](ImVec2 position) {
            AddEffectNode("Gain", ImVec2(position.x - m_editor_ctx.canvas_origin.x - m_editor_ctx.scrolling.x,
                                          position.y - m_editor_ctx.canvas_origin.y - m_editor_ctx.scrolling.y));
        }},
        {"Combiner", [this](ImVec2 position) {
            AddCombinerNode(ImVec2(position.x - m_editor_ctx.canvas_origin.x - m_editor_ctx.scrolling.x,
                                   position.y - m_editor_ctx.canvas_origin.y - m_editor_ctx.scrolling.y));
        }}
    };
    m_editor_ctx.node_context_entries = {
        {"Show waveform", [this](app::NodeId node_id) {
            m_waveform_node = m_waveform_node == node_id ? 0 : node_id;
        }}
    };
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
    m_graph_track_index = -2;

    // 1. Central Instrument Node (Outputs raw sound via 1 connector)
    app::Node inst_node{m_next_id++, "Instrument Engine", ImVec2(40, 80), ImVec2(0,0), {}, {}, ImColor(180, 50, 50)};
    inst_node.outputs.push_back({m_next_id++, inst_node.id, "Raw Sound Out", app::PinKind::Output});
    m_audio_data[inst_node.id] = {AudioNodeType::Instrument};
    m_nodes.push_back(inst_node);

    // 2. Track Processor Master Node (Final Sink)
    app::Node master_node{m_next_id++, "Track Processor", ImVec2(760, 80), ImVec2(0,0), {}, {}, ImColor(50, 180, 50)};
    master_node.inputs.push_back({m_next_id++, master_node.id, "Audio In", app::PinKind::Input});
    m_audio_data[master_node.id] = {AudioNodeType::TrackOutput};
    m_nodes.push_back(master_node);
    RebuildGraphLinks();
}

void AudioEffectsGraphWindow::AddEffectNode(const std::string& name, ImVec2 position, bool persist) {
    app::Node node{m_next_id++, name, position, ImVec2(0,0), {}, {}, ImColor(50, 100, 180)};
    node.inputs.push_back({m_next_id++, node.id, "In", app::PinKind::Input});
    node.outputs.push_back({m_next_id++, node.id, "Out", app::PinKind::Output});
    
    AudioNodeData data{AudioNodeType::Effect, name};
    if (persist) {
        if (Model::Track* track = GetSelectedTrack()) {
            data.effect_index = track->audio_graph.nodes.size();
        }
    }
    m_audio_data[node.id] = data;
    m_nodes.push_back(node);
}

void AudioEffectsGraphWindow::SyncGraphToTrack(Model::Track* track, int track_index) {
    if (track_index == m_graph_track_index) return;

    BuildDefaultGraph();
    m_graph_track_index = track_index;
    if (!track) return;

    if (!track->audio_graph.nodes.empty()) {
        m_nodes.clear();
        m_audio_data.clear();
    }
    for (const auto& graph_node : track->audio_graph.nodes) {
        app::Node node = app::NodeEditor::ToRuntimeNode(graph_node);
        node.header_color = graph_node.type_name == "Instrument" ? ImColor(180, 50, 50) :
            graph_node.type_name == "TrackOutput" ? ImColor(50, 180, 50) :
            graph_node.type_name == "Combiner" ? ImColor(140, 80, 180) : ImColor(50, 100, 180);
        m_nodes.push_back(node);
        auto& data = m_audio_data[node.id];
        data.type = graph_node.type_name == "Instrument" ? AudioNodeType::Instrument :
            graph_node.type_name == "TrackOutput" ? AudioNodeType::TrackOutput :
            graph_node.type_name == "Combiner" ? AudioNodeType::Combiner : AudioNodeType::Effect;
        data.fx_type_name = graph_node.title;
        data.effect_index = static_cast<size_t>(&graph_node - track->audio_graph.nodes.data());
        data.enabled = graph_node.enabled;
        data.parameter_1 = graph_node.parameter_1;
        data.parameter_2 = graph_node.parameter_2;
    }
    m_links.clear();
    for (const auto& graph_link : track->audio_graph.links) m_links.push_back(app::NodeEditor::ToRuntimeLink(graph_link));
    if (m_links.empty()) RebuildGraphLinks();
}

void AudioEffectsGraphWindow::RebuildGraphLinks() {
    m_links.clear();
    if (m_nodes.size() < 2 || m_nodes.front().outputs.empty() || m_nodes.back().inputs.empty()) return;
    m_links.push_back({m_next_id++, m_nodes.front().outputs.front().id, m_nodes.back().inputs.front().id});
}

void AudioEffectsGraphWindow::SaveGraphData(Model::Track& track) {
    track.audio_graph.nodes.clear();
    for (const auto& node : m_nodes) {
        const auto& audio_data = m_audio_data[node.id];
        const char* type_name = audio_data.type == AudioNodeType::Effect ? "Effect" :
            audio_data.type == AudioNodeType::Instrument ? "Instrument" :
            audio_data.type == AudioNodeType::Combiner ? "Combiner" : "TrackOutput";
        auto data = app::NodeEditor::ToGraphNode(node, type_name);
        data.parameter_1 = audio_data.parameter_1;
        data.parameter_2 = audio_data.parameter_2;
        data.enabled = audio_data.enabled;
        track.audio_graph.nodes.push_back(std::move(data));
    }
    track.audio_graph.links.clear();
    for (const auto& link : m_links) track.audio_graph.links.push_back(app::NodeEditor::ToGraphLink(link));
}

void AudioEffectsGraphWindow::AddCombinerNode(ImVec2 position) {
    app::Node node{m_next_id++, "Combiner Node", position, ImVec2(0,0), {}, {}, ImColor(140, 80, 180)};
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
        if (data.fx_type_name == "Gain") {
                ImGui::SliderFloat("Gain", &data.parameter_1, 0.0f, 2.0f);
            } else {
                ImGui::SliderFloat("Dry/Wet", &data.parameter_1, 0.0f, 1.0f);
                ImGui::SliderFloat("Param", &data.parameter_2, 0.0f, 1.0f);
            }
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
    if (m_waveform_node == node.id) DrawWaveform(node);
}

void AudioEffectsGraphWindow::DrawWaveform(app::Node& node) {
    Model::Track* track = GetSelectedTrack();
    if (!track) {
        ImGui::TextDisabled("No track output");
        return;
    }
    const int track_index = static_cast<int>(track - m_app.project.tracks.data());
    auto& processors = m_app.GetAudioEngine().GetTrackProcessors();
    if (track_index < 0 || track_index >= static_cast<int>(processors.size())) {
        ImGui::TextDisabled("Audio unavailable");
        return;
    }
    const auto data_it = m_audio_data.find(node.id);
    size_t stage = 0;
    if (data_it != m_audio_data.end() && data_it->second.type == AudioNodeType::Effect) {
        for (size_t index = 0; index < data_it->second.effect_index && index < track->audio_graph.nodes.size(); ++index) {
            if (track->audio_graph.nodes[index].enabled && track->audio_graph.nodes[index].type_name == "Effect") ++stage;
        }
        if (data_it->second.effect_index < track->audio_graph.nodes.size() &&
            track->audio_graph.nodes[data_it->second.effect_index].enabled) ++stage;
    } else if (data_it != m_audio_data.end() && data_it->second.type == AudioNodeType::TrackOutput) {
        for (const auto& graph_node : track->audio_graph.nodes) {
            if (graph_node.enabled && graph_node.type_name == "Effect") ++stage;
        }
    }
    processors[track_index]->CopyWaveform(stage, m_waveform_values);
    ImGui::PlotLines("##waveform", m_waveform_values.data(), static_cast<int>(m_waveform_values.size()), 0,
                    nullptr, -1.0f, 1.0f, ImVec2(180.0f, 48.0f));
}

void AudioEffectsGraphWindow::DrawSelf() {
    Model::Track* track = GetSelectedTrack();
    const int track_index = track ? static_cast<int>(track - m_app.project.tracks.data()) : -1;
    SyncGraphToTrack(track, track_index);
    
    // Top Bar Actions & Track Context
    ImGui::BeginGroup();
    if (!track) {
        ImGui::TextDisabled("No active track selected.");
    } else {
        ImGui::Text("Routing Graph for: %s", track->name.c_str());
    }
    
    if (ImGui::Button("Reset Graph")) {
        if (track) track->audio_graph = {};
        BuildDefaultGraph();
        m_graph_track_index = track_index;
    }
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
    
    for (auto& node : m_nodes) {
        app::NodeEditor::DrawNode(m_editor_ctx, node, [this](app::Node& n) {
            RenderNodeCustomControls(n);
        });
        const auto data_it = m_audio_data.find(node.id);
        if (track && data_it != m_audio_data.end() && data_it->second.type == AudioNodeType::Effect) {
            auto graph_node = std::find_if(track->audio_graph.nodes.begin(), track->audio_graph.nodes.end(),
                [&node](const app::GraphNodeData& value) { return value.id == node.id; });
            if (graph_node != track->audio_graph.nodes.end()) {
                graph_node->position_x = node.pos.x;
                graph_node->position_y = node.pos.y;
            }
        }
    }

    m_pin_lookup.clear();
    for (const auto& node : m_nodes) {
        for (const auto& pin : node.inputs) m_pin_lookup[pin.id] = pin;
        for (const auto& pin : node.outputs) m_pin_lookup[pin.id] = pin;
    }
    app::NodeEditor::DrawLinks(m_links, m_pin_lookup);

    m_editor_ctx.on_undo_point = [this]() { m_app.SaveUndoPoint(); };
    m_editor_ctx.on_node_deleted = [this, track](app::NodeId node_id) {
        const auto data_it = m_audio_data.find(node_id);
        if (track == nullptr || data_it == m_audio_data.end() || data_it->second.type != AudioNodeType::Effect) return;
        const auto node_it = std::find_if(track->audio_graph.nodes.begin(), track->audio_graph.nodes.end(),
            [node_id](const app::GraphNodeData& node) { return node.id == node_id; });
        if (node_it != track->audio_graph.nodes.end()) track->audio_graph.nodes.erase(node_it);
        m_audio_data.erase(data_it);
    };
    app::NodeEditor::EndCanvas(m_editor_ctx, m_nodes, m_links, m_pin_lookup, m_creation_entries,
                               [this](app::PinId start, app::PinId end) {
        m_links.push_back({m_next_id++, start, end});
    });
    if (track) SaveGraphData(*track);
}

} // namespace gsr::gui