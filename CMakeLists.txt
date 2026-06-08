#=========================# PROJECT INIT #=========================#
cmake_minimum_required (VERSION 3.10)

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

add_subdirectory("src/network")
target_link_libraries(app_internal_libs INTERFACE app_network)

add_subdirectory("src/math")
target_link_libraries(app_internal_libs INTERFACE app_math)

add_subdirectory("src/asset")
target_link_libraries(app_internal_libs INTERFACE app_asset)

add_subdirectory("src/graphics")
target_link_libraries(app_internal_libs INTERFACE app_graphics)

# For each folder/library above, make it so that they can #include anything from the root /src folder
get_target_property(LINKED_LIBS app_internal_libs INTERFACE_LINK_LIBRARIES)
foreach(lib ${LINKED_LIBS})
    string(APPEND LINKED_LIBS_STRING " | ${lib} ")
    target_include_directories(${lib} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_include_directories(${lib} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ext")
endforeach()

# Finally, link all libraries to our main
target_link_libraries(${PROJECT_NAME} PRIVATE app_internal_libs)

#=========================# EXTERNAL LIBRARIES #=========================#
#WINDOW CORE LIBRARY
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
#RENDERER LIBRARY
find_package(GLEW CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW)
#GUI LIBRARY
find_package(imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)
#STEAMWORKS
add_subdirectory("ext/gab-steam")
target_link_libraries(${PROJECT_NAME} PUBLIC steam_api_imported)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${STEAM_LIB_PATH}/steam_api64.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
#IMAGE IMPORTER
find_package(Stb REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Stb_INCLUDE_DIR})

#=========================# IMPLEMENTATION LIBRARIES #=========================#

add_library(app_implementation_libs INTERFACE)

add_subdirectory("ext/gab-tablesim")
target_link_libraries(app_implementation_libs INTERFACE gab_tablesim)

add_subdirectory("ext/picsel")
target_link_libraries(app_implementation_libs INTERFACE gab_picsel)


# For each folder/library above, make it so that they can #include anything from the root /src folder
get_target_property(LINKED_LIBS app_implementation_libs INTERFACE_LINK_LIBRARIES)
foreach(lib ${LINKED_LIBS})
    string(APPEND LINKED_LIBS_STRING " | ${lib} ")
    target_include_directories(${lib} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_include_directories(${lib} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ext")
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