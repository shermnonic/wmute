#ifndef MESHSHADER_H
#define MESHSHADER_H

#include <string>
#include <glutils/GLSLProgram.h>

#include "TransferFunction.h"

/// Per-pixel Phong shader equivalent to OpenGL fixed function pipeline.
/// Note that only the first OpenGL light is considered for shading yet.
class MeshShader
{
public:
	MeshShader();

	bool isInitialized() const { return m_initialized; }

	// init() and destroy() require a valid OpenGL context.
	// The same is true for all other functions below.

	bool init();
	void destroy();

	void bind();
	void release();

	void setDefaultLighting();

	GL::GLSLProgram* program() { return m_program; }

private:
	GL::GLSLProgram* m_program;
	TransferFunction m_tf;
	bool m_initialized;
};

#endif // MESHSHADER_H
