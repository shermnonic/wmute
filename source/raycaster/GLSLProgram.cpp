#include "GLSLProgram.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cassert>

using namespace std;
using namespace GL;

//-----------------------------------------------------------------------------
// Convenience functions
//-----------------------------------------------------------------------------

/// Read a textfile from disk, STL style.
std::string read_shader_from_disk_as_string( const char* filename )
{
	ifstream f( filename );
	if( !f )
		return string();

	stringstream src;
	src << f.rdbuf();
	f.close();

	return src.str();
}

bool GLSLProgram::load_from_disk( const char* vs_filename, const char* fs_filename )
{
	string vertex_shader   = read_shader_from_disk_as_string( vs_filename );
	string fragment_shader = read_shader_from_disk_as_string( fs_filename );
	if( vertex_shader.empty() || fragment_shader.empty() )
	{
		cerr << "Error: Failed to load Raycast GLSL shader files!" << endl;
		return false;
	}

#if 1 // was: #ifdef _DEBUG
	{
		ofstream f("tmp_shader.vs.glsl");
		f << vertex_shader;
		f.close();
	}
	{
		ofstream f("tmp_shader.fs.glsl");
		f << fragment_shader;
		f.close();
	}
#endif

	// compile
	bool success = load( vertex_shader, fragment_shader );

	return success;
}

bool GLSLProgram::load( std::string vertSrc, std::string fragSrc )
{
	GLint vsize = (GLint)vertSrc.size();
	GLint fsize = (GLint)fragSrc.size();

	const char* vsdata = vertSrc.c_str();
	const char* fsdata = fragSrc.c_str();

	glShaderSource( m_vShader, 1, (const char**)&vsdata, &vsize );
	glShaderSource( m_fShader, 1, (const char**)&fsdata, &fsize );
	
	bool ret;
	if( compileShader( m_fShader ) && 
		compileShader( m_vShader ) &&
		linkProgram( m_program )      
	  )
		ret = true;
	else
		ret = false;
	
	return ret;
}

//-----------------------------------------------------------------------------
// GLSLProgram
//-----------------------------------------------------------------------------

GLSLProgram::GLSLProgram()
: m_log(&cout)  // default output stream for log messages
{
	m_program = glCreateProgram();
	m_fShader = glCreateShader( GL_FRAGMENT_SHADER );	
	m_vShader = glCreateShader( GL_VERTEX_SHADER ); 
	
	// it is absolute valid to attach a shader object to a program object before
	// source code has been loaded into the shader object
	glAttachShader( m_program, m_fShader );
	glAttachShader( m_program, m_vShader );

}

GLSLProgram::~GLSLProgram()
{
	// shader must be detached before deletion
	glDetachShader( m_program, m_vShader );
	glDetachShader( m_program, m_fShader );
	
	glDeleteShader( m_vShader );
	glDeleteShader( m_fShader );	
	glDeleteProgram( m_program );
}

bool GLSLProgram::load( const GLchar** vShaderSrc, const GLchar** fShaderSrc )
{
	assert( vShaderSrc );
	assert( fShaderSrc );
	
	glShaderSource( m_vShader, 1, vShaderSrc, 0 );
	glShaderSource( m_fShader, 1, fShaderSrc, 0 );
	
	if( compileShader( m_fShader ) && 
		compileShader( m_vShader ) &&
		linkProgram( m_program )      
	  )
		return true;
	
	return false;
}

void GLSLProgram::bind() const
{
	glUseProgram( m_program );	
}

void GLSLProgram::release() const
{
	glUseProgram( 0 );
}

GLint GLSLProgram::getUniformLocation( const GLchar* name )
{	
	return glGetUniformLocation( m_program, name );
}

string GLSLProgram::getShaderLog( GLuint shader )
{
	int len;          // length of info log
	int lenWritten;   // chars written
	stringstream ss;  // stringstream used for copying char buffer to a string
	
	glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &len );
	
	if( len > 0 )
	{
		char* log = new char[ len ];
		
		glGetShaderInfoLog( shader, len, &lenWritten, log );
		
		ss << log;
		
		delete [] log;
	}
	
	return ss.str();
}

string GLSLProgram::getProgramLog( GLuint program )
{
	int len;          // length of info log
	int lenWritten;   // chars written
	stringstream ss;  // stringstream used for copying char buffer to a string
	
	glGetProgramiv( program, GL_INFO_LOG_LENGTH, &len );
	
	if( len > 0 )
	{
		char* log = new char[ len ];
		
		glGetProgramInfoLog( program, len, &lenWritten, log );
		
		ss << log;
		
		delete [] log;
	}

	return ss.str();
}

bool GLSLProgram::compileShader( GLuint shader )
{
	int compileSuccess;
	
	glCompileShader( shader );	
	glGetShaderiv( shader, GL_COMPILE_STATUS, &compileSuccess );
	
	if( !compileSuccess )
	{
		*m_log << "Shader " << shader << " log:" << endl 
			   << getShaderLog( shader ) << endl;
		return false;
	}
	
	return true;
}

bool GLSLProgram::linkProgram( GLuint program )
{
	int linkSuccess;
	
	glLinkProgram( program );
	glGetProgramiv( program, GL_LINK_STATUS, &linkSuccess );
	if( !linkSuccess )
	{
		*m_log << "Program " << program << " log:" << endl 
			   << getProgramLog( program ) << endl;
		return false;
	}
	
	return true;
}
