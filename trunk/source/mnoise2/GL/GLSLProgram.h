#ifndef GLSLPROGRAM_H
#define GLSLPROGRAM_H

#include "GLConfig.h"
#include <iostream>
#include <string>

#ifdef GL_NAMESPACE
namespace GL {
#endif

/// GLSL Program consisting of vertex- and fragment-shader.
class GLSLProgram
{
public:
	GLSLProgram();
	~GLSLProgram();

	static char* read_shader_from_disk( const char* filename );

	/// Load and compile GLSL shader programs
	bool load( const GLchar** vShaderSrc, const GLchar** fShaderSrc );

	///@{ Provided for convenience
	bool load( std::string vertSrc, std::string fragSrc );
	bool load_from_disk( const char* vs_filename, const char* fs_filename );
	///@}

	void bind()    const;
	void release() const;

	static void release_all();

    GLint getUniformLocation( const GLchar* name );

	void redirectLog( std::ostream& os ) { m_log = &os; }

protected:
	bool compileShader( GLuint shader  );
	bool linkProgram  ( GLuint program );
	
	std::string getShaderLog ( GLuint shader  );
	std::string getProgramLog( GLuint program );

	std::ostream* m_log;  ///< pointer to output stream used for log messages

private:
	GLuint m_program;  ///< GLSL program handle
	GLuint m_fShader;  ///< fragment shader handle
	GLuint m_vShader;  ///< vertex shader handle

	/// copy assignment not allowed
	GLSLProgram& operator= ( const GLSLProgram& other );
	/// copy constructor not allowed
	GLSLProgram( const GLSLProgram& other );
};

#ifdef GL_NAMESPACE
} // namespace GL 
#endif

#endif // GLSLPROGRAM_H
