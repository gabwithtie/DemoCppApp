#include "MenuBar.h"

#include "system/FileDialogue.h"

#include "gui/features/directory/DirectoryBrowser.h"

namespace app {
	void MenuBar::DrawSelf()
	{
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Load Folder")) {
				std::string outPath = FileDialogue::GetFilePath(FileDialogue::OPEN, "root.gbe");

				if (outPath.size() != 0) {
					DirectoryBrowser::SetProjectDirectory(outPath);
				}
			}
		}
		if (ImGui::BeginMenu("Window")) {
			for (const auto& window : this->windows)
			{
				if (ImGui::MenuItem(window->GetWindowId().c_str())) {
					window->Set_is_open(true);
				}
			}
			ImGui::EndMenu();
		}
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