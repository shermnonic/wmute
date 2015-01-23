#ifndef RENDERSET_H
#define RENDERSET_H

#include "Geometry.h"
#include "Parameter.h"

#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <boost/property_tree/ptree.hpp>

//=============================================================================
//  Serializable
//=============================================================================
/**
	\class Serializable

	Simplistic serialization using boost::property_tree.
	Prodivdes a name property.
*/
class Serializable
{
public:
	typedef boost::property_tree::ptree PropertyTree;

	Serializable()
	{
		setName( getDefaultName() );
	}

	virtual PropertyTree& serialize() const = 0;
	virtual void deserialize( PropertyTree& pt ) = 0;

	void serializeToDisk( std::string filename );
	bool deserializeFromDisk( std::string filename );

	///@{ Access name
	std::string getName() const { return m_name; }
	void setName( const std::string& name ) { m_name = name; }
	virtual std::string getDefaultName()
	{
		static int count=0;
		std::stringstream ss;
		ss << "unnamed" << count++;
		return ss.str();
	}
	///@}

private:
	std::string m_name;
};

//=============================================================================
//  ModuleBase
//=============================================================================
/**
	\class ModuleBase

	- Abstract base class for modules, mainly provides type name.
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

	- OpenGL effect which renders into a texture.
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

//=============================================================================
//  RenderArea
//=============================================================================
/**
	\class RenderArea

	- Responsible to manage a single polygon geometry later used as render target.
*/
class RenderArea : public Serializable
{
	///@{ Our geometry types
	typedef Geometry::TPolygon          <float,2,2> Polygon;
	typedef Geometry::THomogeneousQuad2D<float>     HomogeneousQuad;
	///@}
	
public:
	/// C'tor, sets a default render area
	RenderArea();
	/// C'tor, sets axis aligned render area
	RenderArea( float xmin, float ymin, float xmax, float ymax );

	///@{ Draw polygonal area bounds
	void drawAreaOutline() const;
	void drawAreaFilled() const;
	///@}

	/// Render textured area
	void render( int gltexid ) const;

	///@{ Access bounding polygon
	      Polygon& polygon()       { return m_poly; }
	const Polygon& polygon() const { return m_poly; }
	bool  isQuad() const { return m_poly.nverts()==4; }
	///@}

	/// @name Serialization
	///@{
	PropertyTree& serialize() const;
	void deserialize( Serializable::PropertyTree& pt );
	///@}

	/// Possible texture sectors for UV mapping
	enum SplitSectors {
		NoSplit,
		VerticalLeft,
		VerticalRight,
		HorizontalTop,
		HorizontalBottom
	};
	/// Set UV coordinates to show only half of texture
	void setUVSplit( int direction );

private:
	Polygon m_poly; ///< 2D polygon w/ tex-coords (geometry to render to)
	mutable HomogeneousQuad m_hq; ///< Projective mapping if polygon is a quad
	int m_split;
};

//=============================================================================
//  RenderSet
//=============================================================================

class ProjectMe;

typedef std::vector<RenderArea>      RenderAreaCollection;
typedef std::vector<ModuleRenderer*> RenderAreaModuleMapper;

/**
	\class RenderSet

	- Responsible for rendering editor, preview and final screen output.
	- Manages render areas and mapping to modules.
	- Does *not* manage modules!
	- Does *not* provide user interaction (like trackball camera or polygon 
      editing) but offers utility functions to pick and edit area vertices.
	- Areas are edited in normalized coordinates [-1,-1]-[1,1].
*/
class RenderSet : public ModuleRenderer
{
public:
	enum AreaMode { AreaOutline, AreaBlackWhite };

	/// C'tor, creates a single default RenderArea nearly covering the full domain
	RenderSet();

	void clear() { m_areas.clear(); m_mapper.clear(); }

	/// @name ModuleRenderer implementation
	///@{
	void render() { render_internal(-1); } 
	void render(int texid) { render_internal(texid); } // Workaround
	int  target() const { return -1; } // no target, immediate mode rendering
	void destroy() {}
	///@}

	/// @name Input channels
	///@{
	void setChannel( int idx, int texId ); // FIXME: Conflict with addArea(area,module)!
	int  channel( int idx ) const;
	int  numChannels() const;
	///@}

	///@name Rendering
	///@{
	/// Draw area and screen outlines
	void drawOutline() const;	
	/// Render all attached modules into specified areas
	void render_internal( int texid=-1 ) /*const*/;
	///@}

	///@name Area editing
	///@{
	int  pickVertex( float x, float y );
	void getPickedVertexPosition( float& x, float& y ) const;
	void setPickedVertexPosition( float x, float y );
	void unpick() { m_activeArea=-1; }
	///@}

	///@name Render options
	///@{
	int getAreaMode() const { return m_areaMode; }
	void setAreaMode( int i ) { m_areaMode = i; }
	///@}

	///@name Setup
	///@{
	void addArea( RenderArea area, ModuleRenderer* module=0 );
	void setModule( int areaIdx, ModuleRenderer* module );
	///@}

	/// @name Serialization
	///@{
	PropertyTree& serialize() const;
	void deserialize( Serializable::PropertyTree& pt );
	///@}

	///@{name Access
	const RenderAreaCollection& areas() const { return m_areas; }
	RenderAreaCollection& areas() { return m_areas; }
	const RenderAreaModuleMapper& mapper() const { return m_mapper; }
	RenderAreaModuleMapper& mapper() { return m_mapper; }
	///@}

	// WORKAROUND: Module manager is required for deserialization!
	void setModuleManager( ModuleManager* mm ) { m_moduleManager = mm; }
	// WORKAROUND: ProjectMe instance is required for mapping!
	void setProjectMe( ProjectMe* pm ) { m_projectMe = pm; }

protected:
	void beginRendering() const;
	void endRendering()   const;

private:
	//Rect                   m_screenRect;
	std::vector<int> m_channels;

	RenderAreaCollection   m_areas;
	RenderAreaModuleMapper m_mapper;

	ModuleManager* m_moduleManager;
	ProjectMe* m_projectMe;

	int m_activeArea;
	int m_pickedVertex;

	int m_areaMode;
};


//=============================================================================
//  RenderSetManager
//=============================================================================

/**
	\class RenderSetManager

	- Manages (owns) RenderSet(s).
*/
class RenderSetManager
{
public:
	RenderSetManager()
	: m_active(-1)
	{
		setup();
	}
	
	RenderSet* getActiveRenderSet() 
	{ 
		if( m_active >= 0 && m_active < m_set.size() )
			return &m_set.at(m_active);
		return 0; 
	}

	const RenderSet* getActiveRenderSet() const
	{ 
		if( m_active >= 0 && m_active < m_set.size() )
			return &m_set.at(m_active);
		return 0; 
	}

	void clear()
	{
		std::vector<RenderSet>::iterator it = m_set.begin();
		for( ; it != m_set.end(); ++it )
			it->clear();
		m_set.clear();

		setup();
	}

protected:
	void setup()
	{
		// Provide a single RenderSet by default
		m_set.push_back( RenderSet() );
		m_active = 0;
	}

private:
	int m_active;
	std::vector<RenderSet> m_set;
};

#endif // RENDERSET_H
