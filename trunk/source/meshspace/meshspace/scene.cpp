// scene - minimalistic scene graph library
// Max Hermann, Jan 2014
#include "Scene.h"
#ifdef WIN32
#include <Windows.h> // required to include before GL.h
#endif
#include <GL/glew.h>
#include <GL/GL.h>

//#define SCENE_USE_IMMEDIATE_MODE

namespace scene
{	
//-----------------------------------------------------------------------------
void Scene::render( int flags )
{
	ObjectPtrArray::iterator it = m_objects.begin();
	for( unsigned i=0; it != m_objects.end(); ++it, i++ )
	{
		glPushName( i );

		if( (*it)->isVisible() )
			(*it)->render( flags );

		glPopName();
	}
}

//-----------------------------------------------------------------------------
void Scene::addSceneObject( ObjectPtr obj )
{
	m_objects.push_back( obj );
}

//-----------------------------------------------------------------------------
BoundingBox Scene::getBoundingBox() const
{
	BoundingBox bbox;
	ObjectPtrArray::const_iterator it = m_objects.begin();
	for( ; it != m_objects.end(); ++it )
	{
		bbox.include( (*it)->getBoundingBox() );
	}
	return bbox;
}

//-----------------------------------------------------------------------------
void MeshObject::setMesh( boost::shared_ptr<meshtools::Mesh> mesh )
{
	using meshtools::Mesh;

	m_mesh = mesh;

	// Compute vertex normals
	meshtools::updateMeshVertexNormals( m_mesh.get() );

	// Setup mesh buffer
	m_meshBuffer.clear();
	m_meshBuffer.addFrame( m_mesh.get() );
}

//-----------------------------------------------------------------------------
void MeshObject::setMeshBuffer( const MeshBuffer& mb )
{
	using meshtools::Mesh;

	// Create reference mesh from first frame in MeshBuffer
	m_mesh = boost::shared_ptr<meshtools::Mesh>( mb.createMesh() );	

	// Copy mesh buffer
	m_meshBuffer = mb;

	// Set some defaults
	m_meshBuffer.setFrame( 0 );
}

//-----------------------------------------------------------------------------
bool MeshObject::addFrame( meshtools::Mesh* mesh )
{
	using meshtools::Mesh;

	// Set reference mesh to first added mesh
	if( m_meshBuffer.numFrames() == 0 )
		m_mesh = boost::shared_ptr<meshtools::Mesh>(mesh);

	// Compute vertex normals
	meshtools::updateMeshVertexNormals( mesh );

	// Add to mesh buffer
	return m_meshBuffer.addFrame( mesh );
}


//-----------------------------------------------------------------------------
void MeshObject::render( int flags )
{
	glPushAttrib( GL_CURRENT_BIT );
	glColor3f( (GLfloat)getColor().r, (GLfloat)getColor().g, (GLfloat)getColor().b );

	if( flags & Object::RenderSurface )
		m_meshBuffer.draw();

	if( flags & Object::RenderPoints )		
		if( flags & Object::RenderNames  )
			m_meshBuffer.drawNamedPoints();
		else
			m_meshBuffer.drawPoints();

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void MeshObject::renderPoints( const std::vector<unsigned>& idx )
{
	m_meshBuffer.drawPoints( idx );
}


//-----------------------------------------------------------------------------
BoundingBox MeshObject::getBoundingBox() const
{	
	// TODO: Cache result
	
	using namespace meshtools;
	
	Mesh* m = m_mesh.get();
	if( !m ) return BoundingBox();
	
	BoundingBox bbox;	
	for( Mesh::ConstVertexIter v_it=m->vertices_begin(); v_it != m->vertices_end(); ++v_it )
	{
		const Mesh::Point & p = m->point(*v_it);
		bbox.include( p[0], p[1], p[2] );
	}	
	return bbox;
}

//-----------------------------------------------------------------------------
double MeshObject::projectVertexNormal( unsigned idx, float x, float y, float z ) const
{
	return m_meshBuffer.projectVertexNormal( idx, x, y, z );
}

} // namespace scene
