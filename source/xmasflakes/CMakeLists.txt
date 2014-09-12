cmake_minimum_required(VERSION 2.6)

#-----------------------------------------------------------------------------
# Adjust your project name and add your source code filenames
#-----------------------------------------------------------------------------

# Enter the name of your project and executable here:
SET(MY_PROJECT_NAME cgbit-xmas)

# Enter a list of C/C++ source and header files (separated by space or ';'):
SET(MY_SOURCE_FILES Xmas.cpp Trackball.h Trackball.cpp)


#-----------------------------------------------------------------------------
# The following doesn't need to be changed for our usual exercise projects:
#-----------------------------------------------------------------------------

# This defines
#   a 'CMake project' with the name specified above 
#   (corresponds to a Visual studio solution, a linux Makefile or XCode proj.)
# that contains
#   a single 'CMake target' with the same name (corresponds to a VC++ project)
# which builds an executable
#   from the source files specified above
#   that links against the required libraries.

project(${MY_PROJECT_NAME})

# custom CMake rule files (e.g. findGLEW)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# required libraries
find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)
find_package(GLEW   REQUIRED)

# include directories
include_directories(
	${OPENGL_INCLUDE_DIR}
	${GLUT_INCLUDE_DIR}
	${GLEW_INCLUDE_DIR}	
)

# source code files (sources *and* headers )
add_executable(${MY_PROJECT_NAME}
	${MY_SOURCE_FILES}
)

# link against these libraries
target_link_libraries(${MY_PROJECT_NAME}
	${GLEW_LIBRARY}
	${GLUT_LIBRARY}	
	${OPENGL_LIBRARY}
)
