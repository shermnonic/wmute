#include "Module.h"
#include <sstream>
#include <iostream>

int Module::s_moduleCount = 0;

Module::Module( std::string name )
{
	s_moduleCount++;
	
	// If no name given, set some default name
	if( name.empty() )
	{			
		std::stringstream ss;
		ss << "Unnamed module #" << s_moduleCount;
		setModuleName( ss.str() );
	}
}
