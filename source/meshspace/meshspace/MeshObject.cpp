// MeshObject, part of scene - minimalistic scene graph library
// Max Hermann, Jan 2014
#include "MeshObject.h"
#include <GL/glew.h>
#include <GL/GL.h>
#include <glutils/GLError.h>
#include <vector>
#include <iostream>
#include <sstream>

namespace scene
{

//-----------------------------------------------------------------------------
void MeshObject::setMesh( boost::shared_ptr<meshtools::Mesh> mesh, bool keepMeshBuffer )
{
	using meshtools::Mesh;

	m_mesh = mesh;

	// Compute vertex normals
	meshtools::updateMeshVertexNormals( m_mesh.get() );

	// Setup mesh buffer
	if( !keepMeshBuffer )
	{
		m_meshBuffer.clear();
		m_meshBuffer.addFrame( m_mesh.get() );
	}
}

//-----------------------------------------------------------------------------
void MeshObject::setMeshBuffer( const MeshBuffer& mb )
{
	using meshtools::Mesh;

	// Copy mesh buffer
	m_meshBuffer = mb;

	// Set some defaults
	m_meshBuffer.setFrame( 0 );

	updateMesh();
}

//-----------------------------------------------------------------------------
void MeshObject::updateMesh()
{
	// Create reference mesh from first frame in MeshBuffer
	m_mesh = boost::shared_ptr<meshtools::Mesh>( m_meshBuffer.createMesh() );	
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

bool MeshObject::reloadShader()
{
	bool success = m_shader.init();
	if( !success )
	{
		std::cerr << "MeshObject::reloadShader() - could not reload shader!"
			<< std::endl;
	}
	return success;
}

//-----------------------------------------------------------------------------
void MeshObject::render( int flags )
{
	bool useShader = (m_shaderMode != NoShader);

	glPushAttrib( GL_CURRENT_BIT );
	glColor3f( (GLfloat)getColor().r, (GLfloat)getColor().g, (GLfloat)getColor().b );

	// Render shaded scene
	if( flags & Object::RenderSurface )
	{
		int selectionLoc, scalarLoc;

		if( useShader )
		{
			// Initialize shader (if required)
			if( !m_shader.isInitialized() )
				reloadShader();

			m_shader.bind();

			// Selection attribute
			selectionLoc = m_shader.program()->getAttribLocation("selection");
			if( !m_selectionAttribBuffer.empty() && selectionLoc >= 0 ) 
			{
				glVertexAttribPointer( selectionLoc, 1, GL_FLOAT, GL_FALSE, 0, 
					(GLvoid*)(&m_selectionAttribBuffer[0]) );
				glEnableVertexAttribArray( selectionLoc );
			}

			// Scalar attribute
			scalarLoc = m_shader.program()->getAttribLocation("scalar");
			if( !m_scalarAttribBuffer.empty() && scalarLoc >= 0 )
			{
				// Attribute buffer
				glVertexAttribPointer( scalarLoc, 1, GL_FLOAT, GL_FALSE, 0, 
					(GLvoid*)(&m_scalarAttribBuffer[0]) );
				glEnableVertexAttribArray( scalarLoc );

				// Uniforms
				glUniform1f( m_shader.program()->getUniformLocation("scalarShift"), m_scalarShift );
				glUniform1f( m_shader.program()->getUniformLocation("scalarScale"), m_scalarScale );
				glUniform1i( m_shader.program()->getUniformLocation("mapScalars"), 1 );
			}
			else
			{
				glUniform1i( m_shader.program()->getUniformLocation("mapScalars"), 0 );
			}
		}
		
		m_meshBuffer.draw();

		if( useShader )
		{
			if( selectionLoc>=0 ) glDisableVertexAttribArray( selectionLoc );
			if( scalarLoc>=0 )    glDisableVertexAttribArray( scalarLoc );
			m_shader.release();
		}
	}

	// Render vertices as points
	if( flags & Object::RenderPoints )
		if( flags & Object::RenderNames  )
			m_meshBuffer.drawNamedPoints();
		else
			m_meshBuffer.drawPoints();

	glPopAttrib();

	GL::CheckGLError( "MeshObject::render()" );
}

//-----------------------------------------------------------------------------
void MeshObject::renderPoints( const std::vector<unsigned>& idx )
{
	m_meshBuffer.drawPoints( idx );
}

//-----------------------------------------------------------------------------
void MeshObject::renderSelectedPoints()
{	
	if( m_selectedVertices.empty() ) return;
	// TODO: Update index vector only on change in selection set.
	std::vector<unsigned> idx;
	idx.insert( idx.begin(), m_selectedVertices.begin(), m_selectedVertices.end() );
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

//------------------------------------------------------------------------------
void MeshObject::selectVertices( const std::vector<unsigned>& idx, bool selected )
{
	unsigned numVerts = numVertices();

	// Create new selection attribute buffer, initialized with 0.0
	if( m_selectionAttribBuffer.size() != numVerts )
	{
		m_selectionAttribBuffer.clear();
		m_selectionAttribBuffer.resize( numVerts, 0.0 );
	}
	// Set selected vertices to 1.0
	for( unsigned i=0; i < idx.size(); i++ )
	{	
		unsigned id = idx[i];
		if( id < numVerts ) {
			m_selectionAttribBuffer[id] = selected ? 1.f : 0.f;

			if( selected )
			{
				// Add index to selected set
				m_selectedVertices.insert( id );
			}
			else
			{
				// Remove index from selected set
				std::set<unsigned>::iterator it = m_selectedVertices.find( id );
				if( it != m_selectedVertices.end() )
					m_selectedVertices.erase( it );
			}
		}
	}
}

//------------------------------------------------------------------------------
void MeshObject::selectVertex( unsigned idx, bool selected )
{
	std::vector<unsigned> tmp;
	tmp.push_back( idx );
	selectVertices( tmp, selected );
}

//------------------------------------------------------------------------------
void MeshObject::selectNone()
{
	m_selectedVertices.clear();
	m_selectionAttribBuffer.clear();
	m_selectionAttribBuffer.resize( numVertices(), 0.0 );
}

//------------------------------------------------------------------------------
void MeshObject::setScalars( const std::vector<float>& scalars, bool autoscale )
{
	m_scalarAttribBuffer.clear();
	m_scalarAttribBuffer.insert( m_scalarAttribBuffer.begin(),
		scalars.begin(), scalars.end() );

	if( autoscale )
	{
		// Find min/max value
		float minval=std::numeric_limits<float>::max(),
			  maxval=-minval;	
		for( unsigned int i=0; i < scalars.size(); i++ )
		{
			maxval = scalars[i] > maxval ? scalars[i] : maxval;
			minval = scalars[i] < minval ? scalars[i] : minval;
		}

		// Set parameters to rescale to [0,1] inside shader
		m_scalarShift = -minval;
		m_scalarScale = 1.f / (maxval - minval);		
	}
}

//------------------------------------------------------------------------------
void MeshObject::setScalarShiftScale( float shift, float scale ) 
{ 
	m_scalarShift = shift; 
	m_scalarScale = scale; 
	//std::cout << "Scalars shift=" << m_scalarShift << ", scale=" << m_scalarScale << std::endl;
}

//------------------------------------------------------------------------------
meshtools::Mesh* MeshObject::createMesh( int frame ) /*const*/
{
	if( frame < 0 ) frame = meshBuffer().curFrame();

	if( m_scalarAttribBuffer.size() == meshBuffer().numVertices() )
	{
		std::cout << "Applying transfer function to mesh" << std::endl;
		meshBuffer().setupCBuffer();
		for( unsigned i=0; i < m_scalarAttribBuffer.size(); i++ )
		{
			float scalar = (m_scalarScale * m_scalarAttribBuffer.at(i)) + m_scalarShift;
			float r,g,b;

			m_shader.getTransferFunction().getColor( scalar, r,g,b );

			meshBuffer().cbuffer().at(4*i+0) = r;
			meshBuffer().cbuffer().at(4*i+1) = g;
			meshBuffer().cbuffer().at(4*i+2) = b;
			meshBuffer().cbuffer().at(4*i+3) = 1.f;
		}
	}

	return meshBuffer().createMesh( frame );
}

} // namespace scene
