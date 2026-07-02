#================# LIBRARY INIT #================#
set(LIBRARY_NAME "app_graphics")

add_library(${LIBRARY_NAME} STATIC
	"loaders/TextureLoader.cpp"
)


#================# LIBRARY SPECIFIC DEPENDENCIES #================#

target_link_libraries(${LIBRARY_NAME}
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

target_link_libraries(${LIBRARY_NAME} PRIVATE glaze::glaze)

target_link_libraries(${LIBRARY_NAME} PRIVATE glew::glew)

target_link_libraries(${LIBRARY_NAME} PRIVATE imgui::imgui)

target_compile_definitions(${LIBRARY_NAME} PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(${LIBRARY_NAME} PUBLIC glm::glm)

target_link_libraries(${LIBRARY_NAME} PUBLIC stb::stb)