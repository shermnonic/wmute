#include "Connection.h"
#include "ProjectMe.h"
#include "ShaderModule.h"
#include <iostream>
#include <string>
using std::cerr;
using std::cout;
using std::endl;
using std::string;

void Connection::update()
{
	if( isConnected() )
	{
		ShaderModule* sm = dynamic_cast<ShaderModule*>( m_dst.module );
		if( sm )
		{
			// WORKAROUND:
			// ShaderModules have special setChannel() function to also
			// extract information about channel texture size
			sm->setChannel( m_dst.channel, m_src.module );
		}
		else
			m_dst.module->setChannel( m_dst.channel, m_src.module->target() );
	}
}

Serializable::PropertyTree& Connection::serialize() const 
{ 
	static Serializable::PropertyTree cache;
	cache.clear();

	// Only store valid connection
	if( !isConnected() )
		return cache; // Return empty cache

	cache.put("Name",getName());
	cache.put("Source.ModuleName",m_src.module->getName());
	cache.put("Source.ModuleType",m_src.module->getModuleType());
	cache.put("Destination.ModuleName",m_dst.module->getName());
	cache.put("Destination.ModuleType",m_dst.module->getModuleType());
	cache.put("Destination.Channel",m_dst.channel);

	return cache; 
}

void Connection::deserialize( Serializable::PropertyTree& pt )
{
	if( !m_projectMe )
	{
		cerr << "Connection::deserialize() : ProjectMe instance not set!" << endl;
		return;
	}

	setName( pt.get("Name",getDefaultName()) );

	// Read connection info
	string 
		srcName = pt.get("Source.ModuleName",string("")),
		srcType = pt.get("Source.ModuleType",string("")),
		dstName = pt.get("Destination.ModuleName",string("")),
		dstType = pt.get("Destination.ModuleType",string(""));
	int
		channel = pt.get("Destination.Channel",-1);

	if( srcName.empty() || srcType.empty() || dstName.empty() || dstType.empty() )
	{
		cerr << "Connection::deserialize() : Invalid connection data!" << endl;
		return;
	}

	// Get module pointers (requires module manager already be deserialized)
	ModuleRenderer
		*src  = m_projectMe->moduleFromNameAndType( srcName, srcType ),
		*dst  = m_projectMe->moduleFromNameAndType( dstName, dstType );

	if( !src || !dst )
	{
		cerr << "Connection::deserialize() : Module(s) not found!" << endl;
		return;
	}

	// Establish connection
	connect( src, dst, channel );
}
