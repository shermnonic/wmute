# e7 HelloNoise example

add_executable( hellonoise
	hellonoise.cpp
)

target_link_libraries( hellonoise e7
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARY}
	${GLUT_LIBRARIES}
)