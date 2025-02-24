# livespecgl (Wintermute version)
cmake_minimum_required(VERSION 2.8)
project(livespecgl)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

# libs
find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)
find_package(GLEW   REQUIRED)

set(BASS_INCLUDE_PATH "${WINTERMUTE_3RDPARTY_LIBS_PATH}/bass"  CACHE PATH     "BASS include path")
set(BASS_LIBRARY      "${BASS_INCLUDE_PATH}/bass.lib"          CACHE FILEPATH "BASS library")
message(STATUS "BASS_LIBRARY=${BASS_LIBRARY}")

set(GLM_INCLUDE_DIR  "${WINTERMUTE_3RDPARTY_LIBS_PATH}/glm"    CACHE PATH "glm include path")
set(GLUTILS_PATH     "${WINTERMUTE_GLUTILS_PATH}"              CACHE PATH "path to authors custom glutils")

set(GL2PS_SOURCE_DIR "${WINTERMUTE_3RDPARTY_LIBS_PATH}/gl2ps"  CACHE PATH "gl2ps source path")
set(GL2PS_SOURCES ${GL2PS_SOURCE_DIR}/gl2ps.c ${GL2PS_SOURCE_DIR}/gl2ps.h)

include_directories(
	${GLEW_INCLUDE_DIR}
	${GLUT_INCLUDE_DIR}
	${BASS_INCLUDE_PATH}
	${GLM_INCLUDE_DIR}
	${GLUTILS_PATH}
	${GL2PS_SOURCE_DIR}
)

source_group("gl2ps"   FILES ${GL2PS_SOURCES})
source_group("glutils" FILES ${WINTERMUTE_GLUTILS_SOURCES})

add_executable(livespecgl
	livespecgl2.cpp
	SoundInput.cpp
	SoundInput.h
	MonroPressingPhaseSpace.cpp
	MonroPressingPhaseSpace.h
	IlluminatedLinesRenderer.cpp
	IlluminatedLinesRenderer.h
	${WINTERMUTE_GLUTILS_SOURCES}
	${GL2PS_SOURCES}
)

add_executable(beatdetect
	beatdetect.cpp
)

target_link_libraries(beatdetect
	${OPENGL_LIBRARY}
	${GLUT_LIBRARIES}
	${BASS_LIBRARY}
)

target_link_libraries(livespecgl
	${GLEW_LIBRARY}
	${OPENGL_LIBRARY}
	${GLUT_LIBRARIES}
	${BASS_LIBRARY}
)