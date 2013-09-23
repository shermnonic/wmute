# propertytree (param libraries)
#
# Max Hermann, August 2013

cmake_minimum_required(VERSION 2.6)
project(propertytree)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

#-------------------
#  Boost
#-------------------
### --> So far we are only using header only dependencies, found in path
find_package(Boost REQUIRED) #COMPONENTS system filesystem program_options 
include_directories(${Boost_INCLUDE_DIR})
message(STATUS "Boost_INCLUDE_DIR=${Boost_INCLUDE_DIR}")
	
#-------------------
# Qt4
#-------------------
find_package(Qt4 COMPONENTS QtCore QtGui)
include(${QT_USE_FILE})


#---- Libraries & Executables -------------------------------------------------

add_subdirectory( "param" )
include_directories( "include" )

#--- param test program ---

add_executable(	testptree 
	testptree.cpp
)

target_link_libraries( testptree 
	param
	${BOOST_USED_LIBRARIES}
)

#--- qparam test program ---
if( QT_FOUND )

	set( qtest_SOURCE
		testptreegui.cpp
	)

	qt4_automoc( ${qtest_SOURCE} )

	# No Qt resources yet
	#set( qapp_RCCS qtest.qrc )
	#qt4_add_resources( app_RCC_SRCS ${app_RCCS} )

	add_executable( testptreegui
		${qtest_SOURCE}
	)

	target_link_libraries( testptreegui
		param
		qparam
		${QT_LIBRARIES}	
		${Boost_LIBRARIES}	
	)

endif( QT_FOUND )