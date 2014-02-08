// scene - minimalistic scene graph library
// Max Hermann, Jan 2014
#ifndef SCENE_H
#define SCENE_H

#include <meshtools.h>
#include "MeshBuffer.h"

#include <boost/shared_ptr.hpp>
#include <vector>
#include <limits>
#include <iostream>

namespace scene
{	
//-----------------------------------------------------------------------------
// 	BoundingBox
//-----------------------------------------------------------------------------
/// Bounding box class used by scenegraph objects
struct BoundingBox
{
	BoundingBox()
	{
		max[0] = max[1] = max[2] = -std::numeric_limits<double>::max();
		min[0] = min[1] = min[2] = +std::numeric_limits<double>::max();
		mid[0] = mid[1] = mid[2] = 0.;
	}
	
	///@{ Extend this bounding box to include the given 3D point
	void include( double x, double y, double z )
	{
		if( x > max[0] ) max[0] = x;
		if( x < min[0] ) min[0] = x;
		if( y > max[1] ) max[1] = y;
		if( y < min[1] ) min[1] = y;
		if( z > max[2] ) max[2] = z;
		if( z < min[2] ) min[2] = z;

		update_mid();		
	}	
	void include( double* v ) { include(v[0],v[1],v[2]); }
	void include( float*  v ) { include(v[0],v[1],v[2]); }
	///@}

	/// Extend this bounding box to include given bounding box
	void include( const BoundingBox& other )
	{
		for( unsigned i=0; i < 3; i++ )
		{
			min[i] = std::min( other.min[i], min[i] );
			max[i] = std::max( other.max[i], max[i] );
		}

		update_mid();
	}

	/// Update mid point (internally called by all \a include() functions)
	void update_mid()
	{
		mid[0] = .5*(min[0]+max[0]);
		mid[1] = .5*(min[1]+max[1]);
		mid[2] = .5*(min[2]+max[2]);
	}

	void print( std::ostream& os=std::cout ) const
	{
		os << "Scene bounding box = "
		   << "(" << min[0] << "," << min[1] << "," << min[2] << ") - "
		   << "(" << max[0] << "," << max[1] << "," << max[2] << ")" << std::endl;
	}

	double radius() const
	{
		double v[3];
		v[0] = max[0] - mid[0];
		v[1] = max[1] - mid[1];
		v[2] = max[2] - mid[2];
		return sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
	}
	
	double min[3],  ///< Minimum cooridinates of bounding box
		   max[3],  ///< Maximum cooridinates of bounding box
		   mid[3];  ///< Mid point of bounding box
};

//-----------------------------------------------------------------------------
// 	Color
//-----------------------------------------------------------------------------
/// Color class used by scenegraph objects
struct Color
{
	Color(): r(1.),g(1.),b(1.),a(1.) {}
	Color(double R,double G,double B,double A=1.): r(R),g(G),b(B),a(A) {}
	double r,g,b,a;
};

//-----------------------------------------------------------------------------
// 	Object
//-----------------------------------------------------------------------------
/**
	Abstract scenegraph object

	One has to implement the following interface:
	- render()
	- getBoundingBox()

	Common properties of every scene object:
	- Name
	- Color (for use in graph widget, false coloring, bounding box, halo)
	- Visibility state
	- Bounding box
	- Transformation matrix (w.r.t. father node, not yet available)
*/
class Object
{
public:
	enum RenderFlags { 
		RenderSurface=1, 
		RenderPoints =2, 
		RenderNames  =4,
		RenderDefault=RenderSurface
	};

	Object()
	: m_name("(unnamed)"),
	  m_visible(true)
	{}
	
	virtual void render( int flags=RenderDefault )=0;
	virtual BoundingBox getBoundingBox() const=0;

	std::string getName() const { return m_name; }
	Color       getColor() const { return m_color; }
	bool        isVisible() const { return m_visible; }
	
	void setName( std::string name ) { m_name = name; }
	void setColor( Color color ) { m_color = color; }
	void setVisible( bool show ) { m_visible = show; }

private:
	std::string m_name;
	Color       m_color;
	bool        m_visible;
};

//-----------------------------------------------------------------------------
// 	MeshObject
//-----------------------------------------------------------------------------
/**
	Very simplistic rendering of a triangle mesh (\a meshtools::Mesh).
	
	Does not support materials nor texture. Rendering is done using OpenGL
	vertex buffer objects for efficiency. The mesh is stored in a shared 
	pointer throughout the lifetime of an instance.
*/
class MeshObject : public Object
{
public:
	MeshObject()
	{}

	void render( int flags=Object::RenderDefault );
	BoundingBox getBoundingBox() const;
	
	/// Render specified vertices as GL_POINTS. (No index check is performed.)
	void renderPoints( const std::vector<unsigned>& idx );

	/// Set reference mesh, overwrites any existing mesh or mesh animation.
	/// Equivalent to calling \a addFrame() on an empty animation sequence.
	void setMesh( boost::shared_ptr<meshtools::Mesh> mesh );
	/// Set reference mesh, overwrites any existing mesh or mesh animation.
	/// (Overloaded variant provided for convenience.)
	void setMesh( meshtools::Mesh* mesh ) 
	{ 
		setMesh( boost::shared_ptr<meshtools::Mesh>(mesh) );
	}
	/// Directly set mesh buffer. If it contains more than one frame the
	/// reference mesh is defined by the first frame.
	void setMeshBuffer( const MeshBuffer& mb );

	/// Add a frame of a mesh animation with same connectivity as reference.
	/// If no reference was given so far, the first frame takes its role.
	/// Returns true if frame was successfully appended, else false.
	bool addFrame( meshtools::Mesh* mesh );

	/// Returns true if mesh buffer contains more than one frame.
	bool isAnimation() const { return m_meshBuffer.numFrames() > 1; }

	///@{ Wrapper around MeshBuffer mesh animation functions
	unsigned numFrames() const { return m_meshBuffer.numFrames(); }
	void setFrame( int i ) { m_meshBuffer.setFrame(i); }
	///@}

	int numVertices() const { return m_mesh ? (int)m_mesh->n_vertices() : 0; }
	int numFaces()    const { return m_mesh ? (int)m_mesh->n_faces() : 0; }

	int curFrame() const { return m_meshBuffer.curFrame(); }

	MeshBuffer& meshBuffer() { return m_meshBuffer; }
	const MeshBuffer& meshBuffer() const { return m_meshBuffer; }

	/// Wrap \a MeshBuffer::projectVertexNormal()
	double projectVertexNormal( unsigned idx, float x, float y, float z ) const;

private:
	boost::shared_ptr<meshtools::Mesh> m_mesh;  ///< Reference mesh (1st of an animation sequence)
	MeshBuffer m_meshBuffer; ///< Buffer objects and rendering functionality
};

//-----------------------------------------------------------------------------
// 	Scene
//-----------------------------------------------------------------------------

/// Shared pointer to scenegraph object
typedef boost::shared_ptr< Object > ObjectPtr;
/// Array type of scenegraph object pointers
typedef std::vector< ObjectPtr >    ObjectPtrArray;

/**
	Minimalistic scenegraph (so far not a graph but simply a set of objects)

	Currently objects can only be added and there is no possibility to
	remove them from the scene yet.
*/
class Scene
{
public:	
	/// Render all visible objects of the scene
	void render( int flags=Object::RenderDefault );

	/// Add an object to the scene
	void addSceneObject( ObjectPtr obj );
	/// Provided for convenience
	void addSceneObject( Object* obj )	
	{ 
		addSceneObject( ObjectPtr( obj ) ); 
	}

	/// Access current set of object in the scene
	ObjectPtrArray& objects() { return m_objects; }

	/// Get bounding box of whole scene
	BoundingBox getBoundingBox() const;

private:
	ObjectPtrArray m_objects;
};

} // namespace scene

#endif // SCENE_H
