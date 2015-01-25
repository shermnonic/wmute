#ifndef MODULE_H
#define MODULE_H

#include "Serializable.h"
#include "Parameter.h"

#include <vector>
#include <string>
#include <map>

//=============================================================================
//  ModuleBase
//=============================================================================
/**
	\class ModuleBase

	Abstract base class for modules, mainly provides type name.
*/
class ModuleBase : public Serializable
{
	static std::map<std::string,int> s_typeCount;

public:
	ModuleBase( std::string typeName )
		: m_moduleTypeName( typeName )
	{
		s_typeCount[typeName]++;
		setName( getDefaultName() );
		m_parameterList.setName("ParameterList");
		m_optionsList.setName("OptionList");
	}

	~ModuleBase()
	{
		if( s_typeCount.count(m_moduleTypeName) )
			s_typeCount[m_moduleTypeName]--;
	}

	/// Return type of module as string
	std::string getModuleType() const { return m_moduleTypeName; }

	/// Function of touch depends on particular module type
	virtual void touch() {}

	///@name Standard serialization of module type name (must be called explicitly by subclasses!)
	///@{ 
	virtual PropertyTree& serialize() const;
	virtual void deserialize( PropertyTree& pt );
	///@}
	
	///@name Live parameters
	///@{ 
	const ParameterList& parameters() const { return m_parameterList; }
	ParameterList& parameters() { return m_parameterList; }
	///@}

	///@name Setup options (require an explicit applyOptions() call)
	///@{
	const ParameterList& options() const { return m_optionsList; }
	ParameterList& options() { return m_optionsList; }	
	virtual void applyOptions() {}
	///@}

	/// Override default name with numbered module type string
	virtual std::string getDefaultName();

protected:
	std::string m_moduleTypeName;
	ParameterList m_parameterList;
	ParameterList m_optionsList;
};


//=============================================================================
//  ModuleRenderer
//=============================================================================
/**
	\class ModuleRenderer

	OpenGL effect which renders into a texture.
*/
class ModuleRenderer : public ModuleBase
{
public:
	ModuleRenderer( std::string typeName )
		: ModuleBase( typeName )
	{}

	///@{ Serialization of node editor hints (e.g. position)
	virtual PropertyTree& serialize() const;
	virtual void deserialize( PropertyTree& pt );
	///@}

	/// Render the effect into a texture
	virtual void render() = 0;
	/// Return the texture id where the effect has rendered into
	virtual int  target() const = 0;
	/// Release any OpenGL resources (assume a valid GL context)
	virtual void destroy() = 0;

	/// @name Input channels
	///@{
	virtual void setChannel( int idx, int texId ) {}
	virtual int  channel( int idx ) const { return -1; }
	virtual int  numChannels() const { return 0; }
	///@}

	/// @name Node editor hints
	///@{
	struct Position { 
		float x, y;
		Position():x(0.),y(0.) {}
		Position( float x_, float y_ ):x(x_),y(y_) {}
	};
	void setPosition( Position p ) { m_position = p; }
	Position position() const { return m_position; }
	///@}

private:
	Position m_position;
};

//=============================================================================
//  ModuleManager
//=============================================================================
/**
	\class ModuleManager

	Manage a set of \a ModuleRenderer instances.
*/
class ModuleManager
{
public:
	typedef std::vector<ModuleRenderer*> ModuleArray;

	~ModuleManager()
	{
		clear();
	}

	void clear()
	{
		for( int i=0; i < m_modules.size(); i++ )
		{
			// Let's hope that we have a proper OpenGL context!
			m_modules[i]->destroy();
			delete m_modules[i];
		}
		m_modules.clear();
	}

	void addModule( ModuleRenderer* module )
	{
		m_modules.push_back( module );
	}

	///@{ Direct access to module pointers (take care!)
	ModuleArray& modules() { return m_modules; }
	const ModuleArray& modules() const { return m_modules; }
	///@}

	/// Returns pointer to first module matching given name and type, else NULL.
	ModuleRenderer* findModule( std::string name, std::string type )
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

	/// Returns index of module or -1 if not found
	int moduleIndex( ModuleRenderer* m )
	{
		ModuleArray::iterator it = m_modules.begin();
		for( ; it != m_modules.end(); ++it )
		{
			if( (*it) == m )
				return (int)(it - m_modules.begin());
		}
		return -1;
	}

	/// Trigger rendering for *all* modules
	void render()
	{
		for( int i=0; i < m_modules.size(); i++ )
			m_modules[i]->render();
	}

private:
	ModuleArray m_modules;
};

#endif // MODULE_H
