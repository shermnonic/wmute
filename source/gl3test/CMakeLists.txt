# gl3test (Wintermute version)
cmake_minimum_required(VERSION 2.8)
project(gl3test)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

# libs
find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)  # freeglut
find_package(GLEW   REQUIRED)

set(GLM_INCLUDE_DIR "${WINTERMUTE_3RDPARTY_LIBS_PATH}/glm" CACHE PATH "glm include path")

add_executable(gl3test
	gl3test.cpp
)

include_directories(gl3test
	${GLM_INCLUDE_DIR}
	${GLEW_INCLUDE_DIR}
	${GLUT_INCLUDE_DIR}	
)

target_link_libraries(gl3test
	${GLEW_LIBRARY}
	${GLUT_LIBRARY}	
	${OPENGL_LIBRARY}
)