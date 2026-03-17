//Additional includes go here
#include "window/Window.h"
#include "gui/GuiManager.h"
#include "gui/features/console/Console.h"
#include "network/Network.h"

#include "gab-tablesim/TableSim.h"
#include "gab-tablesim/gui/TableWindow.h"
#include "gab-tablesim/gui/HandWindow.h"

#include "system/AppConsoleRedirector.h"

int main(int argc, char** argv) {

    AppConsoleRedirector redirector;

    // Initialize Window + GUI
    app::Window window = app::Window("Card Game", 1280, 720);
    app::Console consolewindow = app::Console(redirector);

    //NETWORK
    app::Network network;

    //GAME SPECIFIC
    app::gab::TableSim tablesim;
    app::gab::HandWindow handwindow;
    app::gab::TableWindow tablewindow(tablesim.Get_table());

    //GUI
    app::GuiManager::WindowAssignmentOverride windowoverride = {
        .top_left = &tablewindow,
        .bottom_left = &handwindow,
        .bottom_right = &consolewindow
    };
    app::GuiManager guimanager(windowoverride);

    //MAIN
    bool done = false;
    while (!done) {
        network.Update();

        window.InitFrame();

        if (window.Get_should_quit())
            break;

        guimanager.Draw();

        window.CommitFrame();
    }

    return 0;
}
