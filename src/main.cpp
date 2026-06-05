//Additional includes go here
#include "window/Window.h"
#include "gui/GuiManager.h"
#include "gui/features/console/Console.h"
#include "network/Network.h"
#include "system/AppConsoleRedirector.h"
#include "graphics/loaders/TextureLoader.h"

#include "gab-tablesim/TableSim.h"
#include "gab-tablesim/gui/TableWindow.h"
#include "gab-tablesim/gui/DeckWindow.h"
#include "gab-tablesim/gui/HandWindow.h"
#include "gab-tablesim/gui/TableSimMenuExtension.h"


int main(int argc, char** argv) {

    AppConsoleRedirector redirector;

    // Initialize Window + GUI
    app::Window window = app::Window("Card Game", 1280, 720);
    app::Console consolewindow = app::Console(redirector);

    //GRAPHICS
    app::graphics::TextureLoader textureloader;
    textureloader.AssignSelfAsLoader();

    //SPECIFIC CONSTRUCTOR


    //GUI
    app::GuiManager::WindowAssignmentOverride windowoverride = {
    };
    app::GuiManager guimanager(windowoverride);

    //GAME SPECIFIC PRE-MAIN

    
    //MAIN
    bool done = false;
    while (!done) {
        window.InitFrame();

        if (window.Get_should_quit())
            break;

        guimanager.Draw();

        window.CommitFrame();
    }

    return 0;
}
