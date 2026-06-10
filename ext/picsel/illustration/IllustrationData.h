#pragma once

#include <string>
#include <vector>

namespace picsel {
    struct IllustrationData { //keep data simple such that it is implicitly serializable by Glaze
        std::string name;
        std::vector<std::string> layerFilenames;

        std::vector<bool> layerVisibility; // Parallel vector for visibility states
        int activeLayerIdx = 0;
        int width = 128;
        int height = 128;

        //other data to add in the future
    };
}