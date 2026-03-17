#pragma once
#include "asset/assetloading/AssetLoader.h"
#include "Math/gbe_math.h"
#include <functional>
#include <unordered_map>
#include <stack>

#include "asset/types/Texture.h"

namespace app::graphics {
	struct TextureData {
		uint32_t texturehandle;

		uint32_t bitsPerPixel;
		std::vector<uint8_t> data;
		gbe::Vector2Int dimensions;
		int colorchannels;
	};

	// typedef std::function<VkDescriptorSet(gbe::vulkan::Sampler*, gbe::vulkan::ImageView*)> GbeUiCallbackFunction; // REMOVED

	class TextureLoader : public app::AssetLoader<app::Texture, app::data::TextureImportData, TextureData> {
	private:
		TextureData defaultImage;
		// static GbeUiCallbackFunction Ui_Callback; // REMOVED
	protected:
		void LoadAsset_(app::Texture* asset, const app::data::TextureImportData& importdata, TextureData* data) override;
		void UnLoadAsset_(TextureData* data) override;
	public:
		void AssignSelfAsLoader() override;
		static TextureData& GetDefaultImage();
		static void ReSave(app::Texture* asset);

		inline virtual void OnAsyncTaskCompleted(AsyncLoadTask* loadtask) override {
			//This is a synchronous loader
		}
	};
}