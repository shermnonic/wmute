#include "ProjectMe.h"
#include <iostream>
using std::cerr;
using std::endl;

//----------------------------------------------------------------------------
PropertyTree& ProjectMe::serialize() const
{
	static Serializable::PropertyTree cache;
	cache.clear();
	
	cache.put("ProjectMe.Name",getName());
	
	// Serialize modules
	if( m_moduleManager )
	{
		const ModuleManager::ModuleArray& modules = m_moduleManager->modules();
		cache.put("ProjectMe.NumModules",modules.size());
		for( int i=0; i < modules.size(); i++ )
		{
			if( modules.at(i) )				
				cache.add_child("ProjectMe.Modules",modules.at(i)->serialize());
			else
				cerr << "ProjectMe::serialize() : Encountered void module pointer!" << endl;
		}
	}
	
	// Serialize active render set
	if( m_renderSetManager && m_renderSetManager->getActiveRenderset() )
	{
		RenderSet* set = m_renderSetManager->getActiveRenderset();
		
		cache.add_child("ProjectMe.RenderSets",set->serialize());
	}
}

//----------------------------------------------------------------------------
void ProjectMe::deserialize( Serializable::PropertyTree& pt )
{
}
