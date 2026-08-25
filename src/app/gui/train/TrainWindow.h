#pragma once

#include "../../../gui/main/GuiWindow.h"

#include <cstddef>
#include <string>

namespace gsr {
    class TrainWindow : public app::GuiWindow {
    private:
        float zoom_level = 1.0f;

        void HandleZoom();
        void HandleDragging();
        void DrawCarriages();
        void AddCarriage(const std::string& type);

        bool CanCreateHeadCarriage() const;
        bool CanAddCarriage(const std::string& type) const;
        bool CanRemoveCarriage(std::size_t carriage_index) const;
        bool CanAddFurniture(std::size_t carriage_index, std::size_t slot_index, const std::string& type) const;
        bool CanRemoveFurniture(std::size_t carriage_index, std::size_t slot_index) const;

    protected:
        void DrawSelf() override;

    public:
        std::string GetWindowId() override { return "Train Simulator"; }
    };
}