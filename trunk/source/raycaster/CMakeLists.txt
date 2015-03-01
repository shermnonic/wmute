# SimpleRaycaster
cmake_minimum_required(VERSION 2.6)
project(SimpleRaycaster)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# libs
find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)
find_package(GLEW   REQUIRED)

include_directories(
	${GLEW_INCLUDE_DIR}
	${GLUT_INCLUDE_DIR}
)

add_executable( SimpleRaycaster
	SimpleRaycaster.h
	SimpleRaycaster.cpp
	GLConfig.h
	GLError.cpp
	GLError.h
	GLSLProgram.cpp
	GLSLProgram.h
	GLTexture.cpp
	GLTexture.h
	RaycastShader.cpp
	RaycastShader.h
	RenderToTexture.cpp
	RenderToTexture.h
	VolumeData.cpp
	VolumeData.h
	Filename.h
	Filename.cpp
	LookupTable.h
	LookupTable.cpp
	main.cpp
	tinyxml2.h
	tinyxml2.cpp
)

target_link_libraries( SimpleRaycaster
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARY}
	${GLUT_LIBRARIES}
)
