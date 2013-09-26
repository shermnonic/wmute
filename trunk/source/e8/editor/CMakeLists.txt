# e8 / editor
# Max Hermann, September 2013

set(E8EDITOR_INCLUDE_DIR "../include/e8/editor/")
include_directories(${E8EDITOR_INCLUDE_DIR})

#---- Dependencies ------------------------------------------------------------

#-------------------
# Qt4
#-------------------
find_package(Qt4 COMPONENTS QtCore QtGui)
include(${QT_USE_FILE})

#---- Imports ----------------------------------------------------------------

#-------------------
#  qparam  
#-------------------
# Lib param is our own property/parameter management library, required by all
# sub projects. Lib param is therefore already included in the parent CMakeList
# There is also a qparam Qt GUI support, but this is only required by the editor 
# so far and therefore imported here.
#
wmuteImportLibrary( qparam )
	#was: include("${WINTERMUTE_INSTALL_LIB_DIR}/qparam-targets.cmake")

#---- Executables ------------------------------------------------------------

add_executable( e8editor
	${E8EDITOR_INCLUDE_DIR}ViewerWidget.h
)

target_link_libraries( e8editor
	param
	qparam
)
