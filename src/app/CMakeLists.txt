#SETUP
set(CUR_TARGET_SUBLIBNAME app)
set(CUR_TARGET_NAME gbe_app_${CUR_TARGET_SUBLIBNAME})

set(${CUR_TARGET_NAME}_src
	App.cpp
	ProjectExporter.cpp
	gui/ProjectPicker.cpp
	gui/ActiveToolSettingsWindow.cpp
	gui/IllustratorWindow.cpp
	gui/LayerManagerWindow.cpp
	gui/ToolRegistryWindow.cpp
	gui/HotkeyWindow.cpp
	gui/tools/PencilTool.cpp
	gui/tools/PathDrawTool.cpp
	gui/SampleWindow.cpp
)

add_library(${CUR_TARGET_NAME} STATIC ${${CUR_TARGET_NAME}_src})
add_library(gbe::app::${CUR_TARGET_SUBLIBNAME} ALIAS ${CUR_TARGET_NAME})
target_include_directories(${CUR_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${CUR_TARGET_NAME} PRIVATE imgui)
target_include_directories(${CUR_TARGET_NAME} PRIVATE ${imgui_SOURCE_DIR})
target_include_directories(${CUR_TARGET_NAME} PRIVATE ${stb_SOURCE_DIR})

target_link_libraries(${CUR_TARGET_NAME} PUBLIC gbe::app::math)
target_link_libraries(${CUR_TARGET_NAME} PUBLIC gbe::app::gui)
target_link_libraries(${CUR_TARGET_NAME} PUBLIC gbe::tools)
target_include_directories(${CUR_TARGET_NAME} PUBLIC ${miniaudio_SOURCE_DIR})