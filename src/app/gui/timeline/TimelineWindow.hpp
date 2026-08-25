#pragma once

#include "../gui/main/GuiWindow.h"
#include "App.hpp"
#include "ClipManager.hpp"

namespace gsr::gui {

class TimelineWindow : public app::GuiWindow {
public:
    explicit TimelineWindow(gsr::App& app);
    ~TimelineWindow() = default;

    std::string GetWindowId() override { return "Timeline"; }
    void DrawSelf() override;

private:
    gsr::App& m_app;
    ClipManager m_clip_manager;

    // Track active key handle across drag frames without index swapping bugs
    Model::TimeKey* m_dragging_key{nullptr};

    void DrawTimeKeyBar();
};

} // namespace gsr::gui