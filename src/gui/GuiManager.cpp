#include "GuiManager.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace app {
	GuiManager::GuiManager() : menubar (this->windows)
	{
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	}
	void GuiManager::Draw()
	{
		ImGuiID dockspace_id = ImGui::GetID("maindockspace");
		ImGui::DockSpaceOverViewport(dockspace_id);

		if (!gui_startframe_init) {
			gui_startframe_init = true;

			ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
			ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node

			// start [CODE]
			this->directorybrowser_window.Set_is_open(true);
			ImGui::DockBuilderDockWindow(this->directorybrowser_window.GetWindowId().c_str(), dockspace_id);
			// end [CODE]

			ImGui::DockBuilderFinish(dockspace_id);
		}

		this->menubar.Draw();

		for (const auto& window : this->windows)
		{
			if (window->Get_is_open())
				window->Draw();
		}
	}
}