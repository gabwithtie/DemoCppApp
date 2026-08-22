#include "window/Window.h"
#include "gui/GuiManager.h"

//Application Specific Includes
#include "game/GameSimulation.hpp"
#include "game/map/TerminalMapWindow.hpp"
#include "game/train/TrainListWindow.hpp"
#include "game/finance/FinanceWindow.hpp"
#include "game/finance/FinanceManager.hpp"
#include "game/station/StationViewerWindow.hpp"

int main(int argc, char** argv) {

    // Initialize Window + GUI
    app::Window window = app::Window("GabApp", 1280, 720);

    //GAME STUFF
    app::GameSimulation simulation;

    app::TerminalMapWindow terminalMapWindow(simulation);
    app::TrainListWindow trainListWindow(simulation);
    app::FinanceWindow financeWindow(simulation.GetFinanceManager());
    app::StationViewerWindow stationViewerWindow(simulation);

    //GUI
    app::GuiManager::WindowAssignmentOverride windowoverride = {

        .showns = {
            {
                &terminalMapWindow
            },
            {
                &trainListWindow
            },
            {
                &financeWindow
            },
            {
                &stationViewerWindow
            }
        }
    };
    app::GuiManager guimanager(windowoverride);

    //GAME SPECIFIC PRE-MAIN


    //MAIN
    bool done = false;
    while (!done) {
        window.InitFrame();

        if (window.GetShouldQuit())
            break;

        guimanager.Draw();

        simulation.Update(0.0016f);

        window.CommitFrame();
    }

    return 0;
}
