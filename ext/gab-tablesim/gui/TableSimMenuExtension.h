#pragma once

#include "gui/features/menubar/MenuBarExtension.h"
#include <imgui.h>
#include <string>

#include "asset/assetloading/BatchLoader.h"
#include "system/FileDialogue.h"

#include "gab-tablesim/network/Decks.h"

#include <filesystem>

namespace app {
	class TableSimMenuExtension : public MenuBarExtension {
	public:
		inline void DrawMenuBarMenu() override {
			if (ImGui::BeginMenu("TableSim")) {
				if (ImGui::MenuItem("Load Deck From Folder")) {
					std::filesystem::path outPath = FileDialogue::GetFilePath(FileDialogue::FOLDER);

					if (outPath.has_filename()) {
						BatchLoader::GenerateMetafiles(outPath);
						const auto loaded_files = BatchLoader::LoadAssetsFromDirectory(outPath);

						gab::Decks::CreateObject(outPath.filename().string(), loaded_files);
					}
				}
				ImGui::EndMenu();
			}
		}
	};
}