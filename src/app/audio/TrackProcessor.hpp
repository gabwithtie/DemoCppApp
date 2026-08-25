#pragma once

#include "IInstrument.hpp"
#include "../model/Track.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace gsr::audio {

class TrackProcessor {
public:
    TrackProcessor() = default;

    void SetInstrument(std::unique_ptr<IInstrument> instrument) {
        m_instrument = std::move(instrument);
    }

    [[nodiscard]] bool HasInstrument() const { return m_instrument != nullptr; }
    [[nodiscard]] IInstrument* GetInstrument() const { return m_instrument.get(); }

    void CopyWaveform(size_t stage, std::vector<float>& output) const {
        output.resize(kWaveformPoints);
        const size_t index = std::min(stage, kMaxWaveformStages - 1);
        for (size_t point = 0; point < kWaveformPoints; ++point) {
            output[point] = m_waveforms[index][point].load(std::memory_order_relaxed);
        }
    }

    void ProcessAudioBlock(float* mix_output_buffer, size_t num_frames, double sample_rate, float track_volume,
                           const app::GraphData& graph) {
        // Pre-allocated scratch buffer resize (guaranteed lock-free if capacity is sufficient)
        if (m_scratch_buffer.size() < num_frames * 2) {
            m_scratch_buffer.resize(num_frames * 2, 0.0f);
        }

        if (m_instrument) {
            m_instrument->ProcessAndRender(m_scratch_buffer.data(), num_frames, sample_rate);
        } else {
            std::fill_n(m_scratch_buffer.data(), num_frames * 2, 0.0f);
        }

        CaptureWaveform(0, num_frames);
        ProcessEffects(graph, num_frames, sample_rate);

        // Mix track scratch buffer into master output with volume control
        for (size_t i = 0; i < num_frames * 2; ++i) {
            mix_output_buffer[i] += m_scratch_buffer[i] * track_volume;
        }
    }

private:
    static constexpr size_t kWaveformPoints = 128;
    static constexpr size_t kMaxWaveformStages = 32;

    void CaptureWaveform(size_t stage, size_t num_frames) {
        const size_t index = std::min(stage, kMaxWaveformStages - 1);
        for (size_t point = 0; point < kWaveformPoints; ++point) {
            const size_t frame = point * num_frames / kWaveformPoints;
            const size_t sample = std::min(frame, num_frames - 1) * 2;
            m_waveforms[index][point].store((m_scratch_buffer[sample] + m_scratch_buffer[sample + 1]) * 0.5f,
                                            std::memory_order_relaxed);
        }
    }

    void ProcessEffects(const app::GraphData& graph, size_t num_frames, double sample_rate) {
        if (graph.nodes.empty() || graph.links.empty()) return;

        std::unordered_map<app::PinId, app::NodeId> pin_nodes;
        std::unordered_map<app::NodeId, const app::GraphNodeData*> nodes;
        for (const auto& node : graph.nodes) {
            nodes[node.id] = &node;
            for (const auto& pin : node.outputs) pin_nodes[pin.id] = node.id;
            for (const auto& pin : node.inputs) pin_nodes[pin.id] = node.id;
        }

        const app::GraphNodeData* source = nullptr;
        const app::GraphNodeData* output = nullptr;
        for (const auto& node : graph.nodes) {
            if (node.type_name == "Instrument") source = &node;
            if (node.type_name == "TrackOutput") output = &node;
        }
        if (!source || !output || source->outputs.empty() || output->inputs.empty()) return;

        std::unordered_map<app::PinId, app::PinId> next_pins;
        for (const auto& link : graph.links) next_pins[link.start_pin_id] = link.end_pin_id;
        std::vector<const app::GraphNodeData*> chain;
        std::unordered_set<app::NodeId> visited;
        const app::GraphNodeData* current = source;
        while (current && current != output && visited.insert(current->id).second) {
            chain.push_back(current);
            if (current->outputs.empty()) return;
            const auto next_pin = next_pins.find(current->outputs.front().id);
            if (next_pin == next_pins.end()) return;
            const auto next_node = pin_nodes.find(next_pin->second);
            if (next_node == pin_nodes.end()) return;
            const auto node_it = nodes.find(next_node->second);
            if (node_it == nodes.end()) return;
            current = node_it->second;
        }
        if (current != output) return;

        bool needs_delay_buffer = false;
        for (const auto* node : chain) {
            if (node->type_name != "Effect" || !node->enabled) continue;
            needs_delay_buffer |= node->title == "Delay" || node->title == "Reverb";
        }
        const size_t delay_frames = static_cast<size_t>(sample_rate * 0.25);
        if (needs_delay_buffer && m_delay_buffer.size() != delay_frames * 2) {
            m_delay_buffer.assign(delay_frames * 2, 0.0f);
            m_delay_index = 0;
        }

        size_t stage = 1;
        for (const auto* effect : chain) {
            if (!effect->enabled || effect->type_name != "Effect") continue;
            if (effect->title == "Gain") {
                const float gain = std::clamp(effect->parameter_1, 0.0f, 2.0f);
                for (size_t sample = 0; sample < num_frames * 2; ++sample) m_scratch_buffer[sample] *= gain;
            } else if (needs_delay_buffer && (effect->title == "Delay" || effect->title == "Reverb")) {
                const float mix = std::clamp(effect->parameter_1, 0.0f, 1.0f);
                const float feedback = effect->title == "Reverb" ? 0.7f : 0.0f;
                for (size_t frame = 0; frame < num_frames; ++frame) {
                    const size_t buffer_index = m_delay_index * 2;
                    const float delayed_left = m_delay_buffer[buffer_index];
                    const float delayed_right = m_delay_buffer[buffer_index + 1];
                    const float input_left = m_scratch_buffer[frame * 2];
                    const float input_right = m_scratch_buffer[frame * 2 + 1];
                    m_delay_buffer[buffer_index] = input_left + delayed_left * feedback;
                    m_delay_buffer[buffer_index + 1] = input_right + delayed_right * feedback;
                    m_scratch_buffer[frame * 2] = input_left * (1.0f - mix) + delayed_left * mix;
                    m_scratch_buffer[frame * 2 + 1] = input_right * (1.0f - mix) + delayed_right * mix;
                    m_delay_index = (m_delay_index + 1) % delay_frames;
                }
            }
            CaptureWaveform(stage++, num_frames);
        }
    }

    std::unique_ptr<IInstrument> m_instrument;
    std::vector<float> m_scratch_buffer; // Reusable buffer to avoid heap allocations in audio loop
    std::vector<float> m_delay_buffer;
    size_t m_delay_index{0};
    std::array<std::array<std::atomic<float>, kWaveformPoints>, kMaxWaveformStages> m_waveforms{};
};

} // namespace gsr::audio