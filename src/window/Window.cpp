#include "window.h"

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

namespace app {
	Window::Window(const char* name, int _w, int _h)
	{
        this->w = _w;
        this->h = _h;

        SDL_Init(SDL_INIT_VIDEO);

        this->window = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->w, this->h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        this->gl_context = SDL_GL_CreateContext(window);
        glewInit();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplSDL2_InitForOpenGL(this->window, this->gl_context);
        ImGui_ImplOpenGL3_Init("#version 330");
	}
    void Window::InitFrame()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) this->should_quit = true;
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }
    void Window::CommitFrame()
    {
        ImGui::Render();
        glViewport(0, 0, this->w, this->h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(this->window);
    }
    Window::~Window()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_GL_DeleteContext(this->gl_context);
        SDL_DestroyWindow(this->window);
        SDL_Quit();
    }
}

