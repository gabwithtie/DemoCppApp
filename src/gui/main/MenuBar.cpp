#include "MenuBar.h"
#include "MenuBar.h"
#include "MenuBar.h"

#include "system/FileDialogue.h"

#include "gui/features/directory/DirectoryBrowser.h"
#include "asset/assetloading/BatchLoader.h"

namespace app {
	MenuBar* MenuBar::instance = nullptr;

	void MenuBar::DrawSelf()
	{
		if (ImGui::BeginMenu("Window")) {
			for (const auto& window : this->windows)
			{
				if (ImGui::MenuItem(window->GetWindowId().c_str())) {
					window->Set_is_open(true);
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