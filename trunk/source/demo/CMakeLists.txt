# demo
# September 2013
cmake_minimum_required(VERSION 2.8)
project(demo)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

#-----------------------------------------------------------------------------
#  Wintermute engine
#-----------------------------------------------------------------------------
wmuteImportLibrary(fmodaudio)
wmuteImportLibrary(param)
wmuteImportLibrary(qparam)
wmuteImportLibrary(e8base)
wmuteImportLibrary(e8viewer)

set( E8_LIBRARIES
	fmodaudio
	param
	e8base
	e8viewer
)

set( E8_INCLUDE_DIRS
	../e8/include
	../propertytree/include
	../fmodplay				# FMODAudio
)

#-----------------------------------------------------------------------------
#  Dependencies
#-----------------------------------------------------------------------------
find_package(OpenGL REQUIRED)
find_package(GLEW   REQUIRED)
find_package(GLFW   REQUIRED)
find_package(FMOD   REQUIRED)
find_package(Qt4 COMPONENTS QtCore QtGui)
if( QT_FOUND )
	include(${QT_USE_FILE})
endif( QT_FOUND )	

message(STATUS "GLFW_LIBRARIES=${GLFW_LIBRARIES}")

include_directories(
	${GLFW_INCLUDE_DIRS}
	${GLEW_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}	
	${FMOD_INCLUDE_DIR}
	${E8_INCLUDE_DIRS}
)

set(demo_COMMON_LIBRARIES
	${GLEW_LIBRARY}
	${OPENGL_LIBRARY}
	${FMOD_LIBRARY}
	${E8_LIBRARIES}
)

#-----------------------------------------------------------------------------
#  Modules
#-----------------------------------------------------------------------------
add_library(demo_module
	DemoRenderer.h
	DemoRenderer.cpp
	Oscilloscope.h
	Oscilloscope.cpp
)

#-----------------------------------------------------------------------------
#  Viewer
#-----------------------------------------------------------------------------
add_executable(demo
	demo_viewer.cpp
)

target_link_libraries(demo
	${GLFW_LIBRARIES}
	${demo_COMMON_LIBRARIES}
	demo_module
)

#-----------------------------------------------------------------------------
#  Editor
#-----------------------------------------------------------------------------
if( QT_FOUND )

	set( demo_editor_SOURCE
		demo_editor.cpp
	)

	qt4_automoc( ${demo_editor_SOURCE} )

	# No Qt resources yet
	#set( demo_editor_RCCS demo_editor.qrc )
	#qt4_add_resources( app_RCC_SRCS ${app_RCCS} )

	add_executable( demo_editor
		${demo_editor_SOURCE}
	)

	target_link_libraries( demo_editor
		${QT_LIBRARIES}	
		${demo_COMMON_LIBRARIES}
		demo_module
		qparam
	)

endif( QT_FOUND )