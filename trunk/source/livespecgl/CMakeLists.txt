# livespecgl (Wintermute version)
cmake_minimum_required(VERSION 2.8)
project(livespecgl)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

# libs
find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)
#find_package(GLEW   REQUIRED)

set(BASS_INCLUDE_PATH "${WINTERMUTE_3RDPARTY_LIBS_PATH}/bass"  CACHE PATH     "BASS include path")
set(BASS_LIBRARY      "${BASS_INCLUDE_PATH}/bass.lib"          CACHE FILEPATH "BASS library")
message(STATUS "BASS_LIBRARY=${BASS_LIBRARY}")

set(GLM_INCLUDE_DIR  "${WINTERMUTE_3RDPARTY_LIBS_PATH}/glm"    CACHE PATH "glm include path")
set(GLUTILS_PATH     "${WINTERMUTE_GLUTILS_PATH}"              CACHE PATH "path to authors custom glutils")

include_directories(
	${GLUT_INCLUDE_DIR}
	${BASS_INCLUDE_PATH}
	${GLM_INCLUDE_DIR}
	${GLUTILS_PATH}
)

add_executable(livespecgl
	livespecgl2.cpp
	SoundInput.cpp
	SoundInput.h
	MonroPressingPhaseSpace.cpp
	MonroPressingPhaseSpace.h
	${GLUTILS_PATH}/Trackball2.cpp
	${GLUTILS_PATH}/Trackball2.h
)

add_executable(beatdetect
	beatdetect.cpp
)

target_link_libraries(beatdetect
#	${GLEW_LIBRARY}
	${OPENGL_LIBRARY}
	${GLUT_LIBRARIES}
	${BASS_LIBRARY}
)

target_link_libraries(livespecgl
#	${GLEW_LIBRARY}
	${OPENGL_LIBRARY}
	${GLUT_LIBRARIES}
	${BASS_LIBRARY}
)