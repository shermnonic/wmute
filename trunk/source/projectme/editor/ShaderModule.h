#ifndef SHADERMODULE_H
#define SHADERMODULE_H

#include <glutils/GLTexture.h>
#include <glutils/GLSLProgram.h>
#include <glutils/RenderToTexture.h>

#include <string>

#ifdef GL_NAMESPACE
using GL::GLTexture;
using GL::GLSLProgram;
#endif

#include "RenderSet.h" // for ModuleRenderer

/**
	\class ShaderModule

	Simple fragment shader module in the spirit of shadertoy.
*/
class ShaderModule : public ModuleRenderer
{
public:
	ShaderModule();

	/// Render shader scene to texture
	void render();
	
	bool init();
	void destroy();

	/// Target texture id
	int target() const { return m_target.GetID(); }

	/// Re-compile current shader
	bool compile();
	/// Compile new shader
	bool compile( std::string vshader, std::string fshader );

	bool loadShader( const char* filename );
	
private:
	bool            m_initialized;
	int             m_width, m_height;
	GLTexture       m_target;
	GLSLProgram*    m_shader;
	RenderToTexture m_r2t;
	std::string     m_vshader, m_fshader;
};

#endif // SHADERMODULE_H
