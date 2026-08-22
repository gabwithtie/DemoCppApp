#pragma once

#include "GuiElement.h"

#include <string>

#include <list>
#include <vector>

#include "../gui/features/menuBar/MenuBarExtension.h"

namespace app {
	class GuiWindow;

	class MenuBar : public GuiElement {
	private:
		static MenuBar* instance;

		bool ext_Begin() override;
		void ext_End() override;

		std::vector<GuiWindow*>& windows;
		std::vector<MenuBarExtension*> extensions;
	public:
		void DrawSelf() override;

		static void AddMenu(MenuBarExtension*);

		MenuBar(std::vector<GuiWindow*>& _windows);
	};
}