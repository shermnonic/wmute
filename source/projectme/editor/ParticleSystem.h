#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <glutils/GLSLProgram.h>
#ifdef GL_NAMESPACE
using GL::GLSLProgram;
#endif

/**
	\class ParticleSystem

	Simple GPU based state-preserving particle system.
*/
class ParticleSystem
{
public:
	ParticleSystem();

	void setup();
	void destroy() { destroyGL(); }

	void update()
	{
		advectParticles();
		swapParticleBuffers();
	}
	
	GLuint getPositions() { return m_texPos[0]; }

	void touch()
	{
		reloadShaderFromDiskHack();
	}
	
protected:	
	///@name Shader management
	///@{
	void reloadShaderFromDiskHack();
	/// Re-compile current shader
	bool compile();
	/// Compile new shader
	bool compile( std::string vshader, std::string fshader );
	///@}	

	///@{ OpenGL setup
	bool initGL();
	void destroyGL();
	///@}

	void advectParticles();
	void swapParticleBuffers();

	///@{ Generate some test data
	void seedParticlePositions();
	void seedParticleVelocities();
	///@}

private:
	bool m_initialized;
	
	GLSLProgram* m_shader;
	std::string  m_vshader, // Store source of vertex/fragment shader locally
	             m_fshader;

	GLuint m_width,  // All FBO attachements have to have the same size
           m_height;
	GLuint m_texPos[2], ///< Particle position double-buffer
	       m_texVel[2]; ///< Particle velocity double-buffer
	GLuint m_fbo;       ///< Frame buffer object (FBO)
	int m_curTargetBuf;	
};

#endif // PARTICLESYSTEM_H
