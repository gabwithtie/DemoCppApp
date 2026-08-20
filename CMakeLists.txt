#=========================# PROJECT INIT #=========================#
cmake_minimum_required (VERSION 3.10)

# FETCH CONTENT SETUP
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/.deps")
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE) 

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()
# Set CPP version
cmake_minimum_required (VERSION 3.22)
set(CMAKE_CXX_STANDARD 23)

# Set name to avoid copy pasting it everywhere
set(PROJECT_NAME "DemoCppApp")
project (${PROJECT_NAME})

#==============MAIN DEPENDENCIES================#

# Force all sub-builds to default to Static
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ONLY_LIBS ON CACHE INTERNAL "")

#SDL
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL2MIXER_SHARED OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_SHARED OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_VENDORED ON CACHE BOOL "" FORCE)
set(SDL2MIXER_OPUS OFF CACHE BOOL "" FORCE)

add_compile_definitions(__PRFCHWINTRIN_H)

set(SDL2IMAGE_VENDORED ON CACHE BOOL "" FORCE)
FetchContent_Declare(SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.32.2
)

set(GLEW_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(BUILD_GLEW_SHARED OFF CACHE BOOL "" FORCE)
set(BUILD_GLEW_STATIC ON CACHE BOOL "" FORCE)

FetchContent_Declare(glew
    GIT_REPOSITORY https://github.com/perlmint/glew-cmake
    GIT_TAG glew-cmake-2.2.0-1
)

FetchContent_Declare(glaze
    GIT_REPOSITORY https://github.com/stephenberry/glaze.git
    GIT_TAG v2.6.9
)
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.8-docking
    SOURCE_DIR "${FETCHCONTENT_BASE_DIR}/imgui"
)
FetchContent_Declare(nfd
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
    GIT_TAG v1.2.1
)
FetchContent_Declare(stb GIT_REPOSITORY
    https://github.com/nothings/stb.git
    GIT_TAG f75e8d1cad7d90d72ef7a4661f1b994ef78b4e31
)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)

#==============COMMIT DEPENDENCIES================#
FetchContent_MakeAvailable(
    SDL2
    glm
    glaze
    imgui
    nfd
    stb
    glew
)

#==============ALIASING AND POPULATION FIXES================#
if(stb_POPULATED AND NOT TARGET stb::stb)
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
    add_library(stb::stb ALIAS stb)
endif()
if(imgui_POPULATED AND NOT TARGET imgui::imgui)
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        # Include the GFX backend requested in vcpkg.json
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR} #for other libraries that depend on imgui
    )
    target_link_libraries(imgui PUBLIC SDL2::SDL2-static)
    add_library(imgui::imgui ALIAS imgui)
endif()
if(nfd_POPULATED AND NOT TARGET nfd::nfd)
    add_library(nfd::nfd ALIAS nfd)
endif()
if(glew_POPULATED AND NOT TARGET glew::glew)
    add_library(glew::glew ALIAS libglew_static)
endif()
if(TARGET SDL2-static AND NOT TARGET SDL2)
    add_library(SDL2 ALIAS SDL2-static)
endif()


#=========================# INIT CODE #=========================#

# Add primary source to this project's executable.
add_executable (${PROJECT_NAME} "src/main.cpp")

if(MSVC)
    # Only apply the "Hide Console" flags to the Release build
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
        LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )
endif()

#=========================# INTERNAL LIBRARIES #=========================#

add_library(app_internal_libs INTERFACE)

add_subdirectory("src/gui")
target_link_libraries(app_internal_libs INTERFACE app_gui)

add_subdirectory("src/window")
target_link_libraries(app_internal_libs INTERFACE app_window)

add_subdirectory("src/math")
target_link_libraries(app_internal_libs INTERFACE app_math)

# For each folder/library above, make it so that they can #include anything from the root /src folder
get_target_property(LINKED_LIBS app_internal_libs INTERFACE_LINK_LIBRARIES)
foreach(lib ${LINKED_LIBS})
    string(APPEND LINKED_LIBS_STRING " | ${lib} ")
    target_include_directories(${lib} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_include_directories(${lib} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ext")
endforeach()

# Finally, link all libraries to our main
add_subdirectory("tools")

target_link_libraries(${PROJECT_NAME} PRIVATE app_internal_libs gbe::tools)

#=========================# EXTERNAL LIBRARIES #=========================#
#WINDOW CORE LIBRARY
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
#RENDERER LIBRARY
target_link_libraries(${PROJECT_NAME} PRIVATE glew::glew)
#GUI LIBRARY
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)
#JSON LIBRARY
target_link_libraries(${PROJECT_NAME} PRIVATE glaze::glaze)

#=========================# IMPLEMENTATION LIBRARIES #=========================#

add_library(app_implementation_libs INTERFACE)


# For each folder/library above, make it so that they can #include anything from the root /src folder
get_target_property(LINKED_LIBS app_implementation_libs INTERFACE_LINK_LIBRARIES)
foreach(lib ${LINKED_LIBS})
    if(TARGET ${lib})
        string(APPEND LINKED_LIBS_STRING " | ${lib} ")
        target_include_directories(${lib} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
        target_include_directories(${lib} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ext")
    endif()
endforeach()

# Finally, link all libraries to our main
target_link_libraries(${PROJECT_NAME} PRIVATE app_implementation_libs)

#=========================# FINAL COPY COMMANDS #=========================#
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD     
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/default"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "Copying data files to build directory"
)