// MeshBuffer - simple OpenGL VBO wrapper with support for mesh animations.
// Max Hermann, Jan 2014
#include "MeshBuffer.h"
#include <GL/glew.h>
#include <glutils/GLError.h>
#include <fstream>

//------------------------------------------------------------------------------
void MeshBuffer::clear()
{
	m_ibuffer.clear();
	m_vbuffer.clear();
	m_nbuffer.clear();
	m_numFrames = 0;
	m_curFrame = -1;
}

//------------------------------------------------------------------------------
void MeshBuffer::setFrame( int f )
{
	// Sanity check
	if( (f < 0) || (f >= (int)m_numFrames) )
	{
		std::cerr << "MeshBuffer::setFrame() : Called with invalid frame number"
		          << std::endl;
		return;
	}
	m_curFrame = f;
	
	m_frameUpdateRequired = true;
}

//------------------------------------------------------------------------------
bool MeshBuffer::addFrame( const meshtools::Mesh* mesh )
{	
	////////////////////////////////////////////////////////////////////////////
	// Remark: We assume that OpenMesh uses consecutive zero-based indices 
	//         as handles to address vertices and faces. The given mesh has to 
	//         be a strict triangle mesh. All meshes have to have the exact 
	//         same connectivity and vertex / normal count.
	////////////////////////////////////////////////////////////////////////////
	
	using meshtools::Mesh;
	
	// --- Fill buffers ---

	// Temporary buffers
	std::vector<float>    vertices; vertices.reserve( mesh->n_vertices()*3 );
	std::vector<float>    normals;  normals .reserve( mesh->n_vertices()*3 );		
	std::vector<unsigned> indices;  indices .reserve(10000);
	
	// Vertex list
	for( int i=0; i < mesh->n_vertices(); i++ )
	{
		Mesh::VertexHandle vh = Mesh::VertexHandle( i );
		
		const Mesh::Point &p = mesh->point( vh );
		vertices.push_back( p[0] );
		vertices.push_back( p[1] );
		vertices.push_back( p[2] );
		
		if( mesh->has_vertex_normals() )
		{	
			const Mesh::Normal & n = mesh->normal( vh );
			normals.push_back( n[0] );
			normals.push_back( n[1] );
			normals.push_back( n[2] );
		}		
	}
	
	// Indexed face list	
	for( int i=0; i < mesh->n_faces(); i++ )
	{
		Mesh::FaceHandle fh( i );
		
		// Get vertex handles for current face fh
		unsigned int count=0;
		std::vector<Mesh::VertexHandle> vhandles; vhandles.reserve(3);
		for( Mesh::CFVIter fv_it = mesh->cfv_begin(fh); fv_it != mesh->cfv_end(fh); ++fv_it )
		{
			vhandles.push_back( *fv_it ); // was: fv_it.handle()
			++count;
		}
		
		// Assert triangle faces
		if( count != 3 )
		{
			std::cerr << "MeshBuffer::addFrame() : Encountered non-triangle "
			             "face in frame #" << m_numFrames++ << std::endl;
			return false;
		}
		
		// Store vertex indices		
		for( size_t j=0; j < 3; ++j )  // Assuming vhandles.size() == 3 !
		{
			indices.push_back( (int)vhandles[j].idx() );
		}
	}

	// --- Sanity checks ---
	
	if( m_numFrames==0 )
	{	
		// Set some defaults for first frame
		m_curFrame = 0;
		m_dirty = true;
		m_frameUpdateRequired = true;
	}
	else
	{
		// This is a later frame and connectivity and vertex / normal count 
		// *must* match already present ones.

		// Check connectivity		
		bool matchingConnectivity = true;
		if( m_ibuffer.size() == indices.size() )
		{
			for( unsigned i=0; i < indices.size(); i++ )
				if( m_ibuffer[i] != indices[i] )
				{
					// Different indices in list
					matchingConnectivity = false;
					break;
				}			
		}
		else
			// Different number of indices
			matchingConnectivity = false;
		
		if( !matchingConnectivity )
		{
			std::cerr << "MeshBuffer::addFrame() : Mismatching connectivity "
				" in frame " << m_numFrames+1 << std::endl;
			return false;
		}
		
		// Check vertex / normal count		
		if( vertices.size()/3 != m_numVertices )
		{
			std::cerr << "MeshBuffer::addFrame() : Mismatching number of "
				" vertices in frame " << m_numFrames+1 << std::endl;
			return false;
		}		
		if( normals.size()/3 != m_numNormals )
		{
			std::cerr << "MeshBuffer::addFrame() : Mismatching number of "
				" normals in frame " << m_numFrames+1 << std::endl;
			return false;
		}
		
		m_dirty = true; // Size of GPU buffer has changed
	}	
	
	// --- Set members ---
	
	// Set connectivity / sanity check connectivity
	if( m_numFrames==0 )
	{		
		// This is the first frame and defines the connectivity.
		m_ibuffer = indices;
		
		// The first frame also defines vertex / normal count
		m_numVertices = (unsigned)vertices.size() / 3;
		m_numNormals  = (unsigned)normals.size() / 3;
	}
	
	// Append data to existing buffers
	m_vbuffer.insert( m_vbuffer.end(), vertices.begin(), vertices.end() );
	m_nbuffer.insert( m_nbuffer.end(), normals.begin(), normals.end() );
	
	m_numFrames++;

	return true;
}

//------------------------------------------------------------------------------
void MeshBuffer::setupCBuffer()
{
	m_cbuffer.clear();
	m_cbuffer.resize( m_numVertices*4, 1.f );
#if 0 // Assign green color for debugging purposes
	for( unsigned i=0; i < m_numVertices; i++ )
	{
		m_cbuffer[4*i  ] = 0.f;
		m_cbuffer[4*i+1] = 1.f;
		m_cbuffer[4*i+2] = 0.f;
		m_cbuffer[4*i+3] = 1.f;
	}
#endif
}

//------------------------------------------------------------------------------
void MeshBuffer::downloadGPU()
{
	// Sanity checks
	if( (m_curFrame < 0) || (m_numFrames==0) || (m_curFrame>=(int)m_numFrames) )
	{
		std::cout << "MeshBuffer::downloadGPU() : Invalid mesh frame!" << std::endl;
		return;
	}
	if( m_ibuffer.empty() || m_vbuffer.empty() || m_nbuffer.empty() )
	{
		std::cout << "MeshBuffer::donwloadGPU() : Empty buffers!" << std::endl;
		return;
	}
	
	if( !m_initialized )
	{
		// Create OpenGL buffers
		glGenBuffers( 1, &m_vbo );
		glGenBuffers( 1, &m_ibo );
		m_initialized = true;		

		GL::CheckGLError("MeshBuffer::downloadGPU() - glGenBuffers()");

		// FIXME: Add error checking and also delete buffers in the end!
	}	

	if( m_dirty )
	{	
		size_t nfloats = m_numVertices*3 + m_numNormals*3 + m_numVertices*4; //m_cbuffer.size();

		// (Re-)allocate buffer (for a single frame)
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
		glBufferData( GL_ARRAY_BUFFER, 
			sizeof(float) * nfloats, NULL,
			GL_STATIC_DRAW );
		
		// Download index buffer (static for all frames)
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_ibo );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER,
			sizeof(unsigned) * m_ibuffer.size(), &(m_ibuffer[0]),
			GL_STATIC_DRAW );
		
		m_dirty = false;

		// Unbind buffers
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

		GL::CheckGLError("MeshBuffer::downloadGPU() - dirty buffers");
	}
	
	// Current frame offset in vertex / normal buffer
	unsigned ofs = m_numVertices*3 * m_curFrame;

	glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
	
	// Download vertices (of current frame)
	size_t start = 0;
	glBufferSubData( GL_ARRAY_BUFFER, start,
		   sizeof(float)*m_numVertices*3, &(m_vbuffer[ofs]) );
	start += sizeof(float)*m_numVertices*3;

	// Download normals (of current frame)	
	glBufferSubData( GL_ARRAY_BUFFER, start,
		   sizeof(float)*m_numNormals*3, &(m_nbuffer[ofs]) );
	start += sizeof(float)*m_numNormals*3;

	// Download colors (optional)
	if( m_cbuffer.empty() )
		setupCBuffer();
	{
		if( m_cbuffer.size() != m_numVertices*4 )
		{
			std::cout << "MeshBuffer::downloadGPU() : Color buffer mismatch, must be of format RGBA!" << std::endl;
			m_cbuffer.clear();
		}
		else
		{
			glBufferSubData( GL_ARRAY_BUFFER, start,
				sizeof(float)*m_cbuffer.size(), &(m_cbuffer[0]) );
		}
	}

	GL::CheckGLError("MeshBuffer::downloadGPU() - glBufferSubData()");

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	GL::CheckGLError("MeshBuffer::downloadGPU()");
}

//------------------------------------------------------------------------------
void MeshBuffer::draw()
{
	// Draw from GPU buffers

	// Sanity checks
	if( !m_initialized || m_dirty || m_frameUpdateRequired )
	{
		downloadGPU();
		m_frameUpdateRequired = false;
	}

	bool useCBuffer = m_cbufferEnabled && !m_cbuffer.empty();
	
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_ibo );

	GL::CheckGLError( "MeshBuffer::draw() - glBindBuffer()" );
	
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_VERTEX_ARRAY );

	GL::CheckGLError( "MeshBuffer::draw() - glEnableClientState()" );

	if( useCBuffer )
	{
		glEnableClientState( GL_COLOR_ARRAY );
		glColorPointer( 4, GL_FLOAT, 0, (void*)(sizeof(float)*m_numVertices*3+sizeof(float)*m_numNormals*3) );
	}

	GL::CheckGLError( "MeshBuffer::draw() - glEnableClientState(), glBindBuffer(), GL_COLOR_ARRAY" );
	
	glNormalPointer( GL_FLOAT, 0, (GLvoid*)(sizeof(float)*m_numVertices*3) );
	glVertexPointer( 3, GL_FLOAT, 0, 0 );
	glIndexPointer( GL_INT, 0, 0 );

	GL::CheckGLError( "MeshBuffer::draw() - gl[Normal/Vertex/Index]Pointer()" );
	
	glDrawElements( GL_TRIANGLES, (GLsizei)m_ibuffer.size(), GL_UNSIGNED_INT, 0 );

	GL::CheckGLError( "MeshBuffer::draw() - glDrawElements( GL_TRIANGELS, ... )" );
	
	glDisableClientState( GL_VERTEX_ARRAY );	
	glDisableClientState( GL_NORMAL_ARRAY );
	if( useCBuffer )
		glDisableClientState( GL_COLOR_ARRAY );

	GL::CheckGLError( "MeshBuffer::draw() - glDisableClientState()" );
	
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	GL::CheckGLError( "MeshBuffer::draw() - glBindBuffer(..., 0)" );
}

//------------------------------------------------------------------------------
void MeshBuffer::drawNamedPoints() const
{
	static std::vector<unsigned> empty_idx;
	drawNamedPoints( empty_idx );
}

void MeshBuffer::drawNamedPoints( const std::vector<unsigned>& idx ) const
{
	// Current frame offset in vertex / normal buffer
	unsigned ofs = m_numVertices*3 * m_curFrame;	

	//glInitNames(); // <- QGLViewer has already taken care of this (?)

	// Draw points in immediate mode, each with a different selection name	
	if( idx.empty() )
	{
		// Draw all points
		for( unsigned i=0; i < m_numVertices; i++ )
		{		
			glPushName( i );
			glBegin( GL_POINTS );
			glVertex3fv( &(m_vbuffer[ofs + 3*i]) );
			glEnd();
			glPopName();
		}
	}
	else
	{
		// Draw given indices
		for( unsigned i=0; i < idx.size(); i++ )
		{		
			glBegin( GL_POINTS );
			glPushName( i );
			glVertex3fv( &(m_vbuffer[ofs + 3*i]) );
			glPopName();
			glEnd();
		}
	}
}

//------------------------------------------------------------------------------
void MeshBuffer::drawPoints() const
{
	static std::vector<unsigned> empty_idx;
	drawPoints( empty_idx );
}

void MeshBuffer::drawPoints( const std::vector<unsigned>& idx ) const
{
	// Draw from CPU vertex arrays

	// Current frame offset in vertex / normal buffer
	unsigned ofs = m_numVertices*3 * m_curFrame;	
	
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, &(m_vbuffer[ofs]) );

	if( !idx.empty() )
		glDrawElements( GL_POINTS, (GLsizei)idx.size(), GL_UNSIGNED_INT, &(idx[0]) );
	else
		glDrawArrays( GL_POINTS, 0, m_numVertices );

	glDisableClientState( GL_VERTEX_ARRAY );
}

//------------------------------------------------------------------------------
void MeshBuffer::write( const char* filename ) const
{
	using namespace std;
	ofstream of( filename, ios_base::binary );
	if( !of.is_open() )
	{
		cerr << "MeshBuffer::write() : Could not open " << filename << endl;
		return;
	}

	const char magic[] = "MESHBUFFER";
	unsigned nframes  = numFrames(),
		     ntris    = (unsigned)m_ibuffer.size()/3,
			 nverts   = m_numVertices, // for *all* frames: m_vbuffer.size()/3 = m_numVerts*m_numFrames
			 nnorms   = m_numNormals;

	of.write( magic, sizeof(magic) );
	of.write( (char*)&nframes, sizeof(unsigned) );
	of.write( (char*)&ntris,   sizeof(unsigned) );
	of.write( (char*)&nverts,  sizeof(unsigned) );
	of.write( (char*)&nnorms,  sizeof(unsigned) );
	of.write( (char*)&m_ibuffer[0], m_ibuffer.size()*sizeof(unsigned) );
	of.write( (char*)&m_vbuffer[0], m_vbuffer.size()*sizeof(float) );
	of.write( (char*)&m_nbuffer[0], m_nbuffer.size()*sizeof(float) );
	of.close();
}

//------------------------------------------------------------------------------
bool MeshBuffer::read( const char* filename )
{
	using namespace std;
	ifstream f( filename, ios_base::binary );
	if( !f.is_open() )
	{
		cerr << "MeshBuffer::read() : Could not open " << filename << endl;
		return false;
	}

	// Read header
	char magic[] = "MESHBUFFER";
	unsigned nframes,
		     ntris,
			 nverts,
			 nnorms;

	f.read( magic, sizeof(magic) );

	if( string(magic) != string("MESHBUFFER") )
	{
		cerr << "MeshBuffer::read() : " << filename << "is not a valid MESHBUFFER file!" << endl;
		return false;
	}

	f.read( (char*)&nframes, sizeof(unsigned) );
	f.read( (char*)&ntris,   sizeof(unsigned) );
	f.read( (char*)&nverts,  sizeof(unsigned) );
	f.read( (char*)&nnorms,  sizeof(unsigned) );

	// Read buffers
	unsigned* ibuffer = new unsigned[ ntris*3 ];
	float* vbuffer = new float[ nframes*nverts*3 ];
	float* nbuffer = new float[ nframes*nnorms*3 ];

	f.read( (char*)&ibuffer[0], sizeof(unsigned)*ntris*3 );
	f.read( (char*)&vbuffer[0], sizeof(float)*nframes*nverts*3 );
	f.read( (char*)&nbuffer[0], sizeof(float)*nframes*nnorms*3 );

	// Close file
	f.close();

	// Setup MeshBuffer
	clear();
	m_numFrames   = nframes;
	m_numVertices = nverts;
	m_numNormals  = nnorms;
	m_ibuffer.insert( m_ibuffer.begin(), &ibuffer[0], &ibuffer[0] + ntris*3 );
	m_vbuffer.insert( m_vbuffer.begin(), &vbuffer[0], &vbuffer[0] + nframes*nverts*3 );
	m_nbuffer.insert( m_nbuffer.begin(), &nbuffer[0], &nbuffer[0] + nframes*nnorms*3 );

	// Free memory
	delete [] ibuffer;
	delete [] vbuffer;
	delete [] nbuffer;

	return true;
}

//------------------------------------------------------------------------------
meshtools::Mesh* MeshBuffer::createMesh( int frame ) const
{
	typedef meshtools::Mesh Mesh;

	std::vector< Mesh::VertexHandle > vhandle( m_numVertices );
	std::vector< Mesh::VertexHandle > fvhandle( 3 );
		
	Mesh* m = new Mesh;

	// sanity check
	if( frame >= m_numFrames || frame < 0 )
	{
		std::cout << "MeshBuffer::createMesh() : Requested invalid frame number "
			<< frame << ", falling back to first frame!" << std::endl;
		frame = 0;
	}

	// Mesh vertex data
	unsigned ofs = (unsigned)frame * m_numVertices * 3;		
	const float* pv = &(m_vbuffer[ofs]);
	for( unsigned i=0; i < m_numVertices; ++i )
	{
		Mesh::Point p;
		p[0] = *pv;  pv++;
		p[1] = *pv;  pv++;
		p[2] = *pv;  pv++;
		vhandle[i] = m->add_vertex( p );
	}

	// Mesh indexed faces
	const unsigned* pf = &(m_ibuffer[0]);
	for( size_t i=0; i < m_ibuffer.size()/3; ++i )
	{
		fvhandle[0] = vhandle[*pf];  pf++;
		fvhandle[1] = vhandle[*pf];  pf++;
		fvhandle[2] = vhandle[*pf];  pf++;
		m->add_face( fvhandle );
	}

	// FIXME: Avoid recomputation and copy existing normals
	meshtools::updateMeshVertexNormals( m );

	return m;
}

//------------------------------------------------------------------------------
double MeshBuffer::projectVertexNormal( unsigned idx, float x, float y, float z ) const
{
	// Current frame offset in vertex / normal buffer
	unsigned ofs = m_numVertices*3 * m_curFrame;

	// Select vertex
	ofs += 3*idx;

	float nx = m_nbuffer[ofs ],
		  ny = m_nbuffer[ofs + 1],
		  nz = m_nbuffer[ofs + 2];

	return nx*x + ny*y + nz*z;
}
