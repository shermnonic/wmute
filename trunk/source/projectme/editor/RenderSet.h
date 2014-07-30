#ifndef RENDERSET_H
#define RENDERSET_H

#include <vector>
#include <string>
#include <map>

//=============================================================================
//  ModuleRenderer
//=============================================================================
class ModuleRenderer
{
public:
	virtual void render() { /*TBD*/ };
};

//=============================================================================
//  RenderArea
//=============================================================================

/**
	\class RenderArea

	- Responsible to manage a single polygon geometry later used as render target.
*/
class RenderArea
{
	/// Simple polygon helper class
	template <typename T, int DIM, int TC>
	class TPolygon
	{
	public:
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

	typedef TPolygon<float,2,2> Polygon;
	
public:
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

private:
	std::string m_name;	
	Polygon     m_poly; ///< 2D polygon w/ tex-coords (geometry to render to)
};

//=============================================================================
//  RenderSet
//=============================================================================

typedef std::vector<RenderArea>                    RenderAreaCollection;
typedef std::multimap<ModuleRenderer*,RenderArea*> RenderAreaModuleMapper;

/**
	\class RenderSet

	- Responsible for rendering editor, preview and final screen output.
	- Manages render areas and mapping to modules.
	- Does *not* manage modules!
	- Does *not* provide user interaction (like trackball camera or polygon 
      editing) but offers utility functoins to pick and edit area vertices.
	- Areas are edited in normalized coordinates [-1,-1]-[1,1].
*/
class RenderSet
{
public:
	enum AreaMode { AreaOutline, AreaBlackWhite };

	/// C'tor, creates a single default RenderArea nearly covering the full domain
	RenderSet();

	///@name Rendering
	///@{
	/// Draw area and screen outlines
	void drawOutline() const;	
	/// Render all attached modules into specified areas
	void render() const;
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
	RenderSet* getActiveRenderSet() { return &m_set; }

private:
	RenderSet m_set;
};

#endif // RENDERSET_H
