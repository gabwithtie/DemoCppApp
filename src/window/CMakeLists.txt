#================# LIBRARY INIT #================#
set(LIBRARY_NAME "app_window")

add_library(${LIBRARY_NAME} STATIC
	"Window.cpp"
)


#================# LIBRARY SPECIFIC DEPENDENCIES #================#

#WINDOW CORE LIBRARY
target_link_libraries(${LIBRARY_NAME} PRIVATE SDL2-static)
#RENDERER LIBRARY
target_link_libraries(${LIBRARY_NAME} PRIVATE glew::glew)
#GUI LIBRARY
target_link_libraries(${LIBRARY_NAME} PRIVATE imgui::imgui)
