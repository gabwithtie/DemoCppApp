#include "ProjectExporter.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace {

static uint32_t AlphaCompositeOver(uint32_t dst, uint32_t src) {
    const float sr = static_cast<float>((src >> 24) & 0xFFu) / 255.0f;
    const float sg = static_cast<float>((src >> 16) & 0xFFu) / 255.0f;
    const float sb = static_cast<float>((src >> 8) & 0xFFu) / 255.0f;
    const float sa = static_cast<float>(src & 0xFFu) / 255.0f;

    const float dr = static_cast<float>((dst >> 24) & 0xFFu) / 255.0f;
    const float dg = static_cast<float>((dst >> 16) & 0xFFu) / 255.0f;
    const float db = static_cast<float>((dst >> 8) & 0xFFu) / 255.0f;
    const float da = static_cast<float>(dst & 0xFFu) / 255.0f;

    const float outA = sa + da * (1.0f - sa);
    if (outA <= 0.0f) {
        return Model::Layer::Transparent;
    }

    const float outR = (sr * sa + dr * da * (1.0f - sa)) / outA;
    const float outG = (sg * sa + dg * da * (1.0f - sa)) / outA;
    const float outB = (sb * sa + db * da * (1.0f - sa)) / outA;

    const uint8_t r = static_cast<uint8_t>(std::clamp(outR, 0.0f, 1.0f) * 255.0f);
    const uint8_t g = static_cast<uint8_t>(std::clamp(outG, 0.0f, 1.0f) * 255.0f);
    const uint8_t b = static_cast<uint8_t>(std::clamp(outB, 0.0f, 1.0f) * 255.0f);
    const uint8_t a = static_cast<uint8_t>(std::clamp(outA, 0.0f, 1.0f) * 255.0f);

    return (static_cast<uint32_t>(r) << 24) |
           (static_cast<uint32_t>(g) << 16) |
           (static_cast<uint32_t>(b) << 8) |
           static_cast<uint32_t>(a);
}

} // namespace

namespace app {

bool ProjectExporter::ExportProjectToPng(const Model::Project& project, const std::filesystem::path& outputPath) {
    if (project.w <= 0 || project.h <= 0) {
        return false;
    }

    const size_t pixelCount = static_cast<size_t>(project.w) * static_cast<size_t>(project.h);
    std::vector<uint32_t> composite(pixelCount, Model::Layer::Transparent);

    for (const auto& layer : project.layers) {
        if (!layer.visible) {
            continue;
        }
        if (layer.width != project.w || layer.height != project.h) {
            continue;
        }
        if (layer.pixels.size() != pixelCount) {
            continue;
        }

        for (size_t i = 0; i < pixelCount; ++i) {
            composite[i] = AlphaCompositeOver(composite[i], layer.pixels[i]);
        }
    }

    std::vector<uint8_t> rgba;
    rgba.reserve(pixelCount * 4);
    for (const uint32_t px : composite) {
        rgba.push_back(static_cast<uint8_t>((px >> 24) & 0xFFu));
        rgba.push_back(static_cast<uint8_t>((px >> 16) & 0xFFu));
        rgba.push_back(static_cast<uint8_t>((px >> 8) & 0xFFu));
        rgba.push_back(static_cast<uint8_t>(px & 0xFFu));
    }

    const int result = stbi_write_png(
        outputPath.string().c_str(),
        project.w,
        project.h,
        4,
        rgba.data(),
        project.w * 4
    );

    return result != 0;
}

} // namespace app
