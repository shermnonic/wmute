#ifndef MESHSHADER_H
#define MESHSHADER_H

#include <string>
#include <glutils/GLSLProgram.h>

/// Per-pixel Phong shader equivalent to OpenGL fixed function pipeline.
/// Note that only the first OpenGL light is considered for shading yet.
class MeshShader
{
	static std::string s_vertexShader;
	static std::string s_fragmentShader;
	
public:
	MeshShader();

	// init() and destroy() require a valid OpenGL context.
	// The same is true for all other functions below.

	bool init();
	void destroy();

	void bind();
	void release();

	void setDefaultLighting();

private:
	GL::GLSLProgram* m_program;
};

#endif // MESHSHADER_H
