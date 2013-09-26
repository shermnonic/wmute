# fmodplay (Wintermute)
# Test fmod API
cmake_minimum_required(VERSION 2.8)
project(fmodplay)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/WintermuteCommon.cmake)

#---- Dependencies ------------------------------------------------------------

#-------------------
#  FMOD
#-------------------
find_package(FMOD REQUIRED)

#---- Libraries (export) ------------------------------------------------------

add_library(fmodaudio STATIC
	FMODAudio.h
	FMODAudio.cpp
)
wmuteExportLibrary(fmodaudio)

#---- Executables -------------------------------------------------------------

add_executable(fmodplay
	fmodplay.cpp
)

include_directories(fmodplay
	${FMOD_INCLUDE_DIR}
)

target_link_libraries(fmodplay
	${FMOD_LIBRARY}
	fmodaudio
)