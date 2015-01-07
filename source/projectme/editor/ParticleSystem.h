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
	GLuint getPositions2() { return m_texPos[(m_curTargetBuf+1)%2]; }
	GLuint getVelocities() { return m_texVel[m_curTargetBuf]; }
	GLuint getForces    () { return m_curTexForce; } // was: m_texForce
	GLuint getBirthPositions() { return m_texBirthPos; }

	void touch()
	{
		loadShadersFromDisk();
		reseed();
	}

	// Call with -1 to reset to internal force texture
	void setForceTexture( int texid )
	{
		if( texid >= 0 )
			m_curTexForce = texid;
		else
			m_curTexForce = m_texForce;
	}
	int getForceTexture() const
	{
		return m_curTexForce;
	}

	void setSpriteTexture( int texid )
	{
		m_texSprite = texid;
	}
	int getSpriteTexture() const
	{
		return m_texSprite;
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

	void killAllParticles();

private:
	bool m_initialized;
	
	GLSLProgram *m_advectShader,
	            *m_renderShader;

	// Width and height of FBO, total count is the max. number of particles.
	// Note that all FBO attachements have to have the same size.
	GLuint m_width,  
           m_height;

	GLuint m_texForce,    ///< Force / acceleration texture
	       m_curTexForce; ///< Currently active force texture (maybe set from outside)
	int	   m_texSprite;
	GLuint m_texBirthPos, ///< Particle re-incarnation positions
	       m_texBirthVel; ///< Particle re-incarnation velocities
	GLuint m_texPos[2],   ///< Particle position double-buffer
	       m_texVel[2];   ///< Particle velocity double-buffer
	GLuint m_fbo;         ///< Frame buffer object (FBO) for advection
	GLuint m_vbo;         ///< Vertex buffer object (VBO) for rendering
	
	int m_curTargetBuf;	
};

#endif // PARTICLESYSTEM_H
