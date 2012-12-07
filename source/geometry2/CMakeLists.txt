# geometry (Wintermute version)
# November 2012
cmake_minimum_required(VERSION 2.8)
project(geometry)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

# libs
find_package(OpenGL REQUIRED)
find_package(GLEW   REQUIRED)
find_package(GLUT   REQUIRED)

add_executable(geometry
	main2.cpp
	glutmain.cpp	
	Geometry2.cpp
	Geometry2.h
	Vector.h
	gl2ps.h
	gl2ps.cpp
	Screenshot2.h
	Screenshot2.cpp
)

include_directories(geometry
	${GLEW_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${GLUT_INCLUDE_DIR}
)

target_link_libraries(geometry
	${GLEW_LIBRARY}
	${OPENGL_LIBRARY}
	${GLUT_LIBRARY}		
)