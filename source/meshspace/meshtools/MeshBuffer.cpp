// MeshBuffer - simple OpenGL VBO wrapper with support for mesh animations.
// Max Hermann, Jan 2014
#include "MeshBuffer.h"
#include <GL/glew.h>
#include <glutils/GLError.h>
#include <fstream>
#include <iostream>
#include <limits> // std::numeric_limits()
#include <cmath>  // std::sqrt()

//------------------------------------------------------------------------------
void MeshBuffer::clear()
{
	m_ibuffer.clear();
	m_vbuffer.clear();
	m_nbuffer.clear();
	m_numFrames = 0;
	m_curFrame = -1;
	m_vcount.clear();
	m_ncount.clear();
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
	std::vector<float>    colors;   colors  .reserve( mesh->n_vertices()*4 );
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

		if( mesh->has_vertex_colors() )
		{
			const Mesh::Color & c = mesh->color( vh );
			colors.push_back( c[0] / 255.f );
			colors.push_back( c[1] / 255.f );
			colors.push_back( c[2] / 255.f );
			colors.push_back( 1.f ); // Default alpha=1.f
		}
	}
	
	// Indexed face list	
	bool has_faces = (mesh->n_faces() > 0);
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

	// Just read number of vertices/normals
	unsigned numVerts = (unsigned)vertices.size() / 3;
	unsigned numNorms = (unsigned)normals.size() / 3;	
	
	if( m_numFrames==0 )
	{	
		// Set some defaults for first frame
		m_curFrame = 0;
		m_dirty = true;
		m_frameUpdateRequired = true;
	}
	else if( has_faces )
	{		
		// This is a later frame *with* connectivity such that is required that
		// and vertex / normal count *must* match already present ones.

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
		if( numVerts != m_numVertices )
		{
			std::cerr << "MeshBuffer::addFrame() : Mismatching number of "
				" vertices in frame " << m_numFrames+1 << std::endl;
			return false;
		}		
		if( numNorms != m_numNormals )
		{
			std::cerr << "MeshBuffer::addFrame() : Mismatching number of "
				" normals in frame " << m_numFrames+1 << std::endl;
			return false;
		}
		
		m_dirty = true; // Size of GPU buffer has changed
	}	
	else // !has_faces
	{
		// This is a later frame *without* connectivity that is treated as
		// point cloud. Here vertex correspondence is not required and vertex
		// and normal counts may differ between frames.

		// Store maximum number of vertices/normals
		m_numVertices = std::max( m_numVertices, numVerts );
		m_numNormals  = std::max( m_numNormals , numNorms );

		// Buffer size has to be updated
		m_dirty = true;
	}

	
	// --- Set members ---
	
	// Set connectivity / sanity check connectivity
	if( m_numFrames==0 )
	{		
		// This is the first frame and defines the connectivity.
		m_ibuffer = indices;
		
		// The first frame also defines vertex / normal count
		m_numVertices = numVerts;
		m_numNormals  = numNorms;

		// Use color from first mesh
		if( mesh->has_vertex_colors() )
		{
			m_cbufferEnabled = true;
			m_cbuffer = colors;
		}

		m_vcount.clear();
		m_ncount.clear();
	}

	m_vcount.push_back( numVerts );
	m_ncount.push_back( numNorms );
	
	// Append data to existing buffers
	m_vbuffer.insert( m_vbuffer.end(), vertices.begin(), vertices.end() );
	m_nbuffer.insert( m_nbuffer.end(), normals.begin(), normals.end() );

	m_numFrames++;

	return true;
}

//------------------------------------------------------------------------------
void MeshBuffer::initSingleFrameFromRawBuffers()
{
	// Set some defaults
	m_curFrame = 0;
	m_dirty = true;
	m_frameUpdateRequired = true;	
	
	m_numFrames   = 1;
	m_numVertices = (unsigned)m_vbuffer.size() / 3;
	m_numNormals  = (unsigned)m_nbuffer.size() / 3;

	m_cbufferEnabled = m_cbuffer.size() == 4*m_numVertices;
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
	if( m_vbuffer.empty() || m_nbuffer.empty() )
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
		// Buffers are pre-allocated with maximum size
		size_t nfloats = m_numVertices*3 + m_numNormals*3 + m_numVertices*4; //m_cbuffer.size();

		// (Re-)allocate buffer (for a single frame)
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
		glBufferData( GL_ARRAY_BUFFER, 
			sizeof(float) * nfloats, NULL,
			GL_STATIC_DRAW );
		
		// Download index buffer (static for all frames)
		if( !m_ibuffer.empty() ) // We also support point clouds w/o connectivity
		{
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_ibo );
			glBufferData( GL_ELEMENT_ARRAY_BUFFER,
				sizeof(unsigned) * m_ibuffer.size(), &(m_ibuffer[0]),
				GL_STATIC_DRAW );
		}
		
		m_dirty = false;

		// Unbind buffers
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

		GL::CheckGLError("MeshBuffer::downloadGPU() - dirty buffers");
	}
	
	// Current frame offset in vertex / normal buffer
	unsigned ofs = ofsVertex(m_curFrame); // was: m_numVertices*3 * m_curFrame;
	// Tightly pack data into buffers
	unsigned nv = numFrameVertices(m_curFrame); // was: m_numVertices
	unsigned vsize = sizeof(float)*nv*3;
	unsigned nn = numFrameNormals(m_curFrame); // was: m_numNormals
	unsigned nsize = sizeof(float)*nn*3;

	glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
	
	// Download vertices (of current frame)
	size_t start = 0;
	glBufferSubData( GL_ARRAY_BUFFER, start, vsize, &(m_vbuffer[ofs]) );
	start += vsize;

	// Download normals (of current frame)	
	glBufferSubData( GL_ARRAY_BUFFER, start, nsize, &(m_nbuffer[ofs]) );
	start += nsize;

	// Download colors (optional)
	//if( m_cbufferEnabled ) // Upload buffer in any case to allow enabling color on-the-fly
	{
		if( m_cbuffer.size() != m_numVertices*4 )
		{
			std::cout << "MeshBuffer::downloadGPU() : Color buffer mismatch, must be of format RGBA!" << std::endl;
			setupCBuffer();
		}

		glBufferSubData( GL_ARRAY_BUFFER, start,
			sizeof(float)*m_cbuffer.size(), &(m_cbuffer[0]) );
	}

	GL::CheckGLError("MeshBuffer::downloadGPU() - glBufferSubData()");

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	GL::CheckGLError("MeshBuffer::downloadGPU()");
}

//------------------------------------------------------------------------------
void MeshBuffer::sanity()
{
	// Sanity checks
	if( !m_initialized || m_dirty || m_frameUpdateRequired )
	{
		downloadGPU();
		m_frameUpdateRequired = false;
	}
}

//------------------------------------------------------------------------------
void MeshBuffer::draw()
{
	// Draw from GPU buffers
	sanity();

	bool useCBuffer = false; // HACK, was: m_cbufferEnabled && !m_cbuffer.empty();
	
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
	if( !m_ibuffer.empty() ) // We also support point clouds w/o connectivity
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_ibo );

	GL::CheckGLError( "MeshBuffer::draw() - glBindBuffer()" );
	
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_VERTEX_ARRAY );

	GL::CheckGLError( "MeshBuffer::draw() - glEnableClientState()" );

	size_t vsize = sizeof(float)*numFrameVertices(m_curFrame)*3; // was: sizeof(float)*m_numVertices*3
	size_t nsize = sizeof(float)*numFrameNormals (m_curFrame)*3; // was: sizeof(float)*m_numNormals*3

	if( useCBuffer )
	{
		glEnableClientState( GL_COLOR_ARRAY );
		glColorPointer( 4, GL_FLOAT, 0, (void*)(vsize+nsize) );
	}

	GL::CheckGLError( "MeshBuffer::draw() - glEnableClientState(), glBindBuffer(), GL_COLOR_ARRAY" );
	
	glNormalPointer( GL_FLOAT, 0, (GLvoid*)(vsize) ); // was: m_numVertices
	glVertexPointer( 3, GL_FLOAT, 0, 0 );
	glIndexPointer( GL_INT, 0, 0 );

	GL::CheckGLError( "MeshBuffer::draw() - gl[Normal/Vertex/Index]Pointer()" );
	
	if( !m_ibuffer.empty() )
	{
		// Indexed triangle face set
		glDrawElements( GL_TRIANGLES, (GLsizei)m_ibuffer.size(), GL_UNSIGNED_INT, 0 );
	}
	else
	{
		// Point cloud		
		unsigned npoints = numFrameVertices(m_curFrame); // was: m_vbuffer.size()/3
		glEnable( GL_PROGRAM_POINT_SIZE );
		glDrawArrays( GL_POINTS, (GLint)0, (GLsizei)(npoints) );
		glDisable( GL_PROGRAM_POINT_SIZE );
	}

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
	unsigned ofs = ofsVertex(m_curFrame); // was: m_numVertices*3 * m_curFrame;	
	unsigned nv = numFrameVertices(m_curFrame); // was: m_numVertices

	//glInitNames(); // <- QGLViewer has already taken care of this (?)

	// Draw points in immediate mode, each with a different selection name	
	if( idx.empty() )
	{
		// Draw all points
		for( unsigned i=0; i < nv; i++ )
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
			glPushName( i );
			glBegin( GL_POINTS );
			glVertex3fv( &(m_vbuffer[ofs + 3*i]) );
			glEnd();
			glPopName();
		}
	}

	GL::CheckGLError("MeshBuffer::drawNamedPoints()");
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
	unsigned ofs = ofsVertex(m_curFrame); // was: m_numVertices*3 * m_curFrame;	
	unsigned nv = numFrameVertices(m_curFrame); // was: m_numVertices
	
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, &(m_vbuffer[ofs]) );

#if 0 // FIXME: Error in color buffer specification
	bool useCBuffer = m_cbufferEnabled && !m_cbuffer.empty();
	if( useCBuffer )
	{
		glEnableClientState( GL_COLOR_ARRAY );
		glColorPointer( 4, GL_FLOAT, 0, (void*)(sizeof(float)*m_numVertices*3+sizeof(float)*m_numNormals*3) );
	}
#endif

	if( !idx.empty() )
		// Draw only selected points
		glDrawElements( GL_POINTS, (GLsizei)idx.size(), GL_UNSIGNED_INT, &(idx[0]) );
	else
		// Draw all points
		glDrawArrays( GL_POINTS, 0, nv );

#if 0 // FIXME: Error in color buffer specification
	if( useCBuffer )
		glDisableClientState( GL_COLOR_ARRAY );
#endif

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

	// sanity check
	if( frame >= (int)m_numFrames || frame < 0 )
	{
		std::cout << "MeshBuffer::createMesh() : Requested invalid frame number "
			<< frame << ", falling back to first frame!" << std::endl;
		frame = 0;
	}

	unsigned nv = numFrameVertices(m_curFrame); // was: m_numVertices; 

	// Handles
	std::vector< Mesh::VertexHandle > vhandle( nv );
	std::vector< Mesh::VertexHandle > fvhandle( 3 );
		
	// Create new mesh
	Mesh* m = new Mesh;

	// Mesh vertex data
	unsigned vofs = ofsVertex(m_curFrame); // was: (unsigned)frame * m_numVertices * 3
	const float* pv = &(m_vbuffer[vofs]);
	for( unsigned i=0; i < nv; ++i )
	{
		Mesh::Point p;
		p[0] = *pv;  pv++;
		p[1] = *pv;  pv++;
		p[2] = *pv;  pv++;
		vhandle[i] = m->add_vertex( p );
		if( m_cbuffer.size() == 4*m_numVertices ) // was: && m_cbufferEnabled
			// Ignore alpha channel for export
			m->set_color( vhandle[i], OpenMesh::Vec3uc( 
				(unsigned char)(255.f*m_cbuffer[4*i]), 
				(unsigned char)(255.f*m_cbuffer[4*i+1]), 
				(unsigned char)(255.f*m_cbuffer[4*i+2])  ));
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

#if 1
	// FIXME: Avoid recomputation and copy existing normals
	meshtools::updateMeshVertexNormals( m );
#else
	if( numFrameNormals(m_curFrame) == numFrameVertices(m_curFrame) )
	{
		// Copy normals from normal buffer
		unsigned nofs = ofsNormal(m_curFrame);
		Mesh::VertexIter v_it(m->vertices_begin()), v_end(m->vertices_end());
		for( ; v_it!=v_end; ++v_it )
		{
			Mesh::Normal n( &(m_nbuffer[nofs]) );
			m->set_normal( *v_it, n );
		}
	}
	else
	{
		// Print warning
		std::cout << "MeshBuffer::createMesh() : " 
			"No normals available for frame " << frame << "!" << std::endl;
	}
#endif

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

//------------------------------------------------------------------------------
float MeshBuffer::computeBBoxDiagonal() const
{
	float minval(std::numeric_limits<float>::max()),
	      maxval(-minval);

	// Bounding box is given by min/max coordinate values
	float min_[3], max_[3];
	min_[0]=min_[1]=min_[2]=minval;
	max_[0]=max_[1]=max_[2]=maxval;

	// Get min/max coordinates over all vertices
	unsigned n = numVertices() * numFrames();
	const float* vptr = &m_vbuffer[0];
	for( unsigned int p=0; p < n; p++ )
	{
		for( unsigned int d=0; d < 3; d++ )
		{
			float val = (*vptr);
			min_[d] = (val < min_[d]) ? val : min_[d];
			max_[d] = (val > max_[d]) ? val : max_[d];
			vptr++;
		}
	}

	/// Return length of (max-min)
	return sqrt( (max_[0]-min_[0])*(max_[0]-min_[0]) +
	             (max_[1]-min_[1])*(max_[1]-min_[1]) +
	             (max_[2]-min_[2])*(max_[2]-min_[2]) );
}

//------------------------------------------------------------------------------
float MeshBuffer::normalizeSize()
{
	float scale = 1.f / computeBBoxDiagonal();
	//double scale = 1. / computeBBoxDiagonal();

	for( FloatBuffer::iterator it=vbuffer().begin(); it!=vbuffer().end(); ++it )
		(*it) *= scale;                           // float precision
	//	(*it) = (float)((double)(*it) * scale);   // double precision

	return scale;
}

//------------------------------------------------------------------------------

// FIXME: Code duplication!

unsigned MeshBuffer::numFrameVertices( int frame ) const
{
	if( frame>=0 && frame<m_vcount.size() )
	{
		return m_vcount[frame];
	}
	else
	{
		std::cerr << "MeshBuffer::numFrameVertices() : Requested invalid frame "
			<< frame << "!" << std::endl;
	}
	return 0;
}

unsigned MeshBuffer::numFrameNormals( int frame ) const
{
	if( frame>=0 && frame<m_ncount.size() )
	{
		return m_ncount[frame];
	}
	else
	{
		std::cerr << "MeshBuffer::numFrameNormals() : Requested invalid frame "
			<< frame << "!" << std::endl;
	}
	return 0;
}

unsigned MeshBuffer::ofsVertex( int frame ) const
{
	unsigned ofs=0;
	if( frame>=0 && frame<m_vcount.size() )
	{
		for( int i=0; i < frame; i++ )
			ofs += m_vcount[i];
	}
	else
	{
		std::cerr << "MeshBuffer::ofsVertex() : Requested invalid frame "
			<< frame << "!" << std::endl;
	}
	return 3*ofs; // 3 floats per vertex
}

unsigned MeshBuffer::ofsNormal( int frame ) const
{
	unsigned ofs=0;
	if( frame>=0 && frame<m_ncount.size() )
	{
		for( int i=0; i < frame; i++ )
			ofs += m_ncount[i];
	}
	else
	{
		std::cerr << "MeshBuffer::ofsNormal() : Requested invalid frame "
			<< frame << "!" << std::endl;
	}
	return 3*ofs;  // 3 floats per normal
}
