# e7 HelloGLUT example

add_executable( helloglut
	helloglut.cpp
)

target_link_libraries( helloglut e7
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARY}
	${GLUT_LIBRARIES}
)