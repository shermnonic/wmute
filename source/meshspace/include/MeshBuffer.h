// MeshBuffer - simple OpenGL VBO wrapper with support for mesh animations.
// Max Hermann, Jan 2014
#ifndef MESHBUFFER_H
#define MESHBUFFER_H

#include <meshtools.h>
#include <vector>

/** @addtogroup meshtools
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
	/** @name Setup */
	///@{
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
	///@}
	
	/** @name Render functions */
	///@{ 
	void draw();
	void drawPoints( const std::vector<unsigned>& idx ) const;
	void drawPoints() const;
	void drawNamedPoints() const;
	void drawNamedPoints( const std::vector<unsigned>& idx ) const;
	///@}

	/** @name Other functions */
	///@{
	/// Change current frame
	void setFrame( int f );
	/// Force download of buffers to GPU (automatically set in \a setFrame())
	void setFrameUpdateRequired() { m_frameUpdateRequired=true; }

	/// Create a new OpenMesh mesh for specific frame
	meshtools::Mesh* createMesh( int frame=0 ) const;

	/// Return scalar product between given direction and vertex normal of vertex idx (No range checking!).
	double projectVertexNormal( unsigned idx, float x, float y, float z ) const;

	/// Scale meshes to bounding box diagonal 1, returns scale factor.
	/// The scale factor is computed internally via \a computeBBoxDiagonal().
	float normalizeSize();
	///@}

	/** @name Properties */
	///@{
	int      curFrame() const { return m_curFrame; }
	unsigned numFrames() const { return m_numFrames; }
	unsigned numVertices() const { return m_numVertices; }
	///@}

	/** @name File IO 
	 *  File IO for custom .meshbuffer/.mb file format
	 */
	///@{ 
	void write( const char* filename ) const;
	bool read( const char* filename );
	///@}

	/** @name Buffer management */
	//@{
	void initSingleFrameFromRawBuffers();

	/// Setup color buffer (which is used for special attributes as well).
	/// Must be called after vbuffer is set, i.e. after a file is loaded, but
	/// before downloadGPU() called for the first time.
	void setupCBuffer();
	void setCBufferEnabled( bool b ) { m_cbufferEnabled = b; }
	///@}

	/** @name Raw buffer access
	 *  Access to raw buffers (you better know what you are doing, 
	 *  see also \a initSingleFrameFromRawBuffers())
	 */
	///@{ 
	typedef std::vector<float>    FloatBuffer;
	typedef std::vector<unsigned> IndexBuffer;
	FloatBuffer& cbuffer() { return m_cbuffer; }
	FloatBuffer& vbuffer() { return m_vbuffer; }
	FloatBuffer& nbuffer() { return m_nbuffer; }
	IndexBuffer& ibuffer() { return m_ibuffer; }
	const FloatBuffer& cbuffer() const { return m_cbuffer; }
	const FloatBuffer& vbuffer() const { return m_vbuffer; }
	const FloatBuffer& nbuffer() const { return m_nbuffer; }
	const IndexBuffer& ibuffer() const { return m_ibuffer; }
	///@}

protected:
	/// Makes sure buffers are downloaded to GPU.
	/// Should be called first in every render/draw function.
	void sanity(); 

	/// Called internally in \a draw() function
	void downloadGPU();

	/// Return bounding box diagonal of all points over all frames.
	float computeBBoxDiagonal() const;

private:	
	int      m_curFrame;
	unsigned m_numFrames;
	unsigned m_numVertices;
	unsigned m_numNormals;
	bool     m_initialized; ///< True if GL buffer objects are created
	bool     m_dirty;       ///< True if GL buffers require resize / reallocation
	bool     m_frameUpdateRequired;
	unsigned m_vbo;     ///< GL vertex buffer object id (should be a GLuint)
	unsigned m_ibo;     ///< GL index buffer object id (should be a GLuint)
	std::vector<unsigned> m_ibuffer; ///< index buffer (same for all meshes)
	std::vector<float>    m_vbuffer; ///< vertex buffer (consecutive frames)
	std::vector<float>    m_nbuffer; ///< vertex-normals buffer (consecutive frames)

	bool m_cbufferEnabled;
	std::vector<float>    m_cbuffer; ///< color buffer (same for all meshes?!)
};

/** @} end group */

#endif // MESHBUFFER_H
