#include <SDL2/SDL.h>
#include <GL/glew.h>
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

//Additional includes go here
#include "window/window.h"
#include "gui/GuiManager.h"

int main(int argc, char** argv) {

    // Initialize Window + GUI
    const char* window_name = "ImGui App";
    int w = 1280;
    int h = 720;
    SDL_Window* window;
    SDL_GLContext gl_context;
    {
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        gl_context = SDL_GL_CreateContext(window);
        glewInit();
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    // start [CODE]
    
    // end [CODE]
    
    app::GuiManager gui_manager;

    bool done = false;

    while (!done) {
        // Start Frame
        {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT) done = true;
            }
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
        }

        gui_manager.Draw();

        // start [CODE]
        
        // end [CODE]

        // Commit Frame
        {
            ImGui::Render();
            glViewport(0, 0, 1280, 720);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            SDL_GL_SwapWindow(window);
        }
    }

    // Cleanup
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    return 0;
}
