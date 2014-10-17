#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <glutils/GLSLProgram.h>
#ifdef GL_NAMESPACE
using GL::GLSLProgram;
#endif

/**
	\class ParticleSystem

	Simple GPU based state-preserving particle system.
	
	This class is responsible for particle advection and rendering.
	
	Advection is performed on the GPU using a ping-pong	technique updating 
	position and velocity buffers using Euler steps based on a given 
	acceleration field (i.e. force field for constant mass particles).

	Rendering is done in a second pass, generating a vertex for each particle.

	The implementation is uses only OpenGL 2.1 functionality.
*/
class ParticleSystem
{
public:
	ParticleSystem();

	void setup();
	void destroy() { destroyGL(); }

	void reseed();

	void update();
	void render();
	
	GLuint getPositions () { return m_texPos[m_curTargetBuf]; }
	GLuint getVelocities() { return m_texVel[m_curTargetBuf]; }
	GLuint getForces    () { return m_texForce; }

	void touch()
	{
		loadShadersFromDisk();
		reseed();
	}
	
protected:	
	void loadShadersFromDisk();
	void loadForceTexture( const char* filename );

	///@{ OpenGL setup
	bool initGL();
	void destroyGL();
	///@}

	void advectParticles();
	void swapParticleBuffers();

	///@{ Generate some test data
	void seedParticlePositions();
	void seedParticleVelocities();
	void setSyntheticForceField();
	///@}

private:
	bool m_initialized;
	
	GLSLProgram *m_advectShader,
	            *m_renderShader;

	// Width and height of FBO, total count is the max. number of particles.
	// Note that all FBO attachements have to have the same size.
	GLuint m_width,  
           m_height;

	GLuint m_texForce;  ///< Force / acceleration texture

	GLuint m_texPos[2], ///< Particle position double-buffer
	       m_texVel[2]; ///< Particle velocity double-buffer
	GLuint m_fbo;       ///< Frame buffer object (FBO) for advection
	GLuint m_vbo;       ///< Vertex buffer object (VBO) for rendering
	
	int m_curTargetBuf;	
};

#endif // PARTICLESYSTEM_H
