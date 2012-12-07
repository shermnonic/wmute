# e7 library + examples (Wintermute version)
cmake_minimum_required(VERSION 2.8)
project(e7)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

# libs
find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)
find_package(GLEW   REQUIRED)

# compiler flags
if(UNIX)
  set( CMAKE_CFLAGS_RELEASE     "-O3 -W -Wall -ansi -pedantic -Wno-unused -DNDEBUG" )
  set( CMAKE_CXX_FLAGS_RELEASE  "-O3    -Wall -ansi -pedantic -Wno-unused -DNDEBUG" )
  set( CMAKE_C_FLAGS_DEBUG      "-g  -W -Wall -ansi -pedantic -Wno-unused -DDEBUG"  )
  set( CMAKE_CXX_FLAGS_DEBUG    "-g  -W -Wall -ansi -pedantic -Wno-unused -DDEBUG"  )
endif(UNIX)

# e7 paths
set( E7_LIB_PATH "${PROJECT_SOURCE_DIR}/e7" )
set( E7_GLM_PATH "${WINTERMUTE_3RDPARTY_LIBS_PATH}/glm" )

# include directories
include_directories(
	${GLEW_INCLUDE_DIR}
	${GLUT_INCLUDE_DIR}	
	${E7_LIB_PATH}
	${E7_GLM_PATH}
)

# add projects to generate
add_subdirectory(e7)
add_subdirectory(Examples)
