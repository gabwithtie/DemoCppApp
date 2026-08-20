set(CURRENT_CMAKE_LIB app_gui)

#SUBDIRECTORY LOADING
add_library(${CURRENT_CMAKE_LIB} STATIC
	"GuiManager.cpp"

	"main/GuiElement.cpp"
	"main/GuiWindow.cpp"
	"main/MenuBar.cpp"

	"features/input/InputWindow.cpp"
	"features/input/InputMap.cpp"
	"features/directory/DirectoryBrowser.cpp"
	"features/menuBar/MenuBarExtension.h")


target_link_libraries(${CURRENT_CMAKE_LIB} PRIVATE imgui::imgui)

target_link_libraries(${CURRENT_CMAKE_LIB} PRIVATE nfd::nfd)

target_compile_definitions(${CURRENT_CMAKE_LIB} PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(${CURRENT_CMAKE_LIB} PUBLIC glm::glm)

target_link_libraries(${CURRENT_CMAKE_LIB} PRIVATE glaze::glaze)