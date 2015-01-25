#include "Module.h"

#include <iostream>
#include <sstream>

//=============================================================================
//  ModuleBase
//=============================================================================

std::map<std::string,int> ModuleBase::s_typeCount;

//-----------------------------------------------------------------------------
Serializable::PropertyTree& ModuleBase::serialize() const
{
	static Serializable::PropertyTree cache;
	cache.clear();
	cache.put("ModuleBase.Type",getModuleType());
	cache.put("ModuleBase.Name",getName());

	// Write parameters and options
	parameters().write( cache );
	options().write( cache );

	return cache;
}

//-----------------------------------------------------------------------------
void ModuleBase::deserialize( Serializable::PropertyTree& pt )
{
	m_moduleTypeName = pt.get("ModuleBase.Type",std::string("<Unknown type>"));
	setName( pt.get("ModuleBase.Name", getDefaultName()) );

	// Read parameters and options
	parameters().read( pt );
	options().read( pt );
}

//-----------------------------------------------------------------------------
std::string ModuleBase::getDefaultName()
{
	std::stringstream ss;
	if( !s_typeCount.count( getModuleType() ) )
		ss << Serializable::getDefaultName();
	else
		ss << getModuleType() << s_typeCount[getModuleType()];
	return ss.str();
}


//=============================================================================
//  ModuleRenderer
//=============================================================================

//-----------------------------------------------------------------------------
Serializable::PropertyTree& ModuleRenderer::serialize() const
{
	static Serializable::PropertyTree cache;
	cache = ModuleBase::serialize();
	cache.put("ModuleRenderer.Position.x",m_position.x);
	cache.put("ModuleRenderer.Position.y",m_position.y);
	return cache;
}

//-----------------------------------------------------------------------------
void ModuleRenderer::deserialize( Serializable::PropertyTree& pt )
{
	ModuleBase::deserialize( pt );

	float
		x = pt.get("ModuleRenderer.Position.x",0.f),
		y = pt.get("ModuleRenderer.Position.y",0.f);

	setPosition( Position(x,y) );
}
