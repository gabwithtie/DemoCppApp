#pragma once

#include "SerializationIncludes.hpp"

#include <algorithm>
#include <mutex>
#include <vector>

namespace gsr::audio {


enum class MidiMessageType : uint8_t {
    NoteOff               = 0x80,
    NoteOn                = 0x90,
    PolyphonicAftertouch = 0xA0,
    ControlChange         = 0xB0,
    ProgramChange         = 0xC0,
    ChannelAftertouch     = 0xD0,
    PitchBend             = 0xE0,
    System                = 0xF0
};

struct MidiMessage {
    MidiMessageType type{MidiMessageType::NoteOn};
    uint8_t channel{0};
    uint8_t data1{0};
    uint8_t data2{0};
    uint32_t frame_offset{0};
};

} // namespace gsr::audio

namespace gsr::audio {

// Inherit from ISerializable for dynamic reflection & property drawing
class IInstrument : public gbe::ISerializable {
public:
    IInstrument() = default;
    explicit IInstrument(gbe::SerializedData& data) : gbe::ISerializable(data) {}
    ~IInstrument() = default;

    void SendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint32_t frame_offset = 0) {
        PushMessage({(velocity > 0) ? MidiMessageType::NoteOn : MidiMessageType::NoteOff,
                     std::clamp<uint8_t>(channel, 0, 15),
                     std::clamp<uint8_t>(note, 0, 127),
                     std::clamp<uint8_t>(velocity, 0, 127),
                     frame_offset});
    }

    void SendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity = 0, uint32_t frame_offset = 0) {
        PushMessage({MidiMessageType::NoteOff,
                     std::clamp<uint8_t>(channel, 0, 15),
                     std::clamp<uint8_t>(note, 0, 127),
                     std::clamp<uint8_t>(velocity, 0, 127),
                     frame_offset});
    }

    void PushMessage(const MidiMessage& msg) {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_midi_queue.push_back(msg);
    }

    void ProcessAndRender(float* output_buffer, size_t num_frames, double sample_rate) {
        m_render_events.clear();
        {
            std::lock_guard<std::mutex> lock(m_queue_mutex);
            m_render_events.swap(m_midi_queue);
        }
        RenderAudioBlock(output_buffer, num_frames, sample_rate, m_render_events);
    }

protected:
    virtual void RenderAudioBlock(
        float* output_buffer,
        size_t num_frames,
        double sample_rate,
        const std::vector<MidiMessage>& midi_events
    ) = 0;

private:
    std::vector<MidiMessage> m_midi_queue;
    std::vector<MidiMessage> m_render_events;
    std::mutex m_queue_mutex;
};

} // namespace gsr::audio