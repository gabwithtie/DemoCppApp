#pragma once

#include "gui/features/menubar/MenuBarExtension.h"
#include <imgui.h>
#include <string>

#include "asset/assetloading/BatchLoader.h"
#include "system/FileDialogue.h"

namespace app {
	class TableSimMenuExtension : public MenuBarExtension {
	public:
		inline void DrawMenuBarMenu() override {
			if (ImGui::BeginMenu("TableSim")) {
				if (ImGui::MenuItem("Load Deck From Folder")) {
					std::string outPath = FileDialogue::GetFilePath(FileDialogue::FOLDER);

					if (outPath.size() != 0) {
						BatchLoader::GenerateMetafiles(outPath);
						BatchLoader::LoadAssetsFromDirectory(outPath);
					}
				}
				ImGui::EndMenu();
			}
		}
	};
}