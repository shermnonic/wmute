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

// internal helper function
char* GLSLProgram::read_shader_from_disk( const char* filename )
{
	ifstream is( filename );
	if( is.good() )
	{	
		is.seekg(0,ios::end);
		int len = is.tellg();
		is.seekg(0,ios::beg);

		char* buf = new char[len+1];
		memset(buf,0,len+1);

		is.read(buf,len);
		is.close();

		//cout <<"[SHADER BEGIN]"<< endl << buf << endl <<"[SHADER END]"<< endl;
		return buf;
	}

	return NULL;
}

bool GLSLProgram::load_from_disk( const char* vs_filename, const char* fs_filename )
{
	// load from disk
	char* vertex_shader   = read_shader_from_disk( vs_filename );
	char* fragment_shader = read_shader_from_disk( fs_filename );
	if( !vertex_shader || !fragment_shader )
	{
		cerr << "Error: Failed to load GLSL shader files '"
			 << vs_filename << "' and '"
			 << fs_filename << "'!" << endl;
		if( vertex_shader   ) delete [] vertex_shader;
		if( fragment_shader ) delete [] fragment_shader;
		return false;
	}

	// compile
	bool success = load( vertex_shader, fragment_shader );

	delete [] vertex_shader;
	delete [] fragment_shader;
	return success;
}

bool GLSLProgram::load( std::string vertSrc, std::string fragSrc )
{
	GLint vsize = vertSrc.size();
	GLint fsize = fragSrc.size();	

	// a bit hacky string to const char** conversion - but it works ;-)
	char* vsdata = new char[vsize+1];
	char* fsdata = new char[fsize+1];

	memset( vsdata, 0, vsize+1 );
	memset( fsdata, 0, fsize+1 );

	memcpy( vsdata, vertSrc.data(), vsize ); // .c_str() vs .data() ?
	memcpy( fsdata, fragSrc.data(), fsize );

	//cout << "Before glShaderSource" << endl;
	glShaderSource( m_vShader, 1, (const char**)&vsdata, &vsize );
	glShaderSource( m_fShader, 1, (const char**)&fsdata, &fsize );
	//cout << "After glShaderSource" << endl;
	
	bool ret;
	if( compileShader( m_fShader ) && 
		compileShader( m_vShader ) &&
		linkProgram( m_program )      
	  )
		ret = true;
	else
		ret = false;
	
	delete [] vsdata;
	delete [] fsdata;
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

void GLSLProgram::release_all()
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
		*m_log << getShaderLog( shader ) << endl;
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
		*m_log << getProgramLog( program ) << endl;
		return false;
	}
	
	return true;
}
