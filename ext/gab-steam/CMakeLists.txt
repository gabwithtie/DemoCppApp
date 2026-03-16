# Define the path to your Steamworks SDK
set(STEAMWORKS_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# Include Headers

# Link Libraries (Windows example)
if(WIN32)
    set(STEAM_LIB_PATH "${STEAMWORKS_SDK_PATH}/redistributable_bin/win64")
    set(STEAM_LIB_PATH ${STEAM_LIB_PATH} PARENT_SCOPE)
    add_library(steam_api_imported SHARED IMPORTED GLOBAL)
    set_target_properties(steam_api_imported PROPERTIES IMPORTED_IMPLIB "${STEAM_LIB_PATH}/steam_api64.lib")
endif()