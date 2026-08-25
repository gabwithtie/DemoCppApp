#include "window/Window.h"
#include "gui/GuiManager.h"

// Application Core
#include "app/App.hpp"

// Application GUI Extensions
#include "gui/MenuBarExtension.hpp"
#include "gui/timeline/TimelineWindow.hpp"
#include "gui/clip/ClipEditorWindow.hpp"
#include "gui/instrument/InstrumentWindow.hpp"
#include "app/gui/ProjectPicker.hpp"

#include <imgui.h>

int main(int argc, char** argv) {
    // 1. Initialize Native Window
    app::Window window = app::Window("GabApp", 1280, 720);

    // 2. Initialize Core App Lifecycle & State
    gsr::App app;
    if (!app.init()) {
        return -1;
    }

    // 3. Instantiate GUI Windows
    gsr::gui::TimelineWindow timelineWindow(app);
    gsr::gui::ClipEditorWindow clipEditorWindow(app);
    gsr::gui::InstrumentWindow instrumentWindow(app);

    gsr::gui::ProjectPicker projectPicker;
    bool projectSelected = false;

    gsr::MenuBarExtension menuBarExtension;

    // 4. Configure Layout Assignments via Designated Initializers
    app::GuiManager::WindowAssignmentOverride windowoverride = {
        .topLeft = {
            &clipEditorWindow,
        },
        .bottomLeft = {
            &timelineWindow,
            },
            .bottomRight = {
                &instrumentWindow
            }
    };
    app::GuiManager guimanager(windowoverride);

    guimanager.GetMenuBar().AddMenu(&menuBarExtension);

    // 5. Main Execution Loop
    while (!window.GetShouldQuit() && app.is_running) {
        window.InitFrame();

        // Evaluate transport timing & app calculations
        const float delta_time = ImGui::GetIO().DeltaTime;
        if (!projectSelected) {
            projectSelected = projectPicker.Draw();
            window.CommitFrame();
            continue;
        }

        app.process_input();
        app.update(delta_time);
        
        // Render ImGui dockspace and active windows
        guimanager.Draw();
        app.render_ui(); // additional app specific ui

        window.CommitFrame();
    }

    // 6. Cleanup
    app.shutdown();
    return 0;
}