#pragma once

#include "GuiElement.h"

#include <string>

#include <imgui.h>
#include <list>
#include <vector>

namespace app {
	class GuiWindow;

	class MenuBar : public GuiElement {
	private:
		bool ext_Begin() override;
		void ext_End() override;

		std::vector<GuiWindow*>& windows;
	public:
		void DrawSelf() override;

		inline MenuBar(std::vector<GuiWindow*>& _windows) : windows(_windows) {
		}
	};
}