#pragma once

#include "../gui/main/GuiWindow.h"
#include "App.hpp"
#include <imgui.h>

namespace gsr::gui {

class InstrumentWindow : public app::GuiWindow {
public:
    explicit InstrumentWindow(gsr::App& app);
    ~InstrumentWindow() = default;

    std::string GetWindowId() override { return "Instrument Inspector"; }
    void DrawSelf() override;

private:
    gsr::App& m_app;
    Model::Track* GetSelectedTrack();
};

} // namespace gsr::gui