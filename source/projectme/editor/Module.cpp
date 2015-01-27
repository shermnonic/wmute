#include "Module.h"

#include <iostream>
#include <sstream>

//=============================================================================
//  ModuleBase
//=============================================================================

std::map<std::string,int> ModuleBase::s_typeCount;

ModuleBase::ModuleBase( std::string typeName )
: m_moduleTypeName( typeName )
{
	s_typeCount[typeName]++;
	setName( getDefaultName() );
	m_parameterList.setName("ParameterList");
	m_optionsList.setName("OptionList");
}

ModuleBase::~ModuleBase()
{
	if( s_typeCount.count(m_moduleTypeName) )
		s_typeCount[m_moduleTypeName]--;
}


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

void ModuleBase::deserialize( Serializable::PropertyTree& pt )
{
	m_moduleTypeName = pt.get("ModuleBase.Type",std::string("<Unknown type>"));
	setName( pt.get("ModuleBase.Name", getDefaultName()) );

	// Read parameters and options
	parameters().read( pt );
	options().read( pt );
}

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

ModuleRenderer::ModuleRenderer( std::string typeName )
: ModuleBase( typeName ),
	m_active( "active", true )
{
	// We put nothing into parameters because they may be cleared in the
	// derived module implementation (for instance ShaderModule does this).
	options().push_back( &m_active );
}

void ModuleRenderer::update() 
{ 
	if( m_active.value() ) 
		render(); 
}

Serializable::PropertyTree& ModuleRenderer::serialize() const
{
	static Serializable::PropertyTree cache;
	cache = ModuleBase::serialize();
	cache.put("ModuleRenderer.Position.x",m_position.x);
	cache.put("ModuleRenderer.Position.y",m_position.y);
	return cache;
}

void ModuleRenderer::deserialize( Serializable::PropertyTree& pt )
{
	ModuleBase::deserialize( pt );

	float
		x = pt.get("ModuleRenderer.Position.x",0.f),
		y = pt.get("ModuleRenderer.Position.y",0.f);

	setPosition( Position(x,y) );
}

//=============================================================================
//  ModuleManager
//=============================================================================

ModuleManager::~ModuleManager()
{
	clear();
}

void ModuleManager::clear()
{
    for( unsigned i=0; i < m_modules.size(); i++ )
	{
		// Let's hope that we have a proper OpenGL context!
		m_modules[i]->destroy();
		delete m_modules[i];
	}
	m_modules.clear();
}

void ModuleManager::addModule( ModuleRenderer* module )
{
	m_modules.push_back( module );
}

ModuleRenderer* ModuleManager::findModule( std::string name, std::string type )
{
	// Linear search
	ModuleArray::iterator it = m_modules.begin();
	for( ; it != m_modules.end(); ++it )
	{
		if( name.compare( (*it)->getName() )       == 0 &&
			type.compare( (*it)->getModuleType() ) == 0 )
		{
			return *it;
		}
	}
	return NULL;
}

int ModuleManager::moduleIndex( ModuleRenderer* m )
{
	ModuleArray::iterator it = m_modules.begin();
	for( ; it != m_modules.end(); ++it )
	{
		if( (*it) == m )
			return (int)(it - m_modules.begin());
	}
	return -1;
}

void ModuleManager::update()
{
    for( unsigned i=0; i < m_modules.size(); i++ )
		m_modules[i]->update();
}
