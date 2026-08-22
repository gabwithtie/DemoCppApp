#pragma once

#include <cstdint>
#include <vector>
#include <mutex>

namespace gsr::midi {

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
    uint8_t channel{0};       // 0 - 15
    uint8_t data1{0};         // Note number (0-127) or CC controller ID
    uint8_t data2{0};         // Velocity (0-127) or CC value
    uint32_t frame_offset{0}; // Sub-buffer sample offset for sample-accurate timing

    // Helpers to create raw 3-byte MIDI commands
    [[nodiscard]] uint8_t GetStatusByte() const {
        return static_cast<uint8_t>(type) | (channel & 0x0F);
    }
};

class MidiInterface {
public:
    MidiInterface() = default;
    ~MidiInterface() = default;

    // Direct API methods for note & transport playback triggers
    void SendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint32_t frame_offset = 0);
    void SendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity = 0, uint32_t frame_offset = 0);
    void SendControlChange(uint8_t channel, uint8_t controller, uint8_t value, uint32_t frame_offset = 0);
    void SendPitchBend(uint8_t channel, uint16_t value, uint32_t frame_offset = 0);
    
    // Panic / Reset controls
    void SendAllNotesOff(uint8_t channel);
    void SendAllNotesOffAllChannels();

    // Raw message queuing
    void PushMessage(const MidiMessage& msg);

    // Buffer processing methods (used by Audio Synthesizer / Audio Thread)
    void FlushPendingMessages(std::vector<MidiMessage>& out_buffer);
    void ClearQueue();

    // Interface State Queries
    [[nodiscard]] size_t GetPendingMessageCount() const;

private:
    std::vector<MidiMessage> m_message_queue;
    mutable std::mutex m_queue_mutex;
};

} // namespace gsr::midi