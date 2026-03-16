#pragma once

#include "gui/main/GuiWindow.h"

namespace app {
	class NetworkWindow : public GuiWindow {
	public:
		std::string GetWindowId() override { return "Network"; }
	protected:
		void DrawSelf() override;
	};
}