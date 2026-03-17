#================# LIBRARY INIT #================#
set(LIBRARY_NAME "app_graphics")

add_library(${LIBRARY_NAME} STATIC
	"loaders/TextureLoader.cpp"
)


#================# LIBRARY SPECIFIC DEPENDENCIES #================#

#WINDOW CORE LIBRARY
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${LIBRARY_NAME}
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
#RENDERER LIBRARY
find_package(GLEW CONFIG REQUIRED)
target_link_libraries(${LIBRARY_NAME} PRIVATE GLEW::GLEW)
#GUI LIBRARY
find_package(imgui CONFIG REQUIRED)
target_link_libraries(${LIBRARY_NAME} PRIVATE imgui::imgui)
#GLM
target_compile_definitions(${LIBRARY_NAME} PUBLIC GLM_ENABLE_EXPERIMENTAL)
find_package(glm CONFIG REQUIRED)
target_link_libraries(${LIBRARY_NAME} PUBLIC glm::glm)
