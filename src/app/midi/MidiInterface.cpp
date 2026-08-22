#include "MidiInterface.hpp"
#include <algorithm>

namespace gsr::midi {

void MidiInterface::SendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint32_t frame_offset) {
    MidiMessage msg;
    msg.type = (velocity > 0) ? MidiMessageType::NoteOn : MidiMessageType::NoteOff;
    msg.channel = std::clamp<uint8_t>(channel, 0, 15);
    msg.data1 = std::clamp<uint8_t>(note, 0, 127);
    msg.data2 = std::clamp<uint8_t>(velocity, 0, 127);
    msg.frame_offset = frame_offset;

    PushMessage(msg);
}

void MidiInterface::SendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint32_t frame_offset) {
    MidiMessage msg;
    msg.type = MidiMessageType::NoteOff;
    msg.channel = std::clamp<uint8_t>(channel, 0, 15);
    msg.data1 = std::clamp<uint8_t>(note, 0, 127);
    msg.data2 = std::clamp<uint8_t>(velocity, 0, 127);
    msg.frame_offset = frame_offset;

    PushMessage(msg);
}

void MidiInterface::SendControlChange(uint8_t channel, uint8_t controller, uint8_t value, uint32_t frame_offset) {
    MidiMessage msg;
    msg.type = MidiMessageType::ControlChange;
    msg.channel = std::clamp<uint8_t>(channel, 0, 15);
    msg.data1 = std::clamp<uint8_t>(controller, 0, 127);
    msg.data2 = std::clamp<uint8_t>(value, 0, 127);
    msg.frame_offset = frame_offset;

    PushMessage(msg);
}

void MidiInterface::SendPitchBend(uint8_t channel, uint16_t value, uint32_t frame_offset) {
    uint16_t clamped_val = std::clamp<uint16_t>(value, 0, 16383);
    
    MidiMessage msg;
    msg.type = MidiMessageType::PitchBend;
    msg.channel = std::clamp<uint8_t>(channel, 0, 15);
    msg.data1 = static_cast<uint8_t>(clamped_val & 0x7F);        // LSB
    msg.data2 = static_cast<uint8_t>((clamped_val >> 7) & 0x7F); // MSB
    msg.frame_offset = frame_offset;

    PushMessage(msg);
}

void MidiInterface::SendAllNotesOff(uint8_t channel) {
    // Standard MIDI CC 123: All Notes Off
    SendControlChange(channel, 123, 0);
}

void MidiInterface::SendAllNotesOffAllChannels() {
    for (uint8_t ch = 0; ch < 16; ++ch) {
        SendAllNotesOff(ch);
    }
}

void MidiInterface::PushMessage(const MidiMessage& msg) {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    m_message_queue.push_back(msg);
}

void MidiInterface::FlushPendingMessages(std::vector<MidiMessage>& out_buffer) {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    if (m_message_queue.empty()) {
        return;
    }

    out_buffer.insert(out_buffer.end(), m_message_queue.begin(), m_message_queue.end());
    m_message_queue.clear();
}

void MidiInterface::ClearQueue() {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    m_message_queue.clear();
}

size_t MidiInterface::GetPendingMessageCount() const {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    return m_message_queue.size();
}

} // namespace gsr::midi