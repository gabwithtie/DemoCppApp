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

	namespace internal {
		class BaseAsset_base {
		protected:
			AssetType assettype;
			std::filesystem::path asset_filepath;
			bool destroy_queued;
			BaseImportData base_import_data;
		public:
			inline std::string Get_assetId() {
				return this->base_import_data.asset_id;
			}
			AssetType Get_assettype();
			inline std::filesystem::path Get_asset_filepath() {
				return asset_filepath;
			}
		};
	}

	template<class TFinal, class TImportData>
	class BaseAsset : public internal::BaseAsset_base {
	protected:
		TImportData import_data;
	public:
		BaseAsset(std::filesystem::path asset_path) {
			app::Parser::PopulateClass(this->import_data, asset_path);

			this->asset_filepath = asset_path;

			std::string filename_with_ext = asset_path.filename().string();
			size_t dot_pos = filename_with_ext.find('.');
			if (dot_pos != std::string::npos)
				this->base_import_data.asset_id = filename_with_ext.substr(0, dot_pos);
			else
				this->base_import_data.asset_id = filename_with_ext;

			AssetLoader_base<TFinal, TImportData>::LoadFileAsset(static_cast<TFinal*>(this), this->import_data);
		}
		bool Get_destroy_queued() {
			return this->destroy_queued;
		}
		TImportData& Get_import_data() {
			return this->import_data;
		}
		inline static TFinal* GetAssetById(std::string id) {
			return AssetLoader_base<TFinal, TImportData>::GetAssetById(id);
		}
	};
}