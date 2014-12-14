#include "ModuleFactory.h"

using namespace std;

ModuleBase* ModuleFactory::createInstance( string moduleType )
{
	CallbackMap::const_iterator it = m_callbacks.find( moduleType );
	if( it == m_callbacks.end() )
	{
		// not found
		return NULL;
	}

	// invoke creation function
	return (it->second)();
}

bool ModuleFactory::registerClass( string moduleType, CreateModuleCallback cb )
{
	return m_callbacks.insert( CallbackMap::value_type( moduleType, cb ) ).second;
}

vector<string> ModuleFactory::getAvailableModules() const
{
	static vector<string> list;
	list.clear();
	CallbackMap::const_iterator it = m_callbacks.begin();
	for( ; it != m_callbacks.end(); ++it )		
		list.push_back( it->first );
	return list;
}
