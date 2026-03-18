#================# LIBRARY INIT #================#
set(LIBRARY_NAME "app_network")

add_library(${LIBRARY_NAME} STATIC
	"Network.cpp"
	"NetworkObject.h"
	"NetworkRegistry.h"
 "NetworkObject.cpp" "NetworkRegistry.cpp" "NetworkDictionary.h")

#================# LIBRARY SPECIFIC DEPENDENCIES #================#

