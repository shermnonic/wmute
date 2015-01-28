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

#include "ModuleRenderer.h"
#include "Parameter.h"

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
	void applyOptions() { /* Call init again to change texture size */ init(); }
	///@}

	///@name ModuleRenderer channels implementation
	///@{
	void setChannel( int idx, int texId );
	int  channel( int idx ) const;
	int  numChannels() const;
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
	/// Access current shader source
	std::string getShaderSource() const { return m_fshader; }
	/// Replace current shader source
	bool setShaderSource( const std::string& shader );
	///@}

	/// @name Serialization
	///@{
	PropertyTree& serialize() const;
	void deserialize( Serializable::PropertyTree& pt );
	///@}

protected:
	/// Invoked in first render() call and on each size change of render texture.
	bool init();

	/// Preprocess shader: Replace some variables by uniforms and update parameters accordingly.
	std::string preprocess( std::string shader );
	
private:
	bool            m_initialized;
	// Some initialization has only to be done once
	bool            m_target_initialized;
	bool            m_r2t_initialized;
	bool            m_shader_initialized;

	GLTexture       m_target;
	GLSLProgram*    m_shader;
	RenderToTexture m_r2t;
	std::string     m_vshader, m_fshader;
	std::string     m_lastCompileMessage;
	std::vector<int> m_channels;

	ParameterList m_superParameters; // Inherited parameters
	ParameterList m_superOptions;

	/// Live shader parameters
	struct UniformParameters
	{
		std::vector<DoubleParameter> floats;
		std::vector<IntParameter>    ints;
		std::vector<BoolParameter>   bools;
		// Maybe later we'll add support for some more types ...
	};
	UniformParameters m_uniformParams;

	struct DefineOptions
	{
		std::vector<EnumParameter> enums;
	};
	DefineOptions m_defineOpts;

	/// Setup options
	struct Opts
	{
		IntParameter width, height;
		Opts() 
		: width("targetWidth"),
		  height("targetHeight")
		{
			width.setValueAndDefault( 512 );
			width.setLimits( 1, 2048 );
			height.setValueAndDefault( 512 );
			height.setLimits( 1, 2048 );
		}
	};
	Opts m_opts;
};

#endif // SHADERMODULE_H
