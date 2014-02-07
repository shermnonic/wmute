class MeshObject : public Object
{
public:
	MeshObject()
		: m_initialized(false),
		  m_dirty(false),
		  m_vbo(0),
		  m_numIndices(0)
	{}

	void render();
	BoundingBox getBoundingBox() const;

	void setMesh( boost::shared_ptr<meshtools::Mesh> mesh );
	// Provided for convenience
	void setMesh( meshtools::Mesh* mesh ) 
	{ 
		setMesh( boost::shared_ptr<meshtools::Mesh>(mesh) );
	}
private:
	boost::shared_ptr<meshtools::Mesh> m_mesh;
	std::vector<float> m_vbuffer; ///< vertex buffer
	std::vector<float> m_nbuffer; ///< normal buffer
	bool m_initialized; ///< GL specific stuff initialized ?
	bool m_dirty;       ///< GL buffer update required ?
	unsigned m_vbo;     ///< GL vertex buffer object id (should be a GLuint)
	int m_numIndices;   ///< Number of vertices in GL buffer
};




//-----------------------------------------------------------------------------
void MeshObject::setMesh( boost::shared_ptr<meshtools::Mesh> mesh )
{
	using meshtools::Mesh;

	m_mesh = mesh;

	// Compute vertex normals
	meshtools::updateMeshVertexNormals( m_mesh.get() );

	// Fill vertex and normal buffer
	m_vbuffer.clear();
	m_nbuffer.clear();
	
	// Allocate memory
	m_vbuffer.reserve(mesh->n_faces()*3*3); // triangles
	if( mesh->has_vertex_normals() )
		m_nbuffer.reserve(mesh->n_faces()*3*3); // vertex normals
	
	// Fill buffers
	for( Mesh::FaceIter f_it=mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it ) 
	{
		for( Mesh::FaceVertexIter fv_it=mesh->fv_iter(f_it); fv_it; ++fv_it ) 
		{
			if( mesh->has_vertex_normals() )
			{	
				const Mesh::Normal & n = mesh->normal(fv_it);
				m_nbuffer.push_back( n[0] );
				m_nbuffer.push_back( n[1] );
				m_nbuffer.push_back( n[2] );
			}
			const Mesh::Point & p = mesh->point(fv_it);
			m_vbuffer.push_back( p[0] );
			m_vbuffer.push_back( p[1] );
			m_vbuffer.push_back( p[2] );			
		}
	}

	// Set dirty flag to indicate that buffer download to GPU is required
	m_dirty = true;
}

//-----------------------------------------------------------------------------
void MeshObject::render()
{
	using meshtools::Mesh;	
	Mesh* mesh = m_mesh.get();	
	if( !mesh ) return;
	
	// use global scene object color
	Color c = getColor();
	glColor4f( c.r, c.g, c.b, c.a );

#ifdef SCENE_USE_IMMEDIATE_MODE	
	// bad, bad immediate mode rendering ;-)
	glBegin( GL_TRIANGLES );
  #if 0 // IMMEDIATRE DRAW DIRECTLY FROM MESH
	for( Mesh::FaceIter f_it=mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it ) 
	{
		for( Mesh::FaceVertexIter fv_it=mesh->fv_iter(f_it); fv_it; ++fv_it ) 
		{
			if( mesh->has_vertex_normals() )
			{	
				const Mesh::Normal & n = mesh->normal(fv_it);
				glNormal3f(n[0], n[1], n[2]);
			}
			const Mesh::Point & p = mesh->point(fv_it);
			glVertex3f(p[0], p[1], p[2]);
		}
	}
  #else // IMMEDIATRE DRAW FROM BUFFERS
	bool hasVertexNormals = m_vbuffer.size() == m_nbuffer.size();
	for( unsigned i=0; i < m_vbuffer.size()/3; i++ )
	{	
		if( hasVertexNormals )
			glNormal3f( m_nbuffer[3*i], m_nbuffer[3*i+1], m_nbuffer[3*i+2] );
		glVertex3f( m_vbuffer[3*i], m_vbuffer[3*i+1], m_vbuffer[3*i+2] );
	}
  #endif
	glEnd();	

#else // USE BUFFER DRAW

  #if 0  // VERTEX ARRAY FALLBACK

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );

	glVertexPointer( 3, GL_FLOAT, 0, &(m_vbuffer[0]) );
	glNormalPointer( GL_FLOAT, 0, &(m_nbuffer[0]) );
	
	glDrawArrays( GL_TRIANGLES, 0, m_vbuffer.size()/3 );			

	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_VERTEX_ARRAY );

  #else  // GPU BUFFER SOLUTION
	if( !m_initialized )
	{
		// Create OpenGL buffers
		glGenBuffers( 1, &m_vbo );
		m_initialized = true;

		// FIXME: Add error checking and also delete buffers in the end!
	}

	if( m_dirty )
	{
		m_numIndices = m_vbuffer.size() / 3;

		// Allocate buffer
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
		glBufferData( GL_ARRAY_BUFFER, 
			sizeof(float)*( m_vbuffer.size() + m_nbuffer.size() ), NULL, 
			GL_STATIC_DRAW );

		// Download vertices
		glBufferSubData( GL_ARRAY_BUFFER, 0,  // offset
			sizeof(float)*m_vbuffer.size(),	&(m_vbuffer[0]) );

		// Download normals
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(float)*m_vbuffer.size(),
			sizeof(float)*m_nbuffer.size(), &(m_nbuffer[0]) );

		// Data can be safely released afterwards
		m_vbuffer.clear(); 
		m_nbuffer.clear();
		
		m_dirty = false;
	}

	// Draw buffer
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_VERTEX_ARRAY );
	glNormalPointer( GL_FLOAT, 0, (void*)(sizeof(float)*m_numIndices*3) );
	glVertexPointer( 3, GL_FLOAT, 0, 0 );
	glDrawArrays( GL_TRIANGLES, 0, m_numIndices );
	glDisableClientState( GL_VERTEX_ARRAY );	
	glDisableClientState( GL_NORMAL_ARRAY );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
  #endif

#endif
}
