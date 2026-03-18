#================# LIBRARY INIT #================#
set(LIBRARY_NAME "gab_tablesim")

add_library(${LIBRARY_NAME} STATIC
	"TableSim.cpp"
 "network/Table.cpp"
 "gui/TableWindow.cpp"
 "gui/HandWindow.cpp"
 "gui/TableSimMenuExtension.h" "network/Decks.cpp" "network/Hands.cpp" "gui/DeckWindow.cpp")

#================# LIBRARY SPECIFIC DEPENDENCIES #================#
target_compile_definitions(${LIBRARY_NAME} PUBLIC GLM_ENABLE_EXPERIMENTAL)
find_package(glm CONFIG REQUIRED)
target_link_libraries(${LIBRARY_NAME} PUBLIC glm::glm)