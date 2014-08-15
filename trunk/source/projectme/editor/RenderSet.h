#ifndef RENDERSET_H
#define RENDERSET_H

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
	static std::string getDefaultName()
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
//  ModuleRenderer
//=============================================================================
/**
	\class ModuleRenderer

	- OpenGL effect which renders into a texture.
*/
class ModuleRenderer : public Serializable
{
public:
	ModuleRenderer( std::string typeName )
		: m_moduleTypeName( typeName )
	{}

	/// Render the effect into a texture
	virtual void render() = 0;
	/// Return the texture id where the effect has rendered into
	virtual int  target() const = 0;
	/// Release any OpenGL resources (assume a valid GL context)
	virtual void destroy() = 0;

	/// Return type of module as string
	std::string getModuleType() const { return m_moduleTypeName; }

private:
	std::string m_moduleTypeName;
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

	ModuleArray& modules() { return m_modules; }
	const ModuleArray& modules() const { return m_modules; }

	void addModule( ModuleRenderer* module )
	{
		m_modules.push_back( module );
	}

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
	/// Simple polygon helper class
	template <typename T, int DIM, int TC>
	class TPolygon
	{
	public:
		void clear() { m_verts.clear(); m_texcoords.clear(); }

		void resize( int n ) { m_verts.resize( DIM * n ); m_texcoords.resize( TC * n ); }	
		int  nverts() const { return (int)m_verts.size() / DIM; }

		      T* vert( int i )       { return (i<0 || i>=m_verts.size()) ? 0 : &m_verts[DIM*i]; }
		const T* vert( int i ) const { return &m_verts[DIM*i]; }

		      T* texcoord( int i )       { return (i<0 || i>=m_texcoords.size()) ? 0 : &m_texcoords[TC*i]; }
		const T* texcoord( int i ) const { return &m_texcoords[TC*i]; }

		std::vector<T>& verts() { return m_verts; }
		std::vector<T>& texcoords() { return m_texcoords; }

	private:
		std::vector<T> m_verts;
		std::vector<T> m_texcoords;
	};

	/// Our 2D polygon type
	typedef TPolygon<float,2,2> Polygon;
	
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
	///@}

	/// @name Serialization
	///@{
	PropertyTree& serialize() const;
	void deserialize( Serializable::PropertyTree& pt );
	///@}

private:
	Polygon     m_poly; ///< 2D polygon w/ tex-coords (geometry to render to)
};

//=============================================================================
//  RenderSet
//=============================================================================

typedef std::vector<RenderArea>      RenderAreaCollection;
typedef std::vector<ModuleRenderer*> RenderAreaModuleMapper;

/**
	\class RenderSet

	- Responsible for rendering editor, preview and final screen output.
	- Manages render areas and mapping to modules.
	- Does *not* manage modules!
	- Does *not* provide user interaction (like trackball camera or polygon 
      editing) but offers utility functoins to pick and edit area vertices.
	- Areas are edited in normalized coordinates [-1,-1]-[1,1].
*/
class RenderSet : public Serializable
{
public:
	enum AreaMode { AreaOutline, AreaBlackWhite };

	/// C'tor, creates a single default RenderArea nearly covering the full domain
	RenderSet();

	void clear() { m_areas.clear(); m_mapper.clear(); }

	///@name Rendering
	///@{
	/// Draw area and screen outlines
	void drawOutline() const;	
	/// Render all attached modules into specified areas
	void render( int texid=-1 ) const;
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

protected:
	void beginRendering() const;
	void endRendering()   const;

private:
	//Rect                   m_screenRect;
	RenderAreaCollection   m_areas;
	RenderAreaModuleMapper m_mapper;

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
		// Provide a single RenderSet by default
		m_set.push_back( RenderSet() );
		m_active = 0;
	}
	
	RenderSet* getActiveRenderSet() 
	{ 
		if( m_active >= 0 && m_active < m_set.size() )
			return &m_set.at(m_active);
		return 0; 
	}

private:
	int m_active;
	std::vector<RenderSet> m_set;
};

#endif // RENDERSET_H
