# e7 Raycaster app

add_executable( raycaster
	raycaster.cpp
)

target_link_libraries( raycaster e7
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARY}
	${GLUT_LIBRARIES}
)