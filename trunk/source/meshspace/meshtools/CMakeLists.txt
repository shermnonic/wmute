# meshtools library - Some basic 3D triangle mesh operations (sub-project)
# Max Hermann, Jan 2014

#---- Library -----------------------------------------------------------------

add_library( meshtools
	meshtools.cpp
)

meshtoolsExportLibrary( meshtools )

#---- Executables -------------------------------------------------------------

#---------------------
# meshvolume
#---------------------
add_executable( meshvolume
	meshvolume.cpp
)
target_link_libraries( meshvolume
	meshtools
	${OPENMESH_LIBRARIES}
)

#---------------------
# meshicp
#---------------------
add_executable( meshicp
	meshicp.cpp
	${MESHTOOLS_3RDPARTY_INCLUDE_DIR}/ICP.h
	${MESHTOOLS_3RDPARTY_INCLUDE_DIR}/nanoflann.hpp	
)
target_link_libraries( meshicp
	meshtools
	${OPENMESH_LIBRARIES}
)

