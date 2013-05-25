# mnoise2 (Wintermute version)
cmake_minimum_required(VERSION 2.8)
project(mnoise2)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

# User config
set( MNOISE2_BUILD_COMMAND_LINE_PROGRAM     "FALSE" CACHE BOOL "Build command line version of program (overrides other GUI options)." )
set( MNOISE2_USE_PLAIN_GLUT_INSTEAD_OF_GLUI "FALSE" CACHE BOOL "Override GLUI user interface and use plain GLUT (for debugging)." )
set( MNOISE2_USE_WIIMOTE                    "FALSE" CACHE BOOL "Experimental Wiimote support." )

# Supported program defines :
#   SCREENSHOT_SUPPORT_SDL
#   SCREENSHOT_SUPPORT_PNG
#   WRITTEN_IMAGES_COMMAND_LINE_PROGRAM
#   USE_PLAIN_GLUT_INSTEAD_OF_GLUI 
#   USE_WIIMOTE

if( MNOISE2_BUILD_COMMAND_LINE_PROGRAM )
	add_definitions(-DWRITTEN_IMAGES_COMMAND_LINE_PROGRAM)
endif( MNOISE2_BUILD_COMMAND_LINE_PROGRAM )

if( MNOISE2_USE_PLAIN_GLUT_INSTEAD_OF_GLUI )
	add_definitions(-DUSE_PLAIN_GLUT_INSTEAD_OF_GLUI)
endif( MNOISE2_USE_PLAIN_GLUT_INSTEAD_OF_GLUI )

# TODO:
# - add optional wiiuse dependency and define
# - make libpng/zlib dependency optional
#	- set SCREENSHOT_SUPPORT_PNG accordingly
#	- introduce a CMake generated config header for this
#	  or a set a corresponding compiler option
# - make glew dependency optional

# libs
find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)
find_package(GLEW   REQUIRED)

#find_package(GLUI   REQUIRED)
set( GLUI_INCLUDE_DIR empty CACHE PATH "Glui include path" )
set( GLUI_LIBRARY glui.lib CACHE FILEPATH "Glui library" ) 

# libpng needs zlib
set( LIBPNG_LIBRARY libpng.lib CACHE FILEPATH "libpng library" )
set( LIBPNG_INCLUDE_DIR empty CACHE PATH "libpng include path" )
set( ZLIB_LIBRARY zlib.lib CACHE FILEPATH "zlib library" )
set( ZLIB_INCLUDE_DIR empty CACHE PATH "zlib include path" )

# e7GL classes
set( E7GL_SOURCES 
	GL/GLConfig.h
	GL/GLError.cpp
	GL/GLError.h
#	GL/GLSLProgram.cpp
#	GL/GLSLProgram.h
	GL/GLTexture.cpp
	GL/GLTexture.h
	GL/RenderToTexture.cpp
	GL/RenderToTexture.h
)
set( E7GL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/GL )

# include directories
include_directories(
	${GLEW_INCLUDE_DIR}
	${GLUT_INCLUDE_DIR}	
	${GLUI_INCLUDE_DIR}
	${LIBPNG_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${E7GL_INCLUDE_DIR}
)

add_executable(mnoise2
  mnoise2.cpp MNoise.cpp MarchingCubes.cpp Frustum.cpp ImprovedNoise.cpp
  PerlinNoise.cpp primitives.cpp vector3.cpp matrix4x4.cpp vector4.cpp gl2ps.cpp
  mnoise2.h MNoise.h MarchingCubes.h Frustum.h ImprovedNoise.h
  PerlinNoise.h primitives.h vector3.h matrix4x4.h vector4.h gl2ps.h 
  Screenshot.cpp Screenshot.h
  ${E7GL_SOURCES}
)

target_link_libraries(mnoise2
	${GLEW_LIBRARY}
	${GLUT_LIBRARY}	
	${OPENGL_LIBRARY}
	${GLUI_LIBRARY}
	${LIBPNG_LIBRARY}
	${ZLIB_LIBRARY}
	)