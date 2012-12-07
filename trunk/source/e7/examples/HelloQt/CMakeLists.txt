# e7 HelloQt example

set( e7_EngineQt_MOC_HRDS 
)
#	${E7_LIB_PATH}/Engines/Qt/EngineQt.h

set( helloqt_SRCS
	helloqt.cpp
)

#-------------------
# Qt4
#-------------------
find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
include(${QT_USE_FILE})

# Qt4 resource files
set( helloqt_RCCS helloqt.qrc )

# Qt4 moc headers
set( helloqt_MOC_HDRS
	${e7_EngineQt_MOC_HRDS}
)

#---- Qt4 resources & moc -----------------------------------------------------

# generate rules for building source files from the Qt resources
qt4_add_resources( helloqt_RCC_SRCS ${helloqt_RCCS} )

# generate rules for building source files that moc generates
qt4_wrap_cpp( helloqt_MOC_SRCS ${helloqt_MOC_HDRS} )

# not sure what the advantage of "automoc" is
qt4_automoc( ${helloqt_SRCS} )

#---- Executable --------------------------------------------------------------

add_executable( helloqt
	${helloqt_SRCS}	
	${helloqt_MOC_SRCS}   # generated Qt moc sources
	${helloqt_RCC_SRCS}   # generated Qt resources
	helloqt.rc            # Visual Studio resource(s), e.g. windows icon
)

target_link_libraries( helloqt e7
	${QT_LIBRARIES}
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARY}	
)