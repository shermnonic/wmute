#ifndef RENDERSET_H
#define RENDERSET_H

#include "Serializable.h"
#include "Parameter.h"
#include "Geometry.h"
#include "Module.h"

#include <vector>
#include <string>
#include <sstream>
#include <map>

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
	virtual ~RenderSet() {};

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
	RenderSetManager();
	~RenderSetManager();
	
	RenderSet* getActiveRenderSet();
	const RenderSet* getActiveRenderSet() const;

	void clear();

protected:
	void setup();

private:
	int m_active;
	std::vector<RenderSet*> m_set;
};

#endif // RENDERSET_H
