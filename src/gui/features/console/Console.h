#pragma once

#include "gui/main/GuiWindow.h"
#include "system/AppConsoleRedirector.h"

namespace app {
	class Console : public GuiWindow {
		AppConsoleRedirector& redirector;
	public:
		std::string GetWindowId() override { return "Console"; }
		Console(AppConsoleRedirector& redirector);
	protected:
		void DrawSelf() override;
	};
}