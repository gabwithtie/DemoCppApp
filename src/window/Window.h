#pragma once

#include <string>

#include <SDL2/SDL.h>

namespace app {
	class Window {
	private:
		SDL_Window* window;
		SDL_GLContext gl_context;
		bool shouldQuit = false;
		int w;
		int h;

	public:
		Window(const char* name, int w, int h);
		void InitFrame();
		void CommitFrame();
		inline bool GetShouldQuit() { // Getters and setters are usually "inline" and have their definition directly in the header
			return this->shouldQuit;
		}
		~Window(); // Destructor automatically called when local variable (i.e., non-pointer variable goes out of scope)
	};
}