#include "MenuBar.h"

#include <imgui.h>

#include "../gui/features/directory/DirectoryBrowser.h"

namespace app {
	MenuBar* MenuBar::instance = nullptr;

	void MenuBar::DrawSelf()
	{
		if (ImGui::BeginMenu("Window")) {
			for (const auto& window : this->windows)
			{
				if (ImGui::MenuItem(window->GetWindowId().c_str())) {
					window->SetOpen(true);
				}
			}
			ImGui::EndMenu();
		}

		for (const auto& ext : extensions)
		{
			ext->DrawMenuBarMenu();
		}
	}

	void MenuBar::AddMenu(MenuBarExtension* _ext)
	{
		instance->extensions.push_back(_ext);
	}

	MenuBar::MenuBar(std::vector<GuiWindow*>& _windows) : windows(_windows) {
		instance = this;
	}

	bool MenuBar::ext_Begin()
	{
		if (ImGui::BeginMainMenuBar()) {
			return true;
		}
		else {
			return false;
		}
	}

	void MenuBar::ext_End()
	{
		ImGui::EndMainMenuBar();
	}
}