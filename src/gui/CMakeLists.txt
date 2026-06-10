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
 "features/network/NetworkWindow.h" "features/network/NetworkWindow.cpp" "features/console/Console.cpp" "features/network/ChatWindow.cpp"  "features/menuBar/MenuBarExtension.h")

find_package(imgui CONFIG REQUIRED)
target_link_libraries(${CURRENT_CMAKE_LIB} PRIVATE imgui::imgui)

find_package(nfd CONFIG REQUIRED)
target_link_libraries(${CURRENT_CMAKE_LIB} PRIVATE nfd::nfd)

target_compile_definitions(${CURRENT_CMAKE_LIB} PUBLIC GLM_ENABLE_EXPERIMENTAL)
find_package(glm CONFIG REQUIRED)
target_link_libraries(${CURRENT_CMAKE_LIB} PUBLIC glm::glm)