# propertytree
# Max Hermann, August 2013

cmake_minimum_required(VERSION 2.6)
project(propertytree)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)


#---- Dependencies ------------------------------------------------------------

#---------------------------------------------------------
#  Boost
#----------------------------------------------------------
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

#---- Executable --------------------------------------------------------------

add_executable(	testptree 
	main.cpp
)

target_link_libraries( sdmvis 
	${BOOST_USED_LIBRARIES}
)
