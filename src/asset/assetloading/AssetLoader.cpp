#include "AssetLoader.h"
#include "asset/BaseAsset.h"

std::unordered_map<app::AssetType, app::IAssetCollection*> app::all_asset_loaders;

app::IBaseAsset* app::GetBaseData(std::filesystem::path path) {
	for (const auto& lpair : all_asset_loaders)
	{
		const auto& assetloader = lpair.second;
		auto assetdata = assetloader->FindAssetByPath(path);

		if (assetdata == nullptr)
			continue;

		return assetdata;
	}

	return nullptr;
}

app::AssetType app::GetAssetType(std::filesystem::path path) {
	for (const auto& lpair : all_asset_loaders)
	{
		const auto& assetloader = lpair.second;
		auto assetdata = assetloader->FindAssetByPath(path);

		if (assetdata == nullptr)
			continue;

		return assetdata->GetAssetType();
	}

	return AssetType::NONE;
}

std::string app::GetAssetId(std::filesystem::path path) {
	for (const auto& lpair : all_asset_loaders)
	{
		const auto& assetloader = lpair.second;
		auto assetdata = assetloader->FindAssetByPath(path);

		if (assetdata == nullptr)
			continue;

		return assetdata->GetAssetId();
	}

	return "";
}