# kineyes - 3D model follows user movement
# requires: GLUT, OpenNI
# November 2012
cmake_minimum_required(VERSION 2.8)
project(kineyes)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

# libs
find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)
find_package(OpenNI REQUIRED)

add_executable( kineyes
	kineyes.cpp
	SimpleEyes.h
	SimpleEyes.cpp
	# NiHandTracker sample code
	NiHandTracker.h
	NiHandTracker.cpp
	NiTrailHistory.h
	NiTrailHistory.cpp
)

include_directories( kineyes
	${GLUT_INCLUDE_DIR}	
	${OpenNI_INCLUDE_DIRS}
)

target_link_libraries( kineyes
	${GLUT_LIBRARY}	
	${OPENGL_LIBRARY}
	${OpenNI_LIBRARIES}
)