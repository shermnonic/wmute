# bluexgl (Wintermute version)
# January 2013
cmake_minimum_required(VERSION 2.8)
project(bluexgl)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

# libs
#find_package(GLEW   REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)

add_executable(bluexgl
	bluexgl.cpp
)

include_directories(bluexgl
#	${GLEW_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${GLUT_INCLUDE_DIR}
)

target_link_libraries(bluexgl
#	${GLEW_LIBRARY}
	${OPENGL_LIBRARY}
	${GLUT_LIBRARY}		
)