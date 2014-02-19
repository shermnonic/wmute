// MeshBuffer - simple OpenGL VBO wrapper with support for mesh animations.
// Max Hermann, Jan 2014
#ifndef MESHBUFFER_H
#define MESHBUFFER_H

#include <meshtools.h>
#include <vector>

/** @addtogroup meshspace meshspace
  * @{ */

/**
	\brief Simple wrapper around OpenGL VBO draw of triangle mesh (animations).
	
	Supports rendering mesh animations where all frames share the same 
	connectivity. It manages vertex, normal, index and color buffer.
	Additionally support for GL_SELECTION mode is provided via \a
	drawNamedPoints() e.g. for vertex selection.

	A custom binary format is implemented via \a read() and \a write().

	Vertex colors are not fully supported yet.
*/
class MeshBuffer
{
public:
	MeshBuffer()
	: m_curFrame(-1),
	  m_numFrames(0),
	  m_numVertices(0),
	  m_numNormals(0),
	  m_initialized(false),
	  m_dirty(true),
	  m_frameUpdateRequired(true),
	  m_cbufferEnabled(false)
	{}

	void clear();
	bool addFrame( const meshtools::Mesh* mesh );	

	///@{ Render functions
	void draw();

	void drawPoints( const std::vector<unsigned>& idx ) const;
	void drawPoints() const;

	void drawNamedPoints() const;
	void drawNamedPoints( const std::vector<unsigned>& idx ) const;
	///@}

	void setFrame( int f );

	int      curFrame() const { return m_curFrame; }
	unsigned numFrames() const { return m_numFrames; }
	unsigned numVertices() const { return m_numVertices; }

	///@{ File IO for custom .meshbuffer/.mb file format
	void write( const char* filename ) const;
	bool read( const char* filename );
	///@}

	/// Create a new OpenMesh mesh for specific frame
	meshtools::Mesh* createMesh( int frame=0 ) const;

	/// Return scalar product between given direction and vertex normal of vertex idx (No range checking!).
	double projectVertexNormal( unsigned idx, float x, float y, float z ) const;

	/// Setup color buffer (which is used for special attributes as well).
	/// Must be called after vbuffer is set, i.e. after a file is loaded, but
	/// before downloadGPU() called for the first time.
	void setupCBuffer();
	void setCBufferEnabled( bool b ) { m_cbufferEnabled = b; }

	///@{ Access color buffer raw data
	std::vector<float>& cbuffer() { return m_cbuffer; }
	const std::vector<float>& cbuffer() const { return m_cbuffer; }
	///@}

	///@{ Access vertex buffer raw data
	std::vector<float>& vbuffer() { return m_vbuffer; }
	const std::vector<float>& vbuffer() const { return m_vbuffer; }
	///@}

	/// Force download of buffers to GPU (automatically set in \a setFrame())
	void setFrameUpdateRequired() { m_frameUpdateRequired=true; }

protected:
	/// Called internally in \a draw() function
	void downloadGPU();

private:	
	int      m_curFrame;
	unsigned m_numFrames;
	unsigned m_numVertices;
	unsigned m_numNormals;
	bool     m_initialized;
	bool     m_dirty;
	bool     m_frameUpdateRequired;
	unsigned m_vbo;     ///< GL vertex buffer object id (should be a GLuint)
	unsigned m_ibo;     ///< GL index buffer object id (should be a GLuint)
	std::vector<unsigned> m_ibuffer; ///< index buffer (same for all meshes)
	std::vector<float>    m_vbuffer; ///< vertex buffer (consecutive frames)
	std::vector<float>    m_nbuffer; ///< vertex-normals buffer (consecutive frames)

	bool m_cbufferEnabled;
	std::vector<float>    m_cbuffer; ///< color buffer (same for all meshes?!)
};

/** @} */ // end group

#endif // MESHBUFFER_H
