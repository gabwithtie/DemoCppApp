#pragma once

#include "model/Project.hpp"

#include <filesystem>

namespace app {

class ProjectExporter {
public:
    static bool ExportProjectToPng(const Model::Project& project, const std::filesystem::path& outputPath);
};

} // namespace app
