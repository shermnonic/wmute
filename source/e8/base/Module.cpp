#include "Module.h"
#include <sstream>
#include <iostream>

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
