Getting projects automatically configured with CMake.
=====================================================
Max Hermann, January 6, 2012

Issues:
-------
- Elegant handling of different debug/release versions/libraries
  (e.g. see http://stackoverflow.com/questions/2209929/linking-different-libraries-for-debug-and-release-builds-in-cmake-on-windows
	target_link_libraries( MyEXE debug 3PDebugLib)
	target_link_libraries( MyEXE optimized 3PReleaseLib)

Qt
--
- built-in CMake script
- checks for qmake executable, so simply put it into path
  (note that absolute paths are prescribed in a manual Qt built)

OpenMesh
--------
- custom CMake script
- set environment variable OPENMESHDIR
- now automatically sets up debug library
  (debug lib needed else VS Debug config will crash)
  (setting directory with pdb enables source code browsing in VS)
  (library names may be different between source build and binaries!)
  
Boost
-----
- built-in CMake script
- set environment variables BOOST_LIBRARYDIR and BOOST_ROOT


Matlab
-----
- built-in CMake script (outdated and not versatile)
- hardcoded check for registry entry MATLABROOT of specific Matlab version


GLEW
----
- set environment variable GLEW_PATH
  (automatically looks for GLEW_PATH/include and GLEW_PATH/lib)

SDL
---
- set environment variable SDLDIR

OpenNI
------
- set environment variable OPEN_NI_ROOT 
  either to 32 or 64 bit library, depending on your default project


General notes on CMake
======================
January 11, 2012


Use GLOB and SOURCE_GROUP
-------------------------

	set(NAME foobar)

	file(GLOB ROOT_SOURCE *.cpp)
	file(GLOB ROOT_HEADER *.h)

	file(GLOB_RECURSE CORE_SOURCE ./core/*.cpp)
	file(GLOB_RECURSE CORE_HEADER ./core/*.h)

	file(GLOB_RECURSE GUI_SOURCE ./gui/*.cpp)
	file(GLOB_RECURSE GUI_HEADER ./gui/*.h)

	source_group("Core" FILES ${CORE_SOURCE})
	source_group("Core" FILES ${CORE_HEADER})
	source_group("GUI" FILES ${GUI_SOURCE})
	source_group("GUI" FILES ${GUI_HEADER})

	add_executable(${NAME} 
		${ROOT_SOURCE}    ${ROOT_HEADER} 
		${CORE_SOURCE}    ${CORE_HEADER}
		${GUI_SOURCE}     ${GUI_HEADER}
	)
