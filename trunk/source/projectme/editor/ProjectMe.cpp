#include "ProjectMe.h"
#include <iostream>
#include <boost/foreach.hpp>
using std::cerr;
using std::endl;

//----------------------------------------------------------------------------
Serializable::PropertyTree& ProjectMe::serialize() const
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
				cache.add_child("ProjectMe.Modules.Module",modules.at(i)->serialize());
			else
				cerr << "ProjectMe::serialize() : Encountered void module pointer!" << endl;
		}
	}
	
	// Serialize active render set
	if( m_renderSetManager && m_renderSetManager->getActiveRenderSet() )
	{
		cache.put("ProjectMe.NumRenderSets",1);
		
		RenderSet* set = m_renderSetManager->getActiveRenderSet();
		cache.add_child("ProjectMe.RenderSets",set->serialize());
	}
	
	return cache;
}

//----------------------------------------------------------------------------
void ProjectMe::deserialize( Serializable::PropertyTree& pt )
{
	setName( pt.get("ProjectMe.Name",getDefaultName()) );

	// Deserialize modules
	if( m_moduleManager )
	{
		// TODO: Module loading not supported yet! This makes sense as soon as 
		//       we have some kind of module factory / plugin system!		
		int nModules = pt.get("ProjectMe.NumModules",-1);
		
		// HACK: Assume that module manager has already the correct module types
		//       loaded (e.g. for a hard coded RenderSet as used during development).
		int count=0;
		ModuleManager::ModuleArray& modules = m_moduleManager->modules();
		BOOST_FOREACH( PropertyTree::value_type& v, pt.get_child("ProjectMe.Modules") )
		{
			if( v.first.compare("Module")==0 && count < modules.size() )
			{
				modules.at(count)->deserialize( v.second );
				count++;
			}		
		}
	}
	
	// Deserialize active render set
	if( m_renderSetManager && m_renderSetManager->getActiveRenderSet() )
	{
		int nSets = pt.get("ProjectMe.NumRenderSets",-1);

		RenderSet* set = m_renderSetManager->getActiveRenderSet();
		BOOST_FOREACH( PropertyTree::value_type& v, pt.get_child("ProjectMe.RenderSets") )
		{
			if( v.first.compare("RenderSet")==0 )
			{				
				// WORKAROUND: v.second is the subtree below "RenderSet", but
				//   RenderSet::deserialize() expects a "RenderSet" root node.
				//   That is why 
				//       set->deserialize( v.second );
				//   will not work and a temporary "RenderSet" sub tree is 
				//   employed here.
				Serializable::PropertyTree sub;
				sub.add_child("RenderSet",v.second);
				set->deserialize( sub );

				break; // Only deserialize first RenderSet
			}
		}
	}	
}
