# e8
# Max Hermann, September 2013

cmake_minimum_required(VERSION 2.6)
project(e8)
set(e8_VERSION_MAJOR 0)
set(e8_VERSION_MINOR 1)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

#---- Dependencies ------------------------------------------------------------

#-------------------
#  Boost
#-------------------
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED) #COMPONENTS system filesystem program_options 
add_definitions(-DBOOST_ALL_NO_LIB) # prevent boost automatic linkage on Windows
include_directories(${Boost_INCLUDE_DIR})
message(STATUS "Boost_INCLUDE_DIR=${Boost_INCLUDE_DIR}")
message(STATUS "Boost_LIBRARIES=${Boost_LIBRARIES}")

#---- Imports ----------------------------------------------------------------

# For general information on CMake export / import libraries see also:
#   http://www.cmake.org/Wiki/CMake/Tutorials/Exporting_and_Importing_Targets

#-------------------
#  param  
#-------------------
# This our own property/parameter management library, required by all sub 
# projects. There is also a qparam Qt GUI support, but this is only required
# by the editor so far and therefore imported there.
#
wmuteImportLibrary( param )
	# was: include("${WINTERMUTE_INSTALL_LIB_DIR}/param-targets.cmake")
include_directories("../propertytree/include")

#---- Libraries & exectuables (in sub projects) ------------------------------

include_directories(
	"include"      # external style, e.g. #include <e8/base/Module.h>
	"include/e8"   # internal style, e.g. #include "base/Module.h"
	)

add_subdirectory( base )

# GLFW
add_subdirectory( viewer )

# Qt
#add_subdirectory( editor )
