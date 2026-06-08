#pragma once

#include "asset/BaseAsset.h"
#include "math/gbe_math.h"

namespace app {
	struct TextureImportData
	{
		std::string path;
		std::string type;
	};

	class Texture : public BaseAsset<Texture, TextureImportData> {
	public:
		Texture(std::filesystem::path asset_path);
	};
}