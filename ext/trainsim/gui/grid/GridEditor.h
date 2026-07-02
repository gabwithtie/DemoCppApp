#pragma once

#include "gui/main/GuiWindow.h"
#include "trainsim/grid/GridManager.h"
#include "common/CellRenderer.h"

namespace Trainsim {

    using namespace app;

    class GridEditor : public GuiWindow {
    private:
        GridManager& manager;

        // UI Layout State
        float left_pane_width = 300.0f;

        // --- Zoom and Pan State ---
        float base_cell_size = 20.0f;     // The reference cell size at 1.0x zoom
        float zoom_level = 1.0f;          // Current scale multiplier
        ImVec2 pan_offset = ImVec2(0.0f, 0.0f); // Offset vector in pixels

        // Inspector Buffer States
        char type_buffer[128] = "";
        char new_data_key[64] = "";
        float new_data_value = 0.0f;
        char message_type_buffer[128] = "";

        unsigned int GetCellColor(const std::string& type);

    public:
        std::string GetWindowId() override { return "Grid Simulation Editor"; }
        GridEditor(GridManager& gridManager);

    protected:
        void DrawSelf() override;
        void DrawInspectorPane();
        void DrawGridCanvasPane();

    public:
        // Exposes registration to external configuration systems
        void RegisterRenderer(const std::string& celltype, CellRenderer* renderer) {
            renderers[celltype] = renderer;
        }

    private:
        std::unordered_map<std::string, CellRenderer*> renderers;
    };

}