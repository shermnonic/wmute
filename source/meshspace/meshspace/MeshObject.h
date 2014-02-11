// MeshObject, part of scene - minimalistic scene graph library
// Max Hermann, Jan 2014
#ifndef MESHOBJECT_H
#define MESHOBJECT_H

#include "scene.h"
#include <boost/shared_ptr.hpp>
#include <meshtools.h>
#include "MeshBuffer.h"

namespace scene {

//-----------------------------------------------------------------------------
// 	MeshObject
//-----------------------------------------------------------------------------
/**
	Scene representation of a triangle mesh (\a meshtools::Mesh).
	
	Internally \a MeshBuffer provides the data structure and rendering 
	functionality. It does not support materials nor texture yet. Rendering is 
	done using OpenGL vertex buffer objects for efficiency. 
	
	The (reference) mesh is stored in a shared pointer throughout the lifetime 
	of an instance of \a MeshObject.
*/
class MeshObject : public Object
{
public:
	MeshObject()
	{}

	///@{ Implementation of \a scene::Object
	void render( int flags=Object::RenderDefault );
	BoundingBox getBoundingBox() const;
	///@}
	
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

	/// Return number of vertices in (reference) mesh
	int numVertices() const { return m_mesh ? (int)m_mesh->n_vertices() : 0; }
	/// Return number of (triangle) faces in (reference) mesh
	int numFaces()    const { return m_mesh ? (int)m_mesh->n_faces() : 0; }
	// Return the currently active frame of \a MeshBuffer
	int curFrame() const { return m_meshBuffer.curFrame(); }

	///@{ Direct access to the underlying \a MeshBuffer instance
	MeshBuffer& meshBuffer() { return m_meshBuffer; }
	const MeshBuffer& meshBuffer() const { return m_meshBuffer; }
	///@}

	/// Wrap \a MeshBuffer::projectVertexNormal()
	double projectVertexNormal( unsigned idx, float x, float y, float z ) const;

private:
	boost::shared_ptr<meshtools::Mesh> m_mesh;  ///< Reference mesh (1st of an animation sequence)
	MeshBuffer m_meshBuffer; ///< Buffer objects and rendering functionality
};

} // namespace scene 

#endif // MESHOBJECT_H
