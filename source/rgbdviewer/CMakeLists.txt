# rgbdviewer - replay raw depth+color images acquired by rgbd-demo
# Max Hermann, January 14, 2013
cmake_minimum_required(VERSION 2.4)
project(rgbdviewer)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

#---- Dependencies ------------------------------------------------------------

#---------------------------------------------------------
# Boost (e.g. filesystem dependency in included libraries)
# // unix needs filesystem AND system-package by boost 
#----------------------------------------------------------
#find_package(Boost COMPONENTS system filesystem REQUIRED)

#-------------------
# OpenGL
#-------------------
find_package(OpenGL REQUIRED)
find_package(GLEW   REQUIRED)

set(GLM_INCLUDE_DIR  "${WINTERMUTE_3RDPARTY_LIBS_PATH}/glm"    CACHE PATH "glm include path")
set(GLUTILS_PATH     "${WINTERMUTE_GLUTILS_PATH}"              CACHE PATH "path to authors custom glutils")

source_group("glutils" FILES ${WINTERMUTE_GLUTILS_SOURCES})

#-------------------
# Qt4
#-------------------
find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
include(${QT_USE_FILE})

# Qt4 resource files
set( app_RCCS rgbdviewer.qrc )

#-------------------
# Qt4 moc headers
#-------------------
set( app_MOC_HDRS
	RGBDViewerMainWindow.h
	RendererWidget.h
)

#-------------------
# All sources
#-------------------
file(GLOB app_FILES_H    "*.h")
file(GLOB app_FILES_CPP  "*.cpp")
set( app_SRCS	
	${app_FILES_H}
	${app_FILES_CPP}
)

#---- Qt4 resources & moc -----------------------------------------------------

# generate rules for building source files from the Qt resources
qt4_add_resources( app_RCC_SRCS ${app_RCCS} )

# generate rules for building source files that moc generates
qt4_wrap_cpp( app_MOC_SRCS ${app_MOC_HDRS} )

# not sure what the advantage of "automoc" is
qt4_automoc( ${app_SRCS} )

#---- Directories -------------------------------------------------------------

# include directories
include_directories(
	${GLEW_INCLUDE_DIR}
	${GLM_INCLUDE_DIR}
	${GLUTILS_PATH}
)

#---- Executable --------------------------------------------------------------

# build sources, moc'd sources and rcc'd sources
add_executable(	rgbdviewer 
	${app_SRCS}
	${WINTERMUTE_GLUTILS_SOURCES}
	${app_MOC_SRCS}  # generated Qt moc sources
	${app_RCC_SRCS}  # generated Qt resources
	rgbdviewer.rc    # Visual Studio resource(s), e.g. windows application icon
)

target_link_libraries( rgbdviewer 
	${QT_LIBRARIES}
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARY}
    #${Boost_FILESYSTEM_LIBRARY}
    #${Boost_SYSTEM_LIBRARY}
)
