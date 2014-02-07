# meshtools - Common CMake configuration and macros.
# Max Hermann, Jan 2014

message(STATUS "---- meshtools global config ----")

#---- CMake config ------------------------------------------------------------

# CMAKE_CURRENT_LIST_DIR only available from CMake version greater 2.8.3
# Workaround from:
# http://code.google.com/p/gqp/source/browse/cmake/Functions.cmake?name=GQP_v0.13.2
if(NOT DEFINED CMAKE_CURRENT_LIST_DIR)
  get_filename_component(CMAKE_CURRENT_LIST_DIR
    ${CMAKE_CURRENT_LIST_FILE} PATH)
endif(NOT DEFINED CMAKE_CURRENT_LIST_DIR)

# Custom CMake modules (e.g. for libQGLViewer)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

#---- Output paths ------------------------------------------------------------

set(MESHTOOLS_OUTPUT_PATH "${CMAKE_CURRENT_LIST_DIR}/.."
	CACHE PATH "Wintermute base path for bin and lib outputs.")

# Set default output paths
set(EXECUTABLE_OUTPUT_PATH    ${MESHTOOLS_OUTPUT_PATH}/bin)
set(LIBRARY_OUTPUT_PATH       ${MESHTOOLS_OUTPUT_PATH}/lib)
set(CMAKE_RUNTIME_OUTPUT_PATH ${MESHTOOLS_OUTPUT_PATH}/bin)

#---- Global project variables ------------------------------------------------

# Header only 3rd party libraries (like SparseICP and nanoflann)
set(MESHTOOLS_3RDPARTY_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/../3rdparty)

#---- Common dependencies -----------------------------------------------------

include_directories(
	${MESHTOOLS_3RDPARTY_INCLUDE_DIR}
)

#---- Build configuration -----------------------------------------------------

# Platform specific defines
if (WIN32)
  add_definitions(
      -D_USE_MATH_DEFINES -DNOMINMAX
  )
endif ()

# Linux compiler flags
if(UNIX)
  message("Setting specific UNIX gcc flags.")
  set( CMAKE_CFLAGS_RELEASE     "-O3 -W -Wall -ansi -pedantic -Wno-unused -DNDEBUG" )
  set( CMAKE_CXX_FLAGS_RELEASE  "-O3    -Wall -ansi -pedantic -Wno-unused -DNDEBUG" )
  set( CMAKE_C_FLAGS_DEBUG      "-g  -W -Wall -ansi -pedantic -Wno-unused -DDEBUG"  )
  set( CMAKE_CXX_FLAGS_DEBUG    "-g  -W -Wall -ansi -pedantic -Wno-unused -DDEBUG"  )
endif(UNIX)

# Allow only Debug and Release builds
set (CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
mark_as_advanced (CMAKE_CONFIGURATION_TYPES)

# Set Debug as default build target
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Debug CACHE STRING
      "Choose the type of build, options are: Debug, Release."
      FORCE)
endif ()

# Add "_d" suffix to debug libraries (because they are installed in the same
# directory as the release libraries, which can not be mixed on Windows.
set(CMAKE_DEBUG_POSTFIX "_d")

#---- Export libraries --------------------------------------------------------

# Custom export library macro
macro( meshtoolsExportLibrary target )
	# Build-tree export
	export(TARGETS ${target} FILE "${MESHTOOLS_INSTALL_LIB_DIR}/${target}-exports.cmake")
	
	message(STATUS "meshtools: Exporting library ${target} to ${MESHTOOLS_INSTALL_LIB_DIR}/${target}-exports.cmake")
	
	# Install export (not used)
	#install(TARGETS e8base DESTINATION ${MESHTOOLS_INSTALL_LIB_DIR} EXPORT e8base-targets)
	#install(EXPORT e8base-targets DESTINATION ${MESHTOOLS_INSTALL_LIB_DIR})
endmacro()

# Custom import library macro
macro( meshtoolsImportLibrary target )
	include("${MESHTOOLS_INSTALL_LIB_DIR}/${target}-exports.cmake")
	
	message(STATUS "meshtools: Importing library ${target} via ${MESHTOOLS_INSTALL_LIB_DIR}/${target}-exports.cmake")
endmacro()

# Libraries are exported for internal use into the following directory
set(MESHTOOLS_INSTALL_LIB_DIR "${MESHTOOLS_OUTPUT_PATH}/exports"
	CACHE PATH "Installation directory for (exported) libraries")
mark_as_advanced(MESHTOOLS_INSTALL_LIB_DIR)

# Additional install paths (not used)
#set(MESHTOOLS_INSTALL_BIN_DIR     bin     CACHE PATH "Installation directory for executables")
#set(MESHTOOLS_INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")

#---- Print status information ------------------------------------------------

message(STATUS "MESHTOOLS_OUTPUT_PATH      = ${MESHTOOLS_OUTPUT_PATH}")
message(STATUS "MESHTOOLS_INSTALL_LIB_DIR  = ${MESHTOOLS_INSTALL_LIB_DIR}")
message(STATUS "CMAKE_MODULE_PATH          = ${CMAKE_MODULE_PATH}")
message(STATUS "CMAKE_CURRENT_LIST_DIR     = ${CMAKE_CURRENT_LIST_DIR}")

message(STATUS "CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")

message(STATUS "----------------------------------")
