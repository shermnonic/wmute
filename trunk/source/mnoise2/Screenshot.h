#ifndef SCREENSHOT_H
#define SCREENSHOT_H
#include <string>

// The following defines are set by CMake configuration
//#define SCREENSHOT_SUPPORT_SDL
//#define SCREENSHOT_SUPPORT_PNG

namespace Screenshot
{
	std::string autoName( std::string prefix="", std::string postfix=".tga" );
	void saveTGA( std::string filename );

	#ifdef SCREENSHOT_SUPPORT_PNG  
	bool savePNG( std::string filename );
	#endif

	#ifdef SCREENSHOT_SUPPORT_SDL
	void saveBMP( std::string filename );
	#endif
}

#endif
