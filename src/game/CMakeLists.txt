set(CURRENT_CMAKE_LIB app_game)

#SUBDIRECTORY LOADING
add_library(${CURRENT_CMAKE_LIB} STATIC
	"test.cpp"
)


target_link_libraries(${CURRENT_CMAKE_LIB} PRIVATE imgui)

target_link_libraries(${CURRENT_CMAKE_LIB} PRIVATE nfd)

target_compile_definitions(${CURRENT_CMAKE_LIB} PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(${CURRENT_CMAKE_LIB} PUBLIC glm)

target_link_libraries(${CURRENT_CMAKE_LIB} PRIVATE glaze)

target_include_directories(${CURRENT_CMAKE_LIB} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")