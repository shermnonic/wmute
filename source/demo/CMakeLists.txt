# demo
cmake_minimum_required(VERSION 2.8)
project(demo)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

# libs
find_package(OpenGL REQUIRED)
find_package(GLEW   REQUIRED)
find_package(GLFW   REQUIRED)
find_package(FMOD   REQUIRED)

message(STATUS "GLFW_LIBRARIES=${GLFW_LIBRARIES}")

add_executable(demo
	demo.cpp
)

include_directories(demo
	${GLEW_INCLUDE_DIR}
	${GLFW_INCLUDE_DIRS}
	${OPENGL_INCLUDE_DIR}
	${FMOD_INCLUDE_DIR}
)

target_link_libraries(demo
	${GLEW_LIBRARY}
	${GLFW_LIBRARIES}
	${OPENGL_LIBRARY}
	${FMOD_LIBRARY}
)