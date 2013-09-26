# e8 / viewer
# Max Hermann, September 2013

set(E8VIEWER_INCLUDE_DIR "../include/e8/viewer/")
include_directories(${E8VIEWER_INCLUDE_DIR})

#---- Dependencies ------------------------------------------------------------

find_package(OpenGL REQUIRED)
find_package(GLFW   REQUIRED)

include_directories(
	${GLFW_INCLUDE_DIRS}
	${OPENGL_INCLUDE_DIR}
)

#---- Libraries --------------------------------------------------------------

add_library( e8viewer STATIC
	${E8VIEWER_INCLUDE_DIR}ViewerGLFW.h
	ViewerGLFW.cpp
)

wmuteExportLibrary( e8viewer )

#---- Executables ------------------------------------------------------------

add_executable( e8viewer_test
	testViewerGLFW.cpp
)

target_link_libraries( e8viewer_test
	${GLFW_LIBRARIES}
	${OPENGL_LIBRARY}
	param     # imported
	e8base
	e8viewer
)

