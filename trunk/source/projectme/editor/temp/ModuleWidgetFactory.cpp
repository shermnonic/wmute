#include "ModuleWidgetFactory.h"

using namespace std;

ModuleWidget* ModuleWidgetFactory::createInstance( string moduleType )
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

bool ModuleWidgetFactory::registerClass( string moduleType, CreateInstanceCallback cb )
{
	return m_callbacks.insert( CallbackMap::value_type( moduleType, cb ) ).second;
}

vector<string> ModuleWidgetFactory::getAvailableModuleWidgets() const
{
	static vector<string> list;
	list.clear();
	CallbackMap::const_iterator it = m_callbacks.begin();
	for( ; it != m_callbacks.end(); ++it )		
		list.push_back( it->first );
	return list;
}
