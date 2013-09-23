# Common CMake configuration for Wintermute projects
# Dec 2012

message("Wintermute common CMake configuration:")

set(WINTERMUTE_BIN_PATH "${CMAKE_CURRENT_LIST_DIR}/../../bin"  CACHE PATH "Output binary path")

set(EXECUTABLE_OUTPUT_PATH    ${WINTERMUTE_BIN_PATH})
set(LIBRARY_OUTPUT_PATH       ${WINTERMUTE_BIN_PATH})
set(CMAKE_RUNTIME_OUTPUT_PATH ${WINTERMUTE_BIN_PATH})

# CMAKE_CURRENT_LIST_DIR only available from CMake version greater 2.8.3
# Workaround from:
# http://code.google.com/p/gqp/source/browse/cmake/Functions.cmake?name=GQP_v0.13.2
if(NOT DEFINED CMAKE_CURRENT_LIST_DIR)
  get_filename_component(CMAKE_CURRENT_LIST_DIR
    ${CMAKE_CURRENT_LIST_FILE} PATH)
endif(NOT DEFINED CMAKE_CURRENT_LIST_DIR)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

message("WINTERMUTE_BIN_PATH = ${WINTERMUTE_BIN_PATH}")
message("CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")
message("CMAKE_CURRENT_LIST_DIR = ${CMAKE_CURRENT_LIST_DIR}")

set(WINTERMUTE_3RDPARTY_LIBS_PATH "${CMAKE_CURRENT_LIST_DIR}/../3rdparty")
set(WINTERMUTE_GLUTILS_PATH       "${CMAKE_CURRENT_LIST_DIR}/..") # /glutils

file(GLOB_RECURSE WINTERMUTE_GLUTILS_SOURCES ${WINTERMUTE_GLUTILS_PATH}/glutils/*.cpp)
source_group("glutils" FILES ${WINTERMUTE_GLUTILS_SOURCES})

# compiler flags
if(UNIX)
  message("Setting specific UNIX gcc flags.")
  set( CMAKE_CFLAGS_RELEASE     "-O3 -W -Wall -ansi -pedantic -Wno-unused -DNDEBUG" )
  set( CMAKE_CXX_FLAGS_RELEASE  "-O3    -Wall -ansi -pedantic -Wno-unused -DNDEBUG" )
  set( CMAKE_C_FLAGS_DEBUG      "-g  -W -Wall -ansi -pedantic -Wno-unused -DDEBUG"  )
  set( CMAKE_CXX_FLAGS_DEBUG    "-g  -W -Wall -ansi -pedantic -Wno-unused -DDEBUG"  )
endif(UNIX)
