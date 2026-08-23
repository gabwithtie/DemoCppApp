#pragma once

#include <algorithm>
#include <cstdint>
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

class IInstrument {
public:
    virtual ~IInstrument() = default;

    // Direct MIDI API methods
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

    void SendControlChange(uint8_t channel, uint8_t controller, uint8_t value, uint32_t frame_offset = 0) {
        PushMessage({MidiMessageType::ControlChange,
                     std::clamp<uint8_t>(channel, 0, 15),
                     std::clamp<uint8_t>(controller, 0, 127),
                     std::clamp<uint8_t>(value, 0, 127),
                     frame_offset});
    }

    void SendAllNotesOff(uint8_t channel = 0) {
        SendControlChange(channel, 123, 0);
    }

    void PushMessage(const MidiMessage& msg) {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_midi_queue.push_back(msg);
    }

    // Audio Engine Process Call
    void ProcessAndRender(float* output_buffer, size_t num_frames, double sample_rate) {
        m_render_events.clear();
        {
            std::lock_guard<std::mutex> lock(m_queue_mutex);
            m_render_events.swap(m_midi_queue); // Lock-free swap for rendering execution
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
    std::vector<MidiMessage> m_render_events; // Double-buffered to avoid allocations in DSP loop
    std::mutex m_queue_mutex;
};

} // namespace gsr::audio