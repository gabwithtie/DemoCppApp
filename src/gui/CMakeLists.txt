set(CURRENT_CMAKE_LIB app_gui)

#SUBDIRECTORY LOADING
add_library(${CURRENT_CMAKE_LIB} STATIC
	"GuiManager.cpp"

	"main/GuiElement.cpp"
	"main/GuiWindow.cpp"
	"main/MenuBar.cpp"

	"features/directory/DirectoryBrowser.cpp"
)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(${CURRENT_CMAKE_LIB} PRIVATE imgui::imgui)

find_package(nfd CONFIG REQUIRED)
target_link_libraries(${CURRENT_CMAKE_LIB} PRIVATE nfd::nfd)
