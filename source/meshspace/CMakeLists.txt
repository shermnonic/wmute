# meshspace - Some basic 3D triangle mesh operations (main cmakefile)
# Max Hermann, Jan 2014
cmake_minimum_required (VERSION 2.6)
project(meshspace)

# Common CMake macros (e.g. meshtoolsExportLibrary)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/meshtoolsCommon.cmake)

#---- Dependencies ------------------------------------------------------------

#-------------------
# Custom dependencies
#-------------------
set(WINTERMUTE_3RDPARTY_LIBS_PATH "${CMAKE_CURRENT_LIST_DIR}/../../3rdparty")
set(WINTERMUTE_GLUTILS_PATH       "${CMAKE_CURRENT_LIST_DIR}/../..") # /glutils

set(GLM_INCLUDE_DIR  "${WINTERMUTE_3RDPARTY_LIBS_PATH}/glm"    CACHE PATH "glm include path")
set(GLUTILS_PATH     "${WINTERMUTE_GLUTILS_PATH}"              CACHE PATH "path to authors custom glutils")

file(GLOB_RECURSE GLUTILS_SOURCES ${GLUTILS_PATH}/glutils/*.cpp)

include_directories( ${GLUTILS_PATH} )
include_directories( ${GLM_INCLUDE_DIR} )

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
# OpenMesh
#---------------------
# Note: Alternatively one could also use FindOpenMesh.cmake
set(OPENMESH_INCLUDE_PATH $ENV{OPENMESH_INCLUDE_PATH} CACHE PATH 
	"Path to OpenMesh base directory.")
set(OPENMESH_LIB_PATH     $ENV{OPENMESH_LIB_PATH}     CACHE PATH 
	"Path to OpenMesh base directory.")
	
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

# meshtools library and CLI applications
# (depends on Eigen3, OpenMesh and header-only libs nanoflann and SICP)
add_subdirectory( "meshtools" )

# mesthspace GUI
# (depends on meshtools lib, Qt4, libQGLViewer)
add_subdirectory( "meshspace" )
