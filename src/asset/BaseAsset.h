#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "asset/AssetLoading/AssetLoader.h"
#include "system/Parser.h"
#include "types/Types.h"

namespace fs = std::filesystem;

namespace app {
	struct BaseImportData {
		std::string asset_type;
		std::string asset_id;
	};

	
	class IBaseAsset {
		protected:
			AssetType assetType;
			std::filesystem::path assetFilepath;
			bool destroyQueued;
			BaseImportData baseImportData;
		public:
			inline std::string GetAssetId() {
				return this->baseImportData.asset_id;
			}
			AssetType GetAssetType();
			inline std::filesystem::path Get_assetFilepath() {
				return assetFilepath;
			}
	};

	template<class TFinal, class TImportData>
	class BaseAsset : public IBaseAsset {
	protected:
		TImportData importData;
	public:
		BaseAsset(std::filesystem::path asset_path) {
			app::Parser::PopulateClass(this->importData, asset_path);

			this->assetFilepath = asset_path;

			std::string filename_with_ext = asset_path.filename().string();
			size_t dot_pos = filename_with_ext.find('.');
			if (dot_pos != std::string::npos)
				this->baseImportData.asset_id = filename_with_ext.substr(0, dot_pos);
			else
				this->baseImportData.asset_id = filename_with_ext;

			IAssetLoader<TFinal, TImportData>::LoadFileAsset(static_cast<TFinal*>(this), this->importData);
		}
		bool GetDestroyed() {
			return this->destroyQueued;
		}
		TImportData& GetImportData() {
			return this->importData;
		}
		inline static TFinal* GetAssetById(std::string id) {
			return IAssetLoader<TFinal, TImportData>::GetAssetById(id);
		}
	};
}