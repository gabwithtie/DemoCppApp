#SETUP
set(CUR_TARGET_SUBLIBNAME gui)
set(CUR_TARGET_NAME gbe_app_${CUR_TARGET_SUBLIBNAME})

set(${CUR_TARGET_NAME}_src
	"GuiManager.cpp"

	"main/GuiElement.cpp"
	"main/GuiWindow.cpp"
	"main/MenuBar.cpp"

	"features/directory/DirectoryBrowser.cpp"
	"features/menuBar/MenuBarExtension.h"
)

add_library(${CUR_TARGET_NAME} STATIC ${${CUR_TARGET_NAME}_src})
add_library(gbe::app::${CUR_TARGET_SUBLIBNAME} ALIAS ${CUR_TARGET_NAME})
target_include_directories(${CUR_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${CUR_TARGET_NAME} PUBLIC imgui)
target_include_directories(${CUR_TARGET_NAME} PUBLIC ${imgui_SOURCE_DIR})

target_link_libraries(${CUR_TARGET_NAME} PRIVATE nfd)
target_compile_definitions(${CUR_TARGET_NAME} PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(${CUR_TARGET_NAME} PUBLIC glm)

target_include_directories(${CUR_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})