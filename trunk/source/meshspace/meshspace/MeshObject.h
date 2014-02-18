// MeshObject, part of scene - minimalistic scene graph library
// Max Hermann, Jan 2014
#ifndef SCENE_MESHOBJECT_H
#define SCENE_MESHOBJECT_H

#include "scene.h"
#include <set>
#include <boost/shared_ptr.hpp>
#include <meshtools.h>
#include "MeshBuffer.h"
#include "MeshShader.h"

namespace scene {

//-----------------------------------------------------------------------------
// 	MeshObject
//-----------------------------------------------------------------------------
/**
	\brief Scene representation of a triangle mesh (\a meshtools::Mesh).
	
	Internally \a MeshBuffer provides the data structure and rendering 
	functionality. It does not support materials nor texture yet. Rendering is 
	done using OpenGL vertex buffer objects for efficiency. 
	
	The (reference) mesh is stored in a shared pointer throughout the lifetime 
	of an instance of \a MeshObject.

	\a MeshObject provides vertex selection functionality. Internally \a
	MeshShader is used to render the selection color-coded. Further a scalar
	vertex attribute can be visualized by color-coding via \a TransferFunction.
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
	/// Render currently selected vertices as GL_POINTS
	void renderSelectedPoints();

	/// Set reference mesh, overwrites any existing mesh or mesh animation.
	/// Equivalent to calling \a addFrame() on an empty animation sequence.
	void setMesh( boost::shared_ptr<meshtools::Mesh> mesh, bool keepMeshBuffer=false );
	/// Set reference mesh, overwrites any existing mesh or mesh animation.
	/// (Overloaded variant provided for convenience.)
	void setMesh( meshtools::Mesh* mesh, bool keepMeshBuffer=false ) 
	{ 
		setMesh( boost::shared_ptr<meshtools::Mesh>(mesh), keepMeshBuffer );
	}
	/// Directly set mesh buffer. If it contains more than one frame the
	/// reference mesh is defined by the first frame.
	void setMeshBuffer( const MeshBuffer& mb );

	/// Add a frame of a mesh animation with same connectivity as reference.
	/// If no reference was given so far, the first frame takes its role.
	/// Returns true if frame was successfully appended, else false.
	bool addFrame( meshtools::Mesh* mesh );

	/// Returns true if mesh buffer contains more than one frame.
	bool isAnimation() const { return numFrames() > 1; }

	///@{ Wrapper around \a MeshBuffer mesh animation functions
	virtual	unsigned numFrames() const { return m_meshBuffer.numFrames(); }
	virtual void setFrame( int i ) { m_meshBuffer.setFrame(i); }
	virtual int curFrame() const { return m_meshBuffer.curFrame(); }
	///@}

	/// Return number of vertices in (reference) mesh
	int numVertices() const { return m_mesh ? (int)m_mesh->n_vertices() : 0; }
	/// Return number of (triangle) faces in (reference) mesh
	int numFaces()    const { return m_mesh ? (int)m_mesh->n_faces() : 0; }

	///@{ Direct access to the underlying \a MeshBuffer instance
	MeshBuffer& meshBuffer() { return m_meshBuffer; }
	const MeshBuffer& meshBuffer() const { return m_meshBuffer; }
	///@}

	/// Wrap \a MeshBuffer::projectVertexNormal()
	double projectVertexNormal( unsigned idx, float x, float y, float z ) const;

	///@{ Vertex selection
	void selectVertices( const std::vector<unsigned>&, bool selected=true );
	void selectVertex( unsigned idx, bool selected=true );	
	void selectNone();
	///@}

	/// Set scalar field on vertices (e.g. used for color-coding in shader)
	void setScalars( const std::vector<float>& scalars );

	/// Recompile the used \a MeshShader from source
	bool reloadShader();

private:
	boost::shared_ptr<meshtools::Mesh> m_mesh;  ///< Reference mesh (1st frame of an animation sequence)
	MeshBuffer m_meshBuffer; ///< Buffer objects and rendering functionality

	MeshShader m_shader; ///< GLSL shader with support for selection and scalar vertex attributes

	std::vector<float> m_selectionAttribBuffer; ///< Selection vertex attribute, 1.0 for selected vertex, 0.0 else
	std::vector<float> m_scalarAttribBuffer;    ///< Scalar vertex attribute, visualized via color transfer function

	float m_scalarShift, m_scalarScale; // Shift-scale scalars to [0,1]

	std::set<unsigned> m_selectedVertices; ///< Indices of currently selected vertices
};

} // namespace scene 

#endif // SCENE_MESHOBJECT_H
