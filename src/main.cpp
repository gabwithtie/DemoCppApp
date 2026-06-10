//Additional includes go here
#include "window/Window.h"
#include "gui/GuiManager.h"
#include "gui/features/console/Console.h"
#include "network/Network.h"
#include "system/AppConsoleRedirector.h"
#include "graphics/loaders/TextureLoader.h"

#include "picsel/picsel.h"

#include "picsel/gui/ProjectWindow.h"
#include "picsel/gui/ImageBrowser.h"
#include "picsel/gui/ViewportWindow.h"

int main(int argc, char** argv) {

    AppConsoleRedirector redirector;

    // Initialize Window + GUI
    app::Window window = app::Window("Picsel", 1280, 720);
    app::Console consolewindow = app::Console(redirector);

    //GRAPHICS
    app::TextureLoader textureloader;
    textureloader.AssignSelfAsLoader();

    //SPECIFIC CONSTRUCTOR
	auto projectWindow = new picsel::ProjectWindow();
	auto imageBrowser = new picsel::ImageBrowser();
    auto viewportWindow = new picsel::ViewportWindow();

    //GUI
    app::GuiManager::WindowAssignmentOverride windowoverride = {
        .topLeft = {
            viewportWindow
		},
        .topRght = {
            imageBrowser
        },
        .hiddens = {
            projectWindow
        }
    };
    app::GuiManager guimanager(windowoverride);

    //GAME SPECIFIC PRE-MAIN
    auto picsel_backend = picsel::Picsel();

    //MAIN
    bool done = false;
    while (!done) {
        window.InitFrame();

        if (window.GetShouldQuit())
            break;

        guimanager.Draw();

        window.CommitFrame();
    }

    return 0;
}
