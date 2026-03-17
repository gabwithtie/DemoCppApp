#pragma once

#include <functional>
#include <string>
#include <algorithm>
#include <filesystem>

#include <iostream>

#include "../types/Types.h"

namespace app {
	namespace internal {
		class BaseAsset_base;
	}

	extern app::internal::BaseAsset_base* GetBaseData(std::filesystem::path path);
	extern app::AssetType GetAssetType(std::filesystem::path path);
	extern std::string GetAssetId(std::filesystem::path path);

	class AssetLoader_base_base {
	public:
		/// <summary>
		/// 
		/// </summary>
		/// <returns>The count of remaining asynchronous load tasks.</returns>
		int virtual CheckAsynchrounousTasks() = 0;
		virtual internal::BaseAsset_base* FindAssetByPath(std::filesystem::path path) = 0;
		virtual internal::BaseAsset_base* FindAssetById(std::string id) = 0;
		virtual std::vector<std::string> GetAllAssetIds() = 0;
	};

	extern std::unordered_map<app::AssetType, AssetLoader_base_base*> all_asset_loaders;

	template<class TAsset, class TAssetImportData>
	class AssetLoader_base : public AssetLoader_base_base {
	protected:
		static AssetLoader_base* active_base_instance;
		std::unordered_map<std::string, TAsset*> fileasset_dictionary;

		std::function<bool(TAsset* asset, const TAssetImportData& import_data)> load_func;
	public:
		static bool LoadFileAsset(TAsset* asset, const TAssetImportData& import_data) {
			if (active_base_instance == nullptr)
				std::cout << "asset loader for this particular type is not assigned!" << std::endl;

			return active_base_instance->load_func(asset, import_data);
		}
		static TAsset* GetAssetById(std::string asset_id) {
			auto it = active_base_instance->fileasset_dictionary.find(asset_id);
			if (it != active_base_instance->fileasset_dictionary.end()) {
				return it->second;
			}

			return nullptr;
		}
		virtual std::vector<std::string> GetAllAssetIds() override {
			std::vector<std::string> ids;
			for (const auto& pair : active_base_instance->fileasset_dictionary) {
				ids.push_back(pair.first);
			}
			return ids;
		}

		internal::BaseAsset_base* FindAssetByPath(std::filesystem::path asset_path) override {
			for (const auto& pair : active_base_instance->fileasset_dictionary)
			{
				internal::BaseAsset_base* baseasset = dynamic_cast<internal::BaseAsset_base*>(pair.second);

				if (baseasset == nullptr)
					continue;

				if (baseasset->Get_asset_filepath() == asset_path)
					return pair.second;
			}

			return nullptr;
		}

		internal::BaseAsset_base* FindAssetById(std::string id) override {
			auto find_it = active_base_instance->fileasset_dictionary.find(id);

			if (find_it == active_base_instance->fileasset_dictionary.end())
				return nullptr;

			auto entry = dynamic_cast<internal::BaseAsset_base*>(find_it->second);;
			if (entry == nullptr)
				return nullptr;

			return entry;
		}
	};

	template<class TAsset, class TAssetImportData>
	AssetLoader_base<TAsset, TAssetImportData>* AssetLoader_base<TAsset, TAssetImportData>::active_base_instance = nullptr;

	template<class TAsset, class TAssetImportData, class TAssetLoadData>
	class AssetLoader : public AssetLoader_base<TAsset, TAssetImportData> {
	public:
		struct AsyncLoadTask {
			bool isDone = false;
			std::string id;
			std::string path;
			TAssetLoadData loaddata;
		};
	private:
		std::vector<AsyncLoadTask*> async_tasks;
	protected:
		static AssetLoader* active_instance;

		std::unordered_map<std::string, TAssetLoadData> loaded_assets;
		virtual void LoadAsset_(TAsset* asset, const TAssetImportData& import_data, TAssetLoadData* load_data) = 0;
		virtual void UnLoadAsset_(TAssetLoadData* load_data) = 0;

	public:

		inline void RegisterAsyncTask(AsyncLoadTask* task) {
			async_tasks.push_back(task);
		}

		inline virtual void OnAsyncTaskCompleted(AsyncLoadTask* task) = 0;

		inline int virtual CheckAsynchrounousTasks() override {
			std::vector<AsyncLoadTask*> checked;

			for (size_t i = 0; i < this->async_tasks.size(); i++)
			{
				auto& async_task = this->async_tasks[i];

				if (async_task->isDone) {
					OnAsyncTaskCompleted(async_task);
					checked.push_back(async_task);
					free(async_task);
				}
			}

			for (const auto& checkedptr : checked)
			{
				std::erase_if(this->async_tasks, [checkedptr](AsyncLoadTask* x) {
					return x == checkedptr;
					});
			}

			return static_cast<int>(this->async_tasks.size());
		}

		virtual void AssignSelfAsLoader() {
			this->active_base_instance = this;
			this->active_instance = this;

			this->load_func = [](TAsset* asset, const TAssetImportData& import_data) {
				TAssetLoadData load_data = {};
				active_instance->loaded_assets.insert_or_assign(asset->Get_assetId(), load_data);
				active_instance->LoadAsset_(asset, import_data, &active_instance->loaded_assets[asset->Get_assetId()]);

				auto it = active_instance->fileasset_dictionary.find(asset->Get_assetId());
				if (it != active_instance->fileasset_dictionary.end()) {
					//implement deloading logic for old asset
				}

				//Always override
				active_instance->fileasset_dictionary.insert_or_assign(asset->Get_assetId(), asset);

				return true;
				};
		}

		static std::unordered_map<std::string, TAssetLoadData>& GetDataMap() {
			return active_instance->loaded_assets;
		}

		static void Register(std::string id, TAssetLoadData assetdata) {
			active_instance->loaded_assets.insert_or_assign(id, assetdata);
		}

		static TAssetLoadData* GetAssetRuntimeData(std::string assetid) {
			auto it = active_instance->loaded_assets.find(assetid);
			if (it != active_instance->loaded_assets.end()) {
				return &it->second;
			}
			else {
				std::cout << "Asset not found. id: \"" + assetid + "\"" << std::endl;
				return nullptr;
			}
		}

		static TAsset* GetAssetByPath(std::string asset_path) {
			for (const auto& pair : active_instance->fileasset_dictionary) {
				if (pair.second->Get_asset_filepath() == asset_path) {
					return pair.second;
				}
			}

			std::cout << "Asset not found. path: \"" + asset_path + "\"" << std::endl;
			return nullptr;
		}

		static std::vector<internal::BaseAsset_base*> GetAssetList() {
			std::vector<internal::BaseAsset_base*> list;

			for (const auto& pair : active_instance->fileasset_dictionary)
			{
				list.push_back(pair.second);
			}

			return list;
		}
	};

	template<class TAsset, class TAssetImportData, class TAssetLoadData>
	AssetLoader<TAsset, TAssetImportData, TAssetLoadData>* AssetLoader<TAsset, TAssetImportData, TAssetLoadData>::active_instance = nullptr;
}