# propertytree
# Max Hermann, August 2013

cmake_minimum_required(VERSION 2.6)
project(propertytree)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

#---- Dependencies ------------------------------------------------------------

#-------------------
#  Boost
#-------------------
### --> So far we are only using header only dependencies, found in path
#set(Boost_USE_STATIC_LIBS ON)
#find_package(Boost COMPONENTS 
#	system 
#	filesystem 
#	program_options 
#	REQUIRED)
#add_definitions(-DBOOST_ALL_NO_LIB) # prevent boost automatic linkage on Windows
#set(BOOST_USED_LIBRARIES
#   ${Boost_FILESYSTEM_LIBRARY}
#   ${Boost_SYSTEM_LIBRARY}
#	${Boost_PROGRAM_OPTIONS_LIBRARY}
#)

#-------------------
# Qt4
#-------------------
find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
include(${QT_USE_FILE})


#---- Qt4 Sources -------------------------------------------------------------

# Qt4 resource files
#set( app_RCCS qpropertytree.qrc )

set( app_MOC_HDRS
	PropertyTreeWidget.h
)

set( app_SRCS
	testptree.cpp
	PropertyTreeWidget.cpp
)

#---- Qt4 resources & moc -----------------------------------------------------

# generate rules for building source files from the Qt resources
#qt4_add_resources( app_RCC_SRCS ${app_RCCS} )

# generate rules for building source files that moc generates
qt4_wrap_cpp( app_MOC_SRCS ${app_MOC_HDRS} )

# not sure what the advantage of "automoc" is
qt4_automoc( ${app_SRCS} )


#---- Executables -------------------------------------------------------------

add_library( params
	ParameterBase.h
	ParameterTypes.h
	ParameterIO.h
	ParameterIO.cpp
)

add_executable(	testptree 
	testptree.cpp
)

target_link_libraries( testptree 
	params
	${BOOST_USED_LIBRARIES}
)

add_executable( testptreegui
	testptreegui.cpp
	PropertyTreeWidget.h
	PropertyTreeWidget.cpp
)

target_link_libraries( testptreegui
	params
	${QT_LIBRARIES}
	${BOOST_USED_LIBRARIES}	
)
