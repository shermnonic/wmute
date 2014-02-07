# meshspace - Some basic 3D triangle mesh operations (main cmakefile)
# Max Hermann, Jan 2014
cmake_minimum_required (VERSION 2.6)
project(meshspace)

# Common CMake macros (e.g. meshtoolsExportLibrary)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/meshtoolsCommon.cmake)

#---- Dependencies ------------------------------------------------------------

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

#---- Sub-projects ------------------------------------------------------------

include_directories( "include" )

# meshtools library and CLI applications
# (depends on Eigen3, OpenMesh and header-only libs nanoflann and SICP)
add_subdirectory( "meshtools" )

# mesthspace GUI
# (depends on meshtools lib, Qt4, libQGLViewer)
add_subdirectory( "meshspace" )
