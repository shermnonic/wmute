# e8 / base
# Max Hermann, September 2013

set(E8BASE_INCLUDE_DIR "../include/e8/base/")
include_directories(${E8BASE_INCLUDE_DIR})

#---- Libraries --------------------------------------------------------------

add_library( e8base STATIC
	${E8BASE_INCLUDE_DIR}AbstractInteractor.h
	${E8BASE_INCLUDE_DIR}AbstractRenderer.h
	${E8BASE_INCLUDE_DIR}Module.h
	${E8BASE_INCLUDE_DIR}ViewerInterface.h
	Module.cpp
)

wmuteExportLibrary( e8base )
