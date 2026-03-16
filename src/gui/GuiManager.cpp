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
			
			ImGuiID dock_main_id = dockspace_id;
			ImGuiID dock_id_top, dock_id_bottom;
			ImGuiID dock_id_top_left, dock_id_top_right;
			ImGuiID dock_id_bottom_left, dock_id_bottom_right;

			dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.6f, NULL, &dock_id_top);
			dock_id_top_right = ImGui::DockBuilderSplitNode(dock_id_top, ImGuiDir_Right, 0.7f, NULL, &dock_id_top_left);
			dock_id_bottom_right = ImGui::DockBuilderSplitNode(dock_id_bottom, ImGuiDir_Right, 0.7f, NULL, &dock_id_bottom_left);

			// start [CODE]
			this->networkwindow.Set_is_open(true);
			this->chatwindow.Set_is_open(true);

			ImGui::DockBuilderDockWindow(this->networkwindow.GetWindowId().c_str(), dock_id_top_right);
			ImGui::DockBuilderDockWindow(this->chatwindow.GetWindowId().c_str(), dock_id_bottom_right);
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