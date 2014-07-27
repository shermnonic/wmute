if (OPENMESH_INCLUDES AND OPENMESH_LIBRARIES)
  #set(OPENMESH_FIND_QUIETLY TRUE)
  return()
endif (OPENMESH_INCLUDES AND OPENMESH_LIBRARIES)


find_path(OPENMESH_INCLUDES
  NAMES
  OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh
  PATHS
  $ENV{OPENMESHDIR}
  $ENV{OPENMESHDIR}/include
  $ENV{OPENMESHDIR}/src
  ${INCLUDE_INSTALL_DIR}
  PATH_SUFFIXES  
)

# Note: In the default VS2008 solution of OpenMesh 2.01 the library is called
# "libeOpenMeshCore.lib" instead of "openmeshcore.lib" as previously.
find_library(OPENMESH_LIBRARY_RELEASE
	OpenMeshCore
	PATHS 
	$ENV{OPENMESHDIR} 
	$ENV{OPENMESHDIR}/lib
	$ENV{OPENMESHDIR}/Build/lib
	${LIB_INSTALL_DIR} 
	PATH_SUFFIXES 
	OpenMesh
)

find_library(OPENMESH_LIBRARY_DEBUG
	OpenMeshCored 
	PATHS 
	$ENV{OPENMESHDIR} 
	$ENV{OPENMESHDIR}/lib
	$ENV{OPENMESHDIR}/Build/lib
	${LIB_INSTALL_DIR} 
	PATH_SUFFIXES 
	OpenMesh
)

# if only release is found, set debug library to release
if(OPENMESH_LIBRARY_RELEASE AND NOT OPENMESH_LIBRARY_DEBUG)
	set(OPENMESH_LIBRARY_DEBUG ${OPENMESH_LIBRARY_RELEASE})
	if(NOT OPENMESH_INCLUDES AND OPENMESH_LIBRARIES)
		MESSAGE( STATUS "OpenMesh debug library not found. Note that using release version in debug mode may lead to a crash on Windows!" )
	endif(NOT OPENMESH_INCLUDES AND OPENMESH_LIBRARIES)
endif(OPENMESH_LIBRARY_RELEASE AND NOT OPENMESH_LIBRARY_DEBUG)

set(OPENMESH_LIBRARIES optimized ${OPENMESH_LIBRARY_RELEASE} debug ${OPENMESH_LIBRARY_DEBUG})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OPENMESH DEFAULT_MSG
                                  OPENMESH_INCLUDES OPENMESH_LIBRARIES)

mark_as_advanced(OPENMESH_INCLUDES OPENMESH_LIBRARIES)
