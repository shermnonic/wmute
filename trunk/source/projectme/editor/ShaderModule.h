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
	typedef ModuleRenderer Super;

	ShaderModule();

	///@name ModuleRenderer implementation
	///@{
	void render();
	int  target() const { return m_target.GetID(); }	
	void destroy();
	void touch() { compile(); }
	///@}

	///@name Shader management
	///@{
	/// Re-compile current shader
	bool compile();
	/// Compile new shader
	bool compile( std::string vshader, std::string fshader );
	/// Load fragment shader from disk, implicitly compiles the shader
	bool loadShader( const char* filename );
	bool saveShader( const char* filename ) const;
	///@}

	/// @name Serialization
	///@{
	PropertyTree& serialize() const;
	void deserialize( Serializable::PropertyTree& pt );
	///@}

	std::string getShaderSource() const { return m_fshader; }
	bool setShaderSource( const std::string& shader );

protected:
	// Invoked once in first render() call
	bool init();
	
private:
	bool            m_initialized;
	int             m_width, m_height;
	GLTexture       m_target;
	GLSLProgram*    m_shader;
	RenderToTexture m_r2t;
	std::string     m_vshader, m_fshader;
	std::string     m_lastCompileMessage;
};

#endif // SHADERMODULE_H
