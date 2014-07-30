#ifndef SHADERMODULE_H
#define SHADERMODULE_H

#include <glutils/GLTexture.h>
#include <glutils/GLSLProgram.h>
#include <glutils/RenderToTexture.h>

#ifdef GL_NAMESPACE
using GL::GLTexture;
using GL::GLSLProgram
#endif

class ShaderModule : public ModuleRenderer
{
public:
	void render();
	
	bool init();
	void destroy();
	
private:
	GLTexture    m_target;
	GLSLProgram* m_shader;
};

#endif // SHADERMODULE_H


#include "ShaderModule.h"
#include <iostream>

using std::cerr;
using std::endl;

using GL::checkGLError;

bool ShaderModule::init()
{
	int width = 512, height = 512;	
	
	// Create texture
	if( !m_target.Create(GL_TEXTURE_2D) )
	{
		cerr << "ShaderModule::init() : Couldn't create 2D textures!" << endl;
		return false;
	}
	
	// Note on texture format:
	// 8-bit per channel resolution leads to quantization artifacts so we
	// currently use 12-bit per channel which should be hardware supported.
	// On newer hardware it should be safe to set GL_RGBA32F.
	GLint internalFormat = GL_RGBA32F; //GL_RGB12;

	// Allocate GPU mem
	m_target.Image(0, internalFormat, width,height, 0, GL_RGBA, GL_FLOAT, NULL );
	
	// Setup Render-2-Texture
	if( !m_r2t.init( width,height, m_target.GetID(), true ) )
	{
		cerr << "ShaderModule::init() : Couldn't setup render-to-texture!" << endl;
		return false;
	}
	
	// Create shader
	if( m_shader ) delete m_shader; m_shader=0;
	m_shader = new GLSLProgram();
	if( !m_shader )
	{
		cerr << "ShaderModule::init() : Creation of GLSL shader failed!" << endl;
		return false;
	}	
	
	return checkGLError( "ShaderModule::init() : GL error at exit!" );
}

void ShaderModule::destroy()
{
	m_r2t.deinit();
	m_target.Destroy();
}

void ShaderModule::render()
{
	
}
