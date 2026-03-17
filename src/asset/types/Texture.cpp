#include "Texture.h"
#include "Asset/AssetLoading/AssetLoader.h"

app::Texture::Texture(std::filesystem::path asset_path) : app::BaseAsset<Texture, data::TextureImportData>(asset_path){
	this->assettype = AssetType::TEXTURE;
}