#include "Texture.h"
#include "Asset/AssetLoading/AssetLoader.h"

app::Texture::Texture(std::filesystem::path asset_path) : app::BaseAsset<Texture, TextureImportData>(asset_path){
	this->assettype = AssetType::TEXTURE;
}