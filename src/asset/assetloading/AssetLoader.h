#pragma once

#include <functional>
#include <string>
#include <algorithm>
#include <filesystem>

#include <iostream>

#include "../types/Types.h"

namespace app {
	
	class IBaseAsset;

	extern app::IBaseAsset* GetBaseData(std::filesystem::path path);
	extern app::AssetType GetAssetType(std::filesystem::path path);
	extern std::string GetAssetId(std::filesystem::path path);

	class IAssetCollection {
	public:
		/// <summary>
		/// 
		/// </summary>
		/// <returns>The count of remaining asynchronous load tasks.</returns>
		int virtual CheckAsynchrounousTasks() = 0;
		virtual IBaseAsset* FindAssetByPath(std::filesystem::path path) = 0;
		virtual IBaseAsset* FindAssetById(std::string id) = 0;
		virtual std::vector<std::string> GetAllAssetIds() = 0;
	};

	extern std::unordered_map<app::AssetType, IAssetCollection*> all_asset_loaders;

	template<class TAsset, class TAssetImportData>
	class IAssetLoader : public IAssetCollection {
	protected:
		static IAssetLoader* activeBaseInstance;
		std::unordered_map<std::string, TAsset*> fileassetDictionary;

		std::function<bool(TAsset* asset, const TAssetImportData& import_data)> load_func;
	public:
		static bool LoadFileAsset(TAsset* asset, const TAssetImportData& import_data) {
			if (activeBaseInstance == nullptr)
				std::cout << "asset loader for this particular type is not assigned!" << std::endl;

			return activeBaseInstance->load_func(asset, import_data);
		}
		static TAsset* GetAssetById(std::string asset_id) {
			auto it = activeBaseInstance->fileassetDictionary.find(asset_id);
			if (it != activeBaseInstance->fileassetDictionary.end()) {
				return it->second;
			}

			return nullptr;
		}
		virtual std::vector<std::string> GetAllAssetIds() override {
			std::vector<std::string> ids;
			for (const auto& pair : activeBaseInstance->fileassetDictionary) {
				ids.push_back(pair.first);
			}
			return ids;
		}

		IBaseAsset* FindAssetByPath(std::filesystem::path asset_path) override {
			for (const auto& pair : activeBaseInstance->fileassetDictionary)
			{
				IBaseAsset* baseasset = dynamic_cast<IBaseAsset*>(pair.second);

				if (baseasset == nullptr)
					continue;

				if (baseasset->Get_asset_filepath() == asset_path)
					return pair.second;
			}

			return nullptr;
		}

		IBaseAsset* FindAssetById(std::string id) override {
			auto find_it = activeBaseInstance->fileassetDictionary.find(id);

			if (find_it == activeBaseInstance->fileassetDictionary.end())
				return nullptr;

			auto entry = dynamic_cast<IBaseAsset*>(find_it->second);;
			if (entry == nullptr)
				return nullptr;

			return entry;
		}
	};

	template<class TAsset, class TAssetImportData>
	IAssetLoader<TAsset, TAssetImportData>* IAssetLoader<TAsset, TAssetImportData>::activeBaseInstance = nullptr;

	template<class TAsset, class TAssetImportData, class TAssetLoadData>
	class AssetLoader : public IAssetLoader<TAsset, TAssetImportData> {
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
		static AssetLoader* activeInstance;

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
			this->activeBaseInstance = this;
			this->activeInstance = this;

			this->load_func = [](TAsset* asset, const TAssetImportData& import_data) {
				TAssetLoadData load_data = {};
				activeInstance->loaded_assets.insert_or_assign(asset->Get_assetId(), load_data);
				activeInstance->LoadAsset_(asset, import_data, &activeInstance->loaded_assets[asset->Get_assetId()]);

				auto it = activeInstance->fileassetDictionary.find(asset->Get_assetId());
				if (it != activeInstance->fileassetDictionary.end()) {
					//implement deloading logic for old asset
				}

				//Always override
				activeInstance->fileassetDictionary.insert_or_assign(asset->Get_assetId(), asset);

				return true;
				};
		}

		static std::unordered_map<std::string, TAssetLoadData>& GetDataMap() {
			return activeInstance->loaded_assets;
		}

		static void Register(std::string id, TAssetLoadData assetdata) {
			activeInstance->loaded_assets.insert_or_assign(id, assetdata);
		}

		static TAssetLoadData* GetAssetRuntimeData(std::string assetid) {
			auto it = activeInstance->loaded_assets.find(assetid);
			if (it != activeInstance->loaded_assets.end()) {
				return &it->second;
			}
			else {
				std::cout << "Asset not found. id: \"" + assetid + "\"" << std::endl;
				return nullptr;
			}
		}

		static TAsset* GetAssetByPath(std::string asset_path) {
			for (const auto& pair : activeInstance->fileassetDictionary) {
				if (pair.second->Get_asset_filepath() == asset_path) {
					return pair.second;
				}
			}

			std::cout << "Asset not found. path: \"" + asset_path + "\"" << std::endl;
			return nullptr;
		}

		static std::vector<IBaseAsset*> GetAssetList() {
			std::vector<IBaseAsset*> list;

			for (const auto& pair : activeInstance->fileassetDictionary)
			{
				list.push_back(pair.second);
			}

			return list;
		}
	};

	template<class TAsset, class TAssetImportData, class TAssetLoadData>
	AssetLoader<TAsset, TAssetImportData, TAssetLoadData>* AssetLoader<TAsset, TAssetImportData, TAssetLoadData>::activeInstance = nullptr;
}