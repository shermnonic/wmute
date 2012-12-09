# Common CMake configuration for Wintermute projects
# Dec 2012

set(WINTERMUTE_BIN_PATH "${CMAKE_CURRENT_LIST_DIR}/../../bin"  CACHE PATH "Output binary path")

set(EXECUTABLE_OUTPUT_PATH    ${WINTERMUTE_BIN_PATH})
set(CMAKE_RUNTIME_OUTPUT_PATH ${WINTERMUTE_BIN_PATH})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

set(WINTERMUTE_3RDPARTY_LIBS_PATH "${CMAKE_CURRENT_LIST_DIR}/../3rdparty")
set(WINTERMUTE_GLUTILS_PATH       "${CMAKE_CURRENT_LIST_DIR}/..") # /glutils

file(GLOB_RECURSE WINTERMUTE_GLUTILS_SOURCES ${WINTERMUTE_GLUTILS_PATH}/glutils/*.cpp)
source_group("glutils" FILES ${WINTERMUTE_GLUTILS_SOURCES})

# compiler flags
if(UNIX)
  set( CMAKE_CFLAGS_RELEASE     "-O3 -W -Wall -ansi -pedantic -Wno-unused -DNDEBUG" )
  set( CMAKE_CXX_FLAGS_RELEASE  "-O3    -Wall -ansi -pedantic -Wno-unused -DNDEBUG" )
  set( CMAKE_C_FLAGS_DEBUG      "-g  -W -Wall -ansi -pedantic -Wno-unused -DDEBUG"  )
  set( CMAKE_CXX_FLAGS_DEBUG    "-g  -W -Wall -ansi -pedantic -Wno-unused -DDEBUG"  )
endif(UNIX)
