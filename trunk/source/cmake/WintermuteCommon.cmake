# Common CMake configuration for Wintermute projects
# Dec 2012

message(STATUS "---- Wintermute global config ----")

#---- CMake config ------------------------------------------------------------

# CMAKE_CURRENT_LIST_DIR only available from CMake version greater 2.8.3
# Workaround from:
# http://code.google.com/p/gqp/source/browse/cmake/Functions.cmake?name=GQP_v0.13.2
if(NOT DEFINED CMAKE_CURRENT_LIST_DIR)
  get_filename_component(CMAKE_CURRENT_LIST_DIR
    ${CMAKE_CURRENT_LIST_FILE} PATH)
endif(NOT DEFINED CMAKE_CURRENT_LIST_DIR)

# CMake module config
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

#---- Output paths ------------------------------------------------------------

set(WINTERMUTE_OUTPUT_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../../"  
	CACHE PATH "Wintermute base path for bin and lib outputs.")

# Set default output paths
set(EXECUTABLE_OUTPUT_PATH    ${WINTERMUTE_OUTPUT_PATH}/bin)
set(LIBRARY_OUTPUT_PATH       ${WINTERMUTE_OUTPUT_PATH}/lib)
set(CMAKE_RUNTIME_OUTPUT_PATH ${WINTERMUTE_OUTPUT_PATH}/bin)

#---- Global project variables ------------------------------------------------

set(WINTERMUTE_3RDPARTY_LIBS_PATH "${CMAKE_CURRENT_LIST_DIR}/../3rdparty")
set(WINTERMUTE_GLUTILS_PATH       "${CMAKE_CURRENT_LIST_DIR}/..") # /glutils

file(GLOB_RECURSE WINTERMUTE_GLUTILS_SOURCES ${WINTERMUTE_GLUTILS_PATH}/glutils/*.cpp)
source_group("glutils" FILES ${WINTERMUTE_GLUTILS_SOURCES})

#---- Export libraries --------------------------------------------------------

# Custom export library macro
macro( wmuteExportLibrary target )
	# Build-tree export
	export(TARGETS ${target} FILE "${WINTERMUTE_INSTALL_LIB_DIR}/${target}-exports.cmake")
	
	message(STATUS "Wintermute: Exporting library ${target} to ${WINTERMUTE_INSTALL_LIB_DIR}/${target}-exports.cmake")
	
	# Install export
	#install(TARGETS e8base DESTINATION ${WINTERMUTE_INSTALL_LIB_DIR} EXPORT e8base-targets)
	#install(EXPORT e8base-targets DESTINATION ${WINTERMUTE_INSTALL_LIB_DIR})
endmacro()

# Custom import library macro
macro( wmuteImportLibrary target )
	include("${WINTERMUTE_INSTALL_LIB_DIR}/${target}-exports.cmake")
	
	message(STATUS "Wintermute: Importing library ${target} via ${WINTERMUTE_INSTALL_LIB_DIR}/${target}-exports.cmake")
endmacro()

# Libraries are exported for internal use into the following directory
set(WINTERMUTE_INSTALL_LIB_DIR "${WINTERMUTE_OUTPUT_PATH}/exports"
	CACHE PATH "Installation directory for (exported) libraries")
mark_as_advanced(WINTERMUTE_INSTALL_LIB_DIR)

# Additional install paths (not used)
#set(WINTERMUTE_INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
#set(WINTERMUTE_INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")

# Add "_d" suffix to debug libraries (because they are installed in the same
# directory as the release libraries, which can not be mixed on Windows.
set(CMAKE_DEBUG_POSTFIX "_d")

#---- Platform specific configuration -----------------------------------------

# compiler flags
if(UNIX)
  message("Setting specific UNIX gcc flags.")
  set( CMAKE_CFLAGS_RELEASE     "-O3 -W -Wall -ansi -pedantic -Wno-unused -DNDEBUG" )
  set( CMAKE_CXX_FLAGS_RELEASE  "-O3    -Wall -ansi -pedantic -Wno-unused -DNDEBUG" )
  set( CMAKE_C_FLAGS_DEBUG      "-g  -W -Wall -ansi -pedantic -Wno-unused -DDEBUG"  )
  set( CMAKE_CXX_FLAGS_DEBUG    "-g  -W -Wall -ansi -pedantic -Wno-unused -DDEBUG"  )
endif(UNIX)

#---- Print status information ------------------------------------------------

message(STATUS "WINTERMUTE_OUTPUT_PATH     = ${WINTERMUTE_OUTPUT_PATH}")
message(STATUS "WINTERMUTE_INSTALL_LIB_DIR = ${WINTERMUTE_INSTALL_LIB_DIR}")
message(STATUS "CMAKE_MODULE_PATH          = ${CMAKE_MODULE_PATH}")
message(STATUS "CMAKE_CURRENT_LIST_DIR     = ${CMAKE_CURRENT_LIST_DIR}")

message(STATUS "----------------------------------")
