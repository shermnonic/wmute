#include "ProjectMe.h"
#include "ModuleFactory.h"
#include <iostream>
#include <boost/foreach.hpp>
using std::cerr;
using std::endl;

//----------------------------------------------------------------------------
void ProjectMe::clear()
{
	m_moduleManager   .clear();
	m_renderSetManager.clear();
	m_connections     .clear();
}

//----------------------------------------------------------------------------
void ProjectMe
	::addConnection( ModuleRenderer* src, ModuleRenderer* dst, int channel )
{
	Connection c;
	c.connect( src, dst, channel );
	m_connections.push_back( c );
}

void ProjectMe
	::delConnection( ModuleRenderer* src, ModuleRenderer* dst, int channel )
{
	// Find connection
	std::vector<Connection>::iterator dit = m_connections.end();
	std::vector<Connection>::iterator it = m_connections.begin();
	for( ; it != m_connections.end(); ++it )
	{
		if( it->source().module == src &&
		    it->destination().module == dst &&
		    it->destination().channel == channel )
		{
			dit = it;
			break;
		}
	}

	// Delete item (if found)
	if( dit != m_connections.end() )
	{
		m_connections.erase( dit );

		// Disconnect
		dst->setChannel( channel, -1 );
	}
	else
	{
		cerr << "ProjectMe::delConnection() : Connection not found!" << endl;
	}
}

//----------------------------------------------------------------------------
Serializable::PropertyTree& ProjectMe::serialize() const
{
	static Serializable::PropertyTree cache;
	cache.clear();
	
	cache.put("ProjectMe.Name",getName());
	
	// Serialize modules
	const ModuleManager::ModuleArray& modules = m_moduleManager.modules();
	cache.put("ProjectMe.NumModules",modules.size());
	for( int i=0; i < modules.size(); i++ )
	{
		if( modules.at(i) )
			cache.add_child("ProjectMe.Modules.Module",modules.at(i)->serialize());
		else
			cerr << "ProjectMe::serialize() : Encountered void module pointer!" << endl;
	}
	
	// Serialize active render set
	if( m_renderSetManager.getActiveRenderSet() )
	{
		cache.put("ProjectMe.NumRenderSets",1);
		
		const RenderSet* set = m_renderSetManager.getActiveRenderSet();
		cache.add_child("ProjectMe.RenderSets",set->serialize());
	}
	
	return cache;
}

//----------------------------------------------------------------------------
void ProjectMe::deserialize( Serializable::PropertyTree& pt )
{
	setName( pt.get("ProjectMe.Name",getDefaultName()) );

	// Deserialize modules
	int nModules = pt.get("ProjectMe.NumModules",-1);

	m_moduleManager.clear();
	ModuleManager::ModuleArray& modules = m_moduleManager.modules();
		
	int count=0;
	BOOST_FOREACH( PropertyTree::value_type& v, pt.get_child("ProjectMe.Modules") )
	{
		// Create module instance of specific type
		if( v.first.compare("Module")==0 )
		{
			// Get type name
			ModuleBase base("Foo");
			base.deserialize( v.second );
			std::cout << "Module #" << count 
					    << " type = " << base.getModuleType() << std::endl;

			// Try to create a module with specified type
			ModuleBase* mod =
				ModuleFactory::ref().createInstance( base.getModuleType() );

			if( mod )
			{
				// Module succesfully created!

				// So far, we only support descendent of ModuleRenderer
				ModuleRenderer* mr = dynamic_cast<ModuleRenderer*>(mod);
				if( mr )
				{
					m_moduleManager.addModule( mr );
					mr->deserialize( v.second );
				}
			}
			else
			{
				// Module not available in factory :-(
				std::cerr << "ProjectMe::deserialize() : "
					<< "Could not instantiate module of type \"" 
					<< base.getModuleType() << "\"!" << std::endl;
			}
				
			count++;
		}		
	}
	
	// Deserialize active render set
	if( m_renderSetManager.getActiveRenderSet() )
	{
		int nSets = pt.get("ProjectMe.NumRenderSets",-1);

		RenderSet* set = m_renderSetManager.getActiveRenderSet();

		// WORKAROUND: RenderSet requires pointer to ModuleManager for mapping
		set->setModuleManager( &m_moduleManager );

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
