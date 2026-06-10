#include "TextureLoader.h"
#include <stdexcept>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Assuming you have stb_image_write.h in your project, this makes ReSave work easily
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <GL/glew.h>

void app::TextureLoader::UpdateGPU(const std::string& assetId)
{
    // Fetch the asset mapping from your internal registry
    auto data = GetAssetData(assetId);

    // Safeguard to prevent crashing on uninitialized or empty textures
    if (!data || data->data.empty()) return;

    // Bind the OpenGL texture and upload the modified sub-image
    glBindTexture(GL_TEXTURE_2D, data->textureHandle);

    // We use glTexSubImage2D because it's much faster for modifying existing textures 
    // than completely re-allocating it with glTexImage2D
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data->dimensions.x, data->dimensions.y, GL_RGBA, GL_UNSIGNED_BYTE, data->data.data());

    glBindTexture(GL_TEXTURE_2D, 0); // Clean up binding
}

void app::TextureLoader::ReSave(app::Texture* asset)
{
    auto data = GetAssetData(asset->GetAssetId());

    if (!data || data->data.empty()) return;

    auto folderpath = asset->Get_assetFilepath().parent_path();
    auto fullpath = folderpath / asset->GetImportData().path;

    // To physically save the changes out to the hard-drive, you can use stbi_write_png!
    stbi_write_png(fullpath.string().c_str(), data->dimensions.x, data->dimensions.y, 4, data->data.data(), data->dimensions.x * 4);
}

void app::TextureLoader::LoadAsset_(app::Texture* target, const app::TextureImportData& importdata, TextureData* loaddata) {
    if (importdata.path.size() == 0) return;

    const auto& pathstr = target->Get_assetFilepath().parent_path() / importdata.path;

    int width = 0;
    int height = 0;
    int channels = 0;

    // Force 4 channels (RGBA) so OpenGL always gets expected layout, avoiding segfaults
    unsigned char* data = stbi_load(pathstr.string().c_str(), &width, &height, &channels, 4);
    if (data == NULL) {
        std::cout << "Failed to load texture: " << pathstr.string() << std::endl;
        return;
    }

    // Create OpenGL texture
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters
    // FIX: Changed from GL_LINEAR to GL_NEAREST. For a Pixel Art software, 
    // Linear filtering makes pixels look blurry. Nearest preserves sharp pixel edges!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Upload pixels to GPU
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    loaddata->textureHandle = image_texture;
    loaddata->dimensions.x = width;
    loaddata->dimensions.y = height;
    loaddata->colorChannels = 4; // Since we forced 4 channels above

    // --- FIX: POPULATE CPU PIXEL ARRAY ---
    // Calculate total layout size in bytes
    size_t total_bytes = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;

    // Copy the raw STB buffer array into our std::vector so tools can manipulate it
    loaddata->data.assign(data, data + total_bytes);
    // -------------------------------------

    stbi_image_free(data);
}

void app::TextureLoader::UnLoadAsset_(TextureData* data) {
    // Standard cleanup to prevent memory leaks when destroying canvases
    if (data && data->textureHandle != 0) {
        glDeleteTextures(1, &data->textureHandle);
        data->textureHandle = 0;
    }
    data->data.clear();
}

void app::TextureLoader::AssignSelfAsLoader()
{
    AssetLoader::AssignSelfAsLoader();
    app::all_asset_loaders.insert_or_assign(app::TEXTURE, this);

    // Default 1x1 White Pixel
    const uint32_t width = 1;
    const uint32_t height = 1;
}

app::TextureData& app::TextureLoader::GetDefaultImage() {
    return static_cast<app::TextureLoader*>(activeInstance)->defaultImage;
}