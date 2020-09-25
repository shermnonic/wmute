# Wintermute
Code repository of [386dx25](https://www.386dx25.de), started in December 2012.
Imported to github in September 2020.

## CMake remarks
* All projects use the [CMake build system](http://www.cmake.org).
* All projects should include cmake/WintermuteCommon.cmake providing some global definitions. For a local environment it should sufficient to adjust the WINTERMUTE_OUTPUT_PATH. It points to the base directory for binary and library outputs which are automatically put in the subfolders /bin and /lib respectively. Wintermute libraries should provide corresponding CMake export() functions.

## Unordered TODOs (outdated)
* check for global WMUTE_BIN_PATH environment variable in CMakeLists
* introduce WMUTE_DATA_PATH define; alternative are links in filesystem
* decide whether to search shaders in data path or locally
* add global INSTALL paths, at least for shared (export) libraries (WMUTE_INSTALL_LIB_DIR)
* add global debug lib postfix (CMAKE_DEBUG_POSTFIX)

## Projects to include: (outdated)
* panorama
* glvidfx (problem with 64bit ffmpeg)
* depthdark
