#pragma once

#include "model/Track.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace gsr::gui {

struct ImportedMidi {
    uint16_t ppq{0};
    std::vector<Model::Note> notes;
};

bool ImportMidiFile(const std::string& path, ImportedMidi& imported);

} // namespace gsr::gui