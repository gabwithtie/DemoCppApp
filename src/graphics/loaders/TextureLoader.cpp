#include "TextureLoader.h"
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <GL/glew.h>

void app::graphics::TextureLoader::ReSave(app::Texture* asset)
{
	auto data = GetAssetRuntimeData(asset->Get_assetId());

    if (data->data.empty()) return;

	auto folderpath = asset->Get_asset_filepath().parent_path();
	auto fullpath = folderpath / asset->Get_import_data().path;
}


void app::graphics::TextureLoader::LoadAsset_(app::Texture* target, const app::data::TextureImportData& importdata, TextureData* loaddata) {
    if (importdata.path.size() == 0) return;

    const auto& pathstr = target->Get_asset_filepath().parent_path() / importdata.path;

    GLuint my_texture = 0;
    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* data = stbi_load(pathstr.string().c_str(), &width, &height, &channels, 4);
    if (data == NULL)
        std::cout << "Failed to load texture" << std::endl;

    // Create OpenGL texture
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters (Important for ImGui)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels to GPU
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    loaddata->texturehandle = my_texture;
    loaddata->dimensions.x = width;
    loaddata->dimensions.y = height;
    loaddata->colorchannels = channels;
    //std::memcpy(&loaddata->data, data, width * height * channels * sizeof(unsigned char));

    stbi_image_free(data);
}

void app::graphics::TextureLoader::UnLoadAsset_(TextureData* data)
{

}

void app::graphics::TextureLoader::AssignSelfAsLoader()
{
    AssetLoader::AssignSelfAsLoader();
    app::all_asset_loaders.insert_or_assign(app::TEXTURE, this);

    // Default 1x1 White Pixel
    const uint32_t width = 1;
    const uint32_t height = 1;
}

app::graphics::TextureData& app::graphics::TextureLoader::GetDefaultImage() {
    return static_cast<app::graphics::TextureLoader*>(active_instance)->defaultImage;
}