set(CURRENT_CMAKE_LIB app_asset)

#SUBDIRECTORY LOADING
add_library(${CURRENT_CMAKE_LIB} STATIC
 
 "types/Texture.cpp"
 
  "AssetLoading/AssetLoader.cpp"
  "BaseAsset.cpp" 
  "AssetLoading/BatchLoader.cpp" "types/Types.h"
)

target_compile_definitions(${CURRENT_CMAKE_LIB} PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(${CURRENT_CMAKE_LIB} PUBLIC glm::glm)

target_link_libraries(${CURRENT_CMAKE_LIB} PRIVATE glaze::glaze)

message("[GABENGINE] LOADED: " ${CMAKE_CURRENT_SOURCE_DIR})
