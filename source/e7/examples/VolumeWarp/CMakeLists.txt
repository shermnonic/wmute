# e7 VolumeWarp app

# User config
set( E7_VOLUMEWARP_USE_QT  "TRUE"  CACHE BOOL "Build Qt GUI" )

# Sources
set( volumewarp_SRCS
	volumewarp.cpp
)

# ---- Qt4 support (optional) ----
if( E7_VOLUMEWARP_USE_QT )

	find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
	include(${QT_USE_FILE})	
	
	# generate rules for building source files from the Qt resources
	qt4_add_resources( volumewarp_RCC_SRCS 
		volumewarp.qrc 
	)
	# generate rules for building source files that moc generates
	qt4_wrap_cpp( volumewarp_MOC_SRCS 
		${e7_EngineQt_MOC_HRDS}
	)
	# not sure what the advantage of "automoc" is
	qt4_automoc( 
		${volumewarp_SRCS} 
	)	
	
	# Sources for Qt4 application
	set( volumewarp_ALLSRC
		${volumewarp_SRCS} 
		${volumewarp_MOC_SRCS}  # generated Qt moc sources
		${volumewarp_RCC_SRCS}  # generated Qt resources
		volumewarp.rc           # Visual Studio resource(s), e.g. windows icon
	)
	
	ADD_DEFINITIONS(-DVOLUMEWARP_USE_QT)
	
else( E7_VOLUMEWARP_USE_QT )

	# source for Glut application
	set( volumewarp_ALLSRC
		${volumewarp_SRCS} 
	)
	
endif( E7_VOLUMEWARP_USE_QT )

add_executable( volumewarp
	${volumewarp_ALLSRC}
)

target_link_libraries( volumewarp e7
	${QT_LIBRARIES}  # will be an empty string if Qt4 not configured
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARY}
	${GLUT_LIBRARIES}
)
