#================# LIBRARY INIT #================#
set(LIBRARY_NAME "gab_picsel")

add_library(${LIBRARY_NAME} STATIC
	"gui/ProjectWindow.cpp"
	"gui/ImageBrowser.cpp"
 "project/ImageManager.cpp"
 "project/ProjectManager.cpp"
 )

#================# LIBRARY SPECIFIC DEPENDENCIES #================#

find_package(imgui CONFIG REQUIRED)
target_link_libraries(${LIBRARY_NAME} PRIVATE imgui::imgui)

find_package(nfd CONFIG REQUIRED)
target_link_libraries(${LIBRARY_NAME} PRIVATE nfd::nfd)

target_compile_definitions(${LIBRARY_NAME} PUBLIC GLM_ENABLE_EXPERIMENTAL)
find_package(glm CONFIG REQUIRED)
target_link_libraries(${LIBRARY_NAME} PUBLIC glm::glm)