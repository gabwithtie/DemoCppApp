#pragma once

#include <string>
#include <vector>

namespace picsel {

    struct VirtualFile {
        std::string name;              // User-facing unique display label
        std::string actual_png_name;   // Physical file target mapping in root/images/
    };

    struct VirtualFolder {
        std::string name;
        std::vector<VirtualFolder> subfolders;
        std::vector<VirtualFile> files;
    };

    struct ProjectData {
        std::string project_name = "Untitled Pixel Project";

        // Root entry of our implicit virtual structure layout tree
        VirtualFolder virtual_root{ "Root", {}, {} };
    };
}