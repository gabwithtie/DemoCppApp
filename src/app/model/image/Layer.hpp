#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace Model {

struct Layer {
    std::string name{"Layer"};
    bool visible{true};

    int width{0};
    int height{0};

    // Packed RGBA8 pixels in row-major order.
    std::vector<uint32_t> pixels{};

    static constexpr uint32_t Transparent = 0x00000000u;
    static constexpr uint32_t White = 0xFFFFFFFFu;

    bool IsInBounds(int x, int y) const {
        return x >= 0 && y >= 0 && x < width && y < height;
    }

    int PixelIndex(int x, int y) const {
        return y * width + x;
    }

    void Resize(int newWidth, int newHeight, uint32_t fillColor = Transparent) {
        width = std::max(0, newWidth);
        height = std::max(0, newHeight);
        pixels.assign(static_cast<size_t>(width) * static_cast<size_t>(height), fillColor);
    }

    void EnsureSize(int expectedWidth, int expectedHeight, uint32_t fillColor = Transparent) {
        if (width == expectedWidth && height == expectedHeight &&
            pixels.size() == static_cast<size_t>(std::max(0, expectedWidth)) * static_cast<size_t>(std::max(0, expectedHeight))) {
            return;
        }

        std::vector<uint32_t> resized(static_cast<size_t>(std::max(0, expectedWidth)) * static_cast<size_t>(std::max(0, expectedHeight)), fillColor);

        const int copyWidth = std::min(width, std::max(0, expectedWidth));
        const int copyHeight = std::min(height, std::max(0, expectedHeight));

        for (int y = 0; y < copyHeight; ++y) {
            for (int x = 0; x < copyWidth; ++x) {
                const int oldIndex = PixelIndex(x, y);
                const int newIndex = y * expectedWidth + x;
                if (oldIndex >= 0 && oldIndex < static_cast<int>(pixels.size()) && newIndex >= 0 && newIndex < static_cast<int>(resized.size())) {
                    resized[static_cast<size_t>(newIndex)] = pixels[static_cast<size_t>(oldIndex)];
                }
            }
        }

        width = std::max(0, expectedWidth);
        height = std::max(0, expectedHeight);
        pixels = std::move(resized);
    }

    void Clear(uint32_t color = Transparent) {
        std::fill(pixels.begin(), pixels.end(), color);
    }

    uint32_t GetPixel(int x, int y) const {
        if (!IsInBounds(x, y)) {
            return Transparent;
        }

        const int index = PixelIndex(x, y);
        if (index < 0 || index >= static_cast<int>(pixels.size())) {
            return Transparent;
        }

        return pixels[static_cast<size_t>(index)];
    }

    void SetPixel(int x, int y, uint32_t color) {
        if (!IsInBounds(x, y)) {
            return;
        }

        const int index = PixelIndex(x, y);
        if (index < 0 || index >= static_cast<int>(pixels.size())) {
            return;
        }

        pixels[static_cast<size_t>(index)] = color;
    }
};

} // namespace Model