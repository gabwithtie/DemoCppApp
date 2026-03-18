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

    //NETWORK
    app::Network network;

    //GAME SPECIFIC
    app::gab::TableSim tablesim;
    app::gab::HandWindow handwindow;
    app::gab::DeckWindow deckwindow;
    app::gab::TableWindow tablewindow;

    app::TableSimMenuExtension tablesimmenu;

    //GUI
    app::GuiManager::WindowAssignmentOverride windowoverride = {
        .top_left = {&tablewindow},
        .top_right = {&deckwindow},
        .bottom_left = {&handwindow },
        .bottom_right = {&consolewindow},
    };
    app::GuiManager guimanager(windowoverride);

    //GAME SPECIFIC PRE-MAIN
    app::MenuBar::AddMenu(&tablesimmenu);

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
