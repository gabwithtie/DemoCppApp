#include "BatchLoader.h"

void app::BatchLoader::ReloadDirectory(std::filesystem::path directory)
{
    GenerateMetafiles(directory);
    LoadAssetsFromDirectory(directory);
}