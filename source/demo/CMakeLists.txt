# demo
# September 2013
cmake_minimum_required(VERSION 2.8)
project(demo)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

# wmute dependencies
wmuteImportLibrary(fmodaudio)
wmuteImportLibrary(param)
wmuteImportLibrary(e8base)
wmuteImportLibrary(e8viewer)

set( E8_LIBRARIES
	fmodaudio
	param
	e8base
	e8viewer
)

set( E8_INCLUDE_DIRS
	../e8/include
	../propertytree/param
	../fmodplay				# FMODAudio
)

# dependencies
find_package(OpenGL REQUIRED)
find_package(GLFW   REQUIRED)
find_package(FMOD   REQUIRED)

message(STATUS "GLFW_LIBRARIES=${GLFW_LIBRARIES}")

add_library(demo_module
	DemoRenderer.h
	DemoRenderer.cpp
)

add_executable(demo
	demo_viewer.cpp
)

include_directories(demo
	${GLFW_INCLUDE_DIRS}
	${OPENGL_INCLUDE_DIR}	
	${FMOD_INCLUDE_DIR}
	${E8_INCLUDE_DIRS}
)

target_link_libraries(demo
	${GLFW_LIBRARIES}
	${OPENGL_LIBRARY}
	${FMOD_LIBRARY}
	${E8_LIBRARIES}
	demo_module
)
