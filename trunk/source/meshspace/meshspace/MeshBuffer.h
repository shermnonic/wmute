// MeshBuffer - simple OpenGL VBO wrapper with support for mesh animations.
// Max Hermann, Jan 2014
#ifndef MESHBUFFER_H
#define MESHBUFFER_H

#include <meshtools.h>
#include <vector>

/**
	Simple wrapper around OpenGL VBO draw of triangle mesh (animations).
	
	Supports rendering mesh animations where all frames share the same 
	connectivity.
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
	  m_frameUpdateRequired(true)
	{}

	void clear();
	bool addFrame( const meshtools::Mesh* mesh );
	void downloadGPU();
	void draw();

	void drawPoints( const std::vector<unsigned>& idx ) const;
	void drawPoints() const;

	void drawNamedPoints() const;
	void drawNamedPoints( const std::vector<unsigned>& idx ) const;

	void setFrame( int f );

	int      curFrame() const { return m_curFrame; }
	unsigned numFrames() const { return m_numFrames; }

	void write( const char* filename ) const;
	bool read( const char* filename );

	meshtools::Mesh* createMesh( int frame=0 ) const;

	// Return scalar product between given direction and vertex normal of vertex idx (No range checking!).
	double projectVertexNormal( unsigned idx, float x, float y, float z ) const;

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
	std::vector<float>    m_vbuffer;
	std::vector<float>    m_nbuffer;
};

#endif // MESHBUFFER_H
