#ifndef SHADERMODULE_H
#define SHADERMODULE_H

#include <glutils/GLTexture.h>
#include <glutils/GLSLProgram.h>
#include <glutils/RenderToTexture.h>

#include <string>

#ifdef GL_NAMESPACE
using GL::GLTexture;
using GL::GLSLProgram
#endif

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

	/// Re-compile shader
	bool compile();
	
private:
	int          m_width, m_height;
	GLTexture    m_target;
	GLSLProgram* m_shader;
	std::string  m_vshader, m_fshader;
};

#endif // SHADERMODULE_H
