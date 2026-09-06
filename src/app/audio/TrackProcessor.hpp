#pragma once

#include "IInstrument.hpp"
#include "effects/EffectFactory.hpp"
#include "../model/Track.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <memory>
#include <string>
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
        const size_t sample_count = num_frames * 2;
        EnsureBufferSize(m_scratch_buffer, sample_count);
        EnsureBufferSize(m_source_buffer, sample_count);

        if (m_instrument) {
            m_instrument->ProcessAndRender(m_source_buffer.data(), num_frames, sample_rate);
        } else {
            std::fill_n(m_source_buffer.data(), sample_count, 0.0f);
        }

        std::copy_n(m_source_buffer.data(), sample_count, m_scratch_buffer.data());
        CaptureWaveform(0, num_frames, m_scratch_buffer);
        ProcessGraphRecursive(graph, num_frames, sample_rate);

        // Mix track scratch buffer into master output with volume control
        for (size_t i = 0; i < sample_count; ++i) {
            mix_output_buffer[i] += m_scratch_buffer[i] * track_volume;
        }
    }

private:
    static constexpr size_t kWaveformPoints = 128;
    static constexpr size_t kMaxWaveformStages = 32;

    struct EffectSlot {
        std::string name;
        std::unique_ptr<effects::Effect> instance;
    };

    static void EnsureBufferSize(std::vector<float>& buffer, size_t sample_count) {
        if (buffer.size() != sample_count) {
            buffer.assign(sample_count, 0.0f);
        }
    }

    void CaptureWaveform(size_t stage, size_t num_frames, const std::vector<float>& source_buffer) {
        if (num_frames == 0 || source_buffer.size() < 2) {
            return;
        }
        const size_t index = std::min(stage, kMaxWaveformStages - 1);
        for (size_t point = 0; point < kWaveformPoints; ++point) {
            const size_t frame = point * num_frames / kWaveformPoints;
            const size_t sample = std::min(frame, num_frames - 1) * 2;
            m_waveforms[index][point].store((source_buffer[sample] + source_buffer[sample + 1]) * 0.5f,
                                            std::memory_order_relaxed);
        }
    }

    effects::Effect* GetOrCreateEffect(app::NodeId node_id, const std::string& effect_name) {
        auto& slot = m_effects_by_node[node_id];
        if (!slot.instance || slot.name != effect_name) {
            slot.name = effect_name;
            slot.instance = effects::CreateEffectByName(effect_name);
        }
        return slot.instance.get();
    }

    const std::vector<float>* RenderNodeRecursive(
        app::NodeId node_id,
        const std::unordered_map<app::NodeId, const app::GraphNodeData*>& nodes,
        const std::unordered_map<app::PinId, app::NodeId>& pin_to_node,
        const std::unordered_map<app::PinId, std::vector<app::PinId>>& incoming_by_input_pin,
        std::unordered_map<app::NodeId, std::vector<float>>& rendered_cache,
        std::unordered_set<app::NodeId>& recursion_stack,
        size_t num_frames,
        double sample_rate,
        size_t& waveform_stage) {
        const auto cache_it = rendered_cache.find(node_id);
        if (cache_it != rendered_cache.end()) {
            return &cache_it->second;
        }

        if (!recursion_stack.insert(node_id).second) {
            return nullptr;
        }

        const auto node_it = nodes.find(node_id);
        if (node_it == nodes.end()) {
            recursion_stack.erase(node_id);
            return nullptr;
        }

        const app::GraphNodeData& node = *node_it->second;
        const size_t sample_count = num_frames * 2;
        auto& output = rendered_cache[node_id];
        output.assign(sample_count, 0.0f);

        if (node.type_name == "Instrument") {
            std::copy_n(m_source_buffer.data(), sample_count, output.data());
        } else {
            for (const auto& input_pin : node.inputs) {
                const auto links_it = incoming_by_input_pin.find(input_pin.id);
                if (links_it == incoming_by_input_pin.end()) {
                    continue;
                }

                for (app::PinId upstream_pin : links_it->second) {
                    const auto upstream_node_it = pin_to_node.find(upstream_pin);
                    if (upstream_node_it == pin_to_node.end()) {
                        continue;
                    }

                    const std::vector<float>* upstream_audio = RenderNodeRecursive(
                        upstream_node_it->second,
                        nodes,
                        pin_to_node,
                        incoming_by_input_pin,
                        rendered_cache,
                        recursion_stack,
                        num_frames,
                        sample_rate,
                        waveform_stage);

                    if (!upstream_audio) {
                        continue;
                    }

                    for (size_t sample = 0; sample < sample_count; ++sample) {
                        output[sample] += (*upstream_audio)[sample];
                    }
                }
            }
        }

        if (node.type_name == "Effect" && node.enabled) {
            if (effects::Effect* effect = GetOrCreateEffect(node.id, node.title)) {
                effect->Process(output, num_frames, sample_rate, node);
                CaptureWaveform(waveform_stage++, num_frames, output);
            }
        }

        recursion_stack.erase(node_id);
        return &output;
    }

    void ProcessGraphRecursive(const app::GraphData& graph, size_t num_frames, double sample_rate) {
        if (graph.nodes.empty()) {
            std::fill(m_scratch_buffer.begin(), m_scratch_buffer.end(), 0.0f);
            return;
        }

        std::unordered_map<app::PinId, app::NodeId> pin_nodes;
        std::unordered_map<app::NodeId, const app::GraphNodeData*> nodes;
        std::unordered_map<app::PinId, std::vector<app::PinId>> incoming_pins;

        for (const auto& node : graph.nodes) {
            nodes[node.id] = &node;
            for (const auto& pin : node.outputs) pin_nodes[pin.id] = node.id;
            for (const auto& pin : node.inputs) pin_nodes[pin.id] = node.id;
        }

        for (const auto& link : graph.links) {
            incoming_pins[link.end_pin_id].push_back(link.start_pin_id);
        }

        const app::GraphNodeData* output = nullptr;
        for (const auto& node : graph.nodes) {
            if (node.type_name == "TrackOutput") output = &node;
        }

        if (!output) {
            std::fill(m_scratch_buffer.begin(), m_scratch_buffer.end(), 0.0f);
            return;
        }

        std::unordered_map<app::NodeId, std::vector<float>> rendered_cache;
        std::unordered_set<app::NodeId> recursion_stack;
        size_t stage = 1;

        const std::vector<float>* output_audio = RenderNodeRecursive(
            output->id,
            nodes,
            pin_nodes,
            incoming_pins,
            rendered_cache,
            recursion_stack,
            num_frames,
            sample_rate,
            stage);

        if (!output_audio) {
            std::fill(m_scratch_buffer.begin(), m_scratch_buffer.end(), 0.0f);
            return;
        }

        const size_t sample_count = num_frames * 2;
        for (size_t sample = 0; sample < sample_count; ++sample) {
            m_scratch_buffer[sample] = (*output_audio)[sample];
        }
    }

    std::unique_ptr<IInstrument> m_instrument;
    std::vector<float> m_scratch_buffer; // Reusable buffer to avoid heap allocations in audio loop
    std::vector<float> m_source_buffer;
    std::unordered_map<app::NodeId, EffectSlot> m_effects_by_node;
    std::array<std::array<std::atomic<float>, kWaveformPoints>, kMaxWaveformStages> m_waveforms{};
};

} // namespace gsr::audio