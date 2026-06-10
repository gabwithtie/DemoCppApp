#pragma once
#include "asset/assetloading/AssetLoader.h"
#include "Math/gbe_math.h"
#include <functional>
#include <unordered_map>
#include <stack>
#include <string>

#include "asset/types/Texture.h"

namespace app {
	struct TextureData {
		uint32_t textureHandle;

		uint32_t bitsPerPixel;
		std::vector<uint8_t> data;
		gbe::Vector2Int dimensions;
		int colorChannels;
	};

	class TextureLoader : public app::AssetLoader<Texture, TextureImportData, TextureData> {
	private:
		TextureData defaultImage;
	protected:
		void LoadAsset_(app::Texture* asset, const app::TextureImportData& importdata, TextureData* data) override;
		void UnLoadAsset_(TextureData* data) override;
	public:
		using AssetLoader::AssetLoader;

		void AssignSelfAsLoader() override;
		static TextureData& GetDefaultImage();
		static void ReSave(app::Texture* asset);

		// --- NEW: Syncs CPU-edited std::vector<uint8_t> back to the OpenGL texture
		static void UpdateGPU(const std::string& assetId);

		inline virtual void OnAsyncTaskCompleted(AsyncLoadTask* loadtask) override {
			//This is a synchronous loader
		}
	};
}