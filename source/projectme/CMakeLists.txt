# projectme - Simple projection mapping engine.
# Max Hermann, Jul 2014
cmake_minimum_required (VERSION 2.8.11)
project(projectme)

# Common CMake macros (e.g. wintermuteExportLibrary)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

#---- User config  ------------------------------------------------------------

# User variables
set( PROJECTME_USE_BASS "TRUE" CACHE BOOL 
		"Enable audio support via BASS library." )
        
set( PROJECTME_USE_VLC "FALSE" CACHE BOOL 
		"Enable video playback support via libVLC." )
        
		
if( PROJECTME_USE_BASS )
	message( STATUS "BASS enabled." )
else()
	add_definitions(-DPROJECTME_BASS_DISABLED)
endif()

#---- Dependencies ------------------------------------------------------------

#-------------------
# Wintermute dependencies
#-------------------

# WintermuteCommon.cmake defines
# - WINTERMUTE_3RDPARTY_LIBS_PATH
# - WINTERMUTE_GLUTILS_PATH
# - WINTERMUTE_GLUTILS_SOURCES

# g-trucs GLM - OpenGL math library 
set(GLM_INCLUDE_DIR  "${WINTERMUTE_3RDPARTY_LIBS_PATH}/glm"    CACHE PATH "glm include path")
# My own OpenGL utilities
set(GLUTILS_PATH     "${WINTERMUTE_GLUTILS_PATH}"              CACHE PATH "path to authors custom glutils")
set(GLUTILS_SOURCES ${WINTERMUTE_GLUTILS_SOURCES})

# un4seen BASS library
set(BASS_INCLUDE_PATH "${WINTERMUTE_3RDPARTY_LIBS_PATH}/bass"  CACHE PATH     "BASS include path")
if (WIN32)
	set(BASS_LIBRARY      "${BASS_INCLUDE_PATH}/win64/bass.lib"     CACHE FILEPATH "BASS library")
else() # Assume UNIX
	set(BASS_LIBRARY      "${BASS_INCLUDE_PATH}/linux64/libbass.so" CACHE FILEPATH "BASS library")
endif()	
message(STATUS "BASS_LIBRARY=${BASS_LIBRARY}")

include_directories( 
	${GLUTILS_PATH} 
	${GLM_INCLUDE_DIR}
	${BASS_INCLUDE_PATH} 
)

#-------------------
# libVLC (optional, required for video playback)
#-------------------
if( PROJECTME_USE_VLC )
    find_package(LIBVLC)
    if( LIBVLC_FOUND )
        include_directories( ${LIBVLC_INCLUDE_DIR} )
        add_definitions( -DUSE_LIBVLC )
        message(STATUS "libVLC enabled")
        set( PROJECTME_USE_LIBVLC "TRUE" )
    else()
        message(STATUS "libVLC could not be found!")
        set( PROJECTME_USE_LIBVLC "FALSE" )
    endif()
endif()
message(STATUS "LIBVLC_LIBRARY=${LIBVLC_LIBRARY}")

#---------------------------------------------------------
# Boost (e.g. filesystem dependency in included libraries)
# // unix needs filesystem AND system-package by boost 
#----------------------------------------------------------
set(Boost_USE_STATIC_LIBS      ON)
set(Boost_USE_MULTITHREADED    ON)
set(Boost_USE_STATIC_RUNTIME  OFF)
set(Boost_NO_SYSTEM_PATHS      ON)
set(Boost_DEBUG OFF)
find_package(Boost COMPONENTS system REQUIRED) # header only so far
#find_package(Boost COMPONENTS system filesystem program_options REQUIRED)
# prevent boost automatic linkage on Windows
add_definitions(-DBOOST_ALL_NO_LIB)
message(STATUS "Boost_INCLUDE_DIR=${Boost_INCLUDE_DIR}")
message(STATUS "Boost_LIBRARIES=${Boost_LIBRARIES}")
message(STATUS "Boost_LIBRARY_DIRS=${Boost_LIBRARY_DIRS}")
message(STATUS "BOOST_LIBRARYDIR=${BOOST_LIBRARYDIR}")
include_directories(${Boost_INCLUDE_DIR})

#~ #-------------------
#~ # Qt4
#~ #-------------------
#~ find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL QtXml) # REQUIRED
#~ include(${QT_USE_FILE})
#~ if( QT_FOUND )
#~ 	message(STATUS "Qt4 found")
#~ endif()

#-------------------
# Qt5
#-------------------
find_package(Qt5Widgets)
find_package(Qt5OpenGL)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)


#-------------------
# OpenGL, GLEW
#-------------------
find_package(OpenGL REQUIRED)
find_package(GLEW   REQUIRED)
include_directories( ${GLEW_INCLUDE_DIR} )
message(STATUS "GLEW_INCLUDE_DIR = ${GLEW_INCLUDE_DIR}")

#---------------------
# OpenMP (optional)
#---------------------
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")	
	add_definitions( -DUSE_OPENMP )
	message(STATUS "OpenMP enabled")
endif()

#---------------------
# OpenMesh (manually optional)
#---------------------
# Note: Alternatively one could also use FindOpenMesh.cmake
set(OPENMESH_INCLUDE_PATH $ENV{OPENMESH_INCLUDE_PATH} CACHE PATH 
	"Path to OpenMesh base directory.")
set(OPENMESH_LIB_PATH     $ENV{OPENMESH_LIB_PATH}     CACHE PATH 
	"Path to OpenMesh base directory.")

set( OPENMESH_FOUND 0 )

if( OPENMESH_FOUND )
	set(OPENMESH_LIBRARIES
		optimized OpenMeshCore  debug OpenMeshCored
		optimized OpenMeshTools debug OpenMeshToolsd
	)	
	include_directories(
		${OPENMESH_INCLUDE_PATH}
	)
	link_directories(
		${OPENMESH_LIB_PATH}
	)
else()
	set(OPENMESH_LIBRARIES "")
endif()

#---------------------
# Eigen
#---------------------
set(EIGEN_INCLUDE_PATH $ENV{EIGEN_INCLUDE_PATH} CACHE PATH
	"Path to Eigen3 base directory.")
	
include_directories(
	${EIGEN_INCLUDE_PATH}
)

#-------------------
# teem (optional)
#-------------------
find_package(TEEM)
if( TEEM_FOUND )
	include_directories( ${TEEM_INCLUDES} )
	add_definitions( -DUSE_TEEM )
	message(STATUS "Teem enabled")
else()
	set(TEEM_LIBRARIES "")
endif()



#---- Sub-projects ------------------------------------------------------------

include_directories( "include" )

add_subdirectory( "editor" )

