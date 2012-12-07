// gl3test.cpp

#include <GL/glew.h>

// freeglut GL3 
//#define GL3_PROTOTYPES
//#include "GL3/gl3.h"
//#define __gl_h_
#include <GL/freeglut.h>

#include <glm/glm.hpp> // glm::vec3, glm::vec4, glm::ivec4, glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::ortho
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <iostream>
#include <string>
#include <exception>

const GLvoid *gl_bufptr( GLsizei index );
bool gl_compile( GLuint shader, GLenum type, GLsizei count, const GLchar** string );
bool gl_link( GLuint program );
void gl_checkError( const char* funcname );
void gl_info( std::ostream& os=std::cout );


//#define ATTRIB_HACK
// see http://www.opengl.org/wiki/GlVertexAttribPointer


//------------------------------------------------------------------------------
//  Class definitions
//------------------------------------------------------------------------------

class InitException : public std::exception
{
	std::string m_desc;
public:
	InitException( std::string desc ): m_desc(desc) {}
	const char* what() const throw() { return m_desc.c_str(); }
};

class GL3App
{
public:
	virtual void init() {} // once called at startup
	virtual void destroy() {};
	virtual void reshape( int w, int h ) { glViewport(0,0,w,h); }
	virtual void render() { glClearColor(0,0,1,1); glClear(GL_COLOR_BUFFER_BIT); swapBuffers(); }
	virtual void idle() {}
protected:
	void swapBuffers() { glutSwapBuffers(); }
	void update() { glutPostRedisplay(); }
	unsigned int elapsedTime() { return glutGet(GLUT_ELAPSED_TIME); }
};

class GL3Test : public GL3App
{
public:
	void init();
	void destroy();
	void reshape( int w, int h );
	void render();
	void idle();
protected:
	bool initShader();
	void initBuffer();
private:
	std::string m_vshader, m_fshader;
	GLuint m_program, m_vs, m_fs;
	GLuint mx_projectionMatrix, mx_viewMatrix, mx_color, mx_vertex;
	GLuint m_vbo[2];
	GLuint m_vao;

	glm::mat4 m_projmat;
	glm::mat4 m_viewmat;

	float m_rotation;
};

GL3Test g_gl3test;
GL3App* g_globalApp = &g_gl3test;


//------------------------------------------------------------------------------
//  GLSL shaders
//------------------------------------------------------------------------------

//#define GLSL_SHADER(S) #S
//const GLchar GL3Test_vshader[] = GLSL_SHADER(
//#version 140 \n
//uniform mat4 mx_projectionMatrix; \n
//in vec3 mx_color; \n
//in vec3 mx_vertex; \n
//smooth out vec3 mx_smoothColor; \n
//void main() \n
//{ \n
//	mx_smoothColor = mx_color; \n
//	gl_Position = mx_projectionMatrix * vec4(mx_vertex,1.0); \n
//}\n\0
//);


const GLchar* GL3Test_vshader[] = {
"#version 140\n",
"uniform mat4 mx_projectionMatrix;\n",
"uniform mat4 mx_viewMatrix;\n",
"in vec3 mx_color;\n",
"in vec3 mx_vertex;\n",
"smooth out vec3 mx_smoothColor;\n",
"void main()\n",
"{\n",
"	mx_smoothColor = mx_color;\n",
"	gl_Position = mx_projectionMatrix * mx_viewMatrix * vec4(mx_vertex,1.0);\n",
"}\n",
};


const GLchar* GL3Test_fshader[] = {
"#version 140\n",
"smooth in vec3 mx_smoothColor;\n",
"out vec4 mx_fragColor;\n",
"void main()\n",
"{\n",
"	mx_fragColor = vec4(mx_smoothColor,1.0);\n",
"}\n",
};

const GLfloat GL3Test_vertices[] = {
	-.5, -.5, 0,
	-.5,  .5, 0,
	 .5,  .5, 0,
	 .5, -.5, 0 
};

const GLfloat GL3Test_colors[] = {
	  1,0,0,
	  0,1,0,
	  0,0,1,
	  1,1,1,
};


//------------------------------------------------------------------------------
//  App implementation
//------------------------------------------------------------------------------

void GL3Test::init()
{
	m_program = glCreateProgram();
	m_vs = glCreateShader( GL_VERTEX_SHADER );
	m_fs = glCreateShader( GL_FRAGMENT_SHADER );
	
	if( !initShader() ) // binds program
		throw InitException( "Error on GL3Test::initShader!" );
	
	initBuffer(); // binds buffer

	gl_checkError("GL3Test::init - glVertexAttribPointer");
	
	glClearColor( .1f,.1f,.2f, 1 );

	m_rotation = 0.f;
}

void GL3Test::destroy()
{
	glDeleteProgram( m_program );
	glDeleteShader( m_vs );
	glDeleteShader( m_fs );
	// TODO: destroy remaining GL stuff (vbo, vao, ...)
}

void GL3Test::reshape( int w, int h )
{
	GL3App::reshape(w,h);

	float asp = w/(float)h;
#if 0
	if( asp > 1.0 )
		m_projmat = glm::ortho(-asp,asp,-1.f,1.f);
	else
		m_projmat = glm::ortho(-1.f,1.f,-1.f/asp,1.f/asp);
#else
	m_projmat = glm::perspective( 42.f, asp, 0.1f, 42.f );
#endif
}

void GL3Test::idle()
{	
	static unsigned int lastTime = elapsedTime();
	unsigned int curTime = elapsedTime();
	unsigned int deltaTime = curTime - lastTime;
	if( deltaTime > 13 )
	{
		lastTime = curTime;
		m_rotation += ((float)deltaTime/15000.f)*360;
		update();
	}
}

void GL3Test::render()
{
	gl_checkError("GL3Test::render - enter");
	glClear( GL_COLOR_BUFFER_BIT );

	gl_checkError("GL3Test::render - init");

	glUseProgram( m_program );

	m_viewmat = glm::mat4(1.f);
	m_viewmat = glm::translate( m_viewmat, glm::vec3(0,0,-2.5f) );
	m_viewmat = glm::rotate( m_viewmat, m_rotation, glm::vec3(0.f,0.f,1.f) );
	glUniformMatrix4fv( mx_projectionMatrix, 1, GL_FALSE, &m_projmat[0][0] );
	glUniformMatrix4fv( mx_viewMatrix, 1, GL_FALSE, &m_viewmat[0][0] );
	gl_checkError("GL3Test::render - glUniformMatrix4fv");

	glBindVertexArray( m_vao );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	glBindVertexArray( 0 );
	gl_checkError("GL3Test::render - glDrawArrays");

	glUseProgram( 0 );
	
	glFlush();
	swapBuffers();
	gl_checkError("GL3Test::render - finalize");
}

bool GL3Test::initShader()
{
	using namespace std;

	static GLuint program = m_program, vs = m_vs, fs = m_fs;

	//if( !gl_compile( vs, GL_VERTEX_SHADER, 1,(const GLchar**)&GL3Test_vshader ) )
	const GLsizei vs_nlines = sizeof(GL3Test_vshader) / sizeof(GLchar*);
	if( !gl_compile( vs, GL_VERTEX_SHADER, vs_nlines, GL3Test_vshader ) )
	{
		cerr << "Failed to compile vertex shader!" << endl;
		return false;
	}
	//if( !gl_compile( fs, GL_FRAGMENT_SHADER, 1, (const GLchar**)&GL3Test_fshader ) )
	const GLsizei fs_nlines = sizeof(GL3Test_fshader) / sizeof(GLchar*);
	if( !gl_compile( fs, GL_FRAGMENT_SHADER, fs_nlines, GL3Test_fshader ) )
	{
		cerr << "Failed to compile fragment shader!" << endl;
		return false;
	}

	glAttachShader( program, vs );
	glAttachShader( program, fs );

#ifdef ATTRIB_HACK
	// explicitly specify attrib locations yourself
	// must be done *before* linking the shader
	mx_color  = 0;
	mx_vertex = 1;
	glBindAttribLocation( m_program, mx_vertex, "mx_vertex" );
	glBindAttribLocation( m_program, mx_color, "mx_color" );
#endif

	if( !gl_link( program ) )
	{
		cerr << "Failed to link GLSL program!" << endl;
		return false;
	}

	// custom setup

	glUseProgram( program );

	mx_projectionMatrix = glGetUniformLocation( program, "mx_projectionMatrix" );
	mx_viewMatrix       = glGetUniformLocation( program, "mx_viewMatrix" );
#ifndef ATTRIB_HACK
	mx_color  = glGetAttribLocation( program, "mx_color" );
	mx_vertex = glGetAttribLocation( program, "mx_vertex" );	
#endif

	gl_checkError("GL3Test::initShader");
	return true;
}

void GL3Test::initBuffer()
{
	glGenVertexArrays( 1, &m_vao );
	glBindVertexArray( m_vao );

	glGenBuffers( 2, m_vbo );

	// vertices
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo[0] );
	glBufferData( GL_ARRAY_BUFFER, sizeof(GL3Test_vertices), GL3Test_vertices, GL_STATIC_DRAW );
	glVertexAttribPointer( (GLuint)mx_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	glEnableVertexAttribArray( mx_vertex );

	// color
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo[1] );
	glBufferData( GL_ARRAY_BUFFER, sizeof(GL3Test_colors), GL3Test_colors, GL_STATIC_DRAW );	
	glVertexAttribPointer( (GLuint)mx_color, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	glEnableVertexAttribArray( mx_color );

	glBindVertexArray( 0 ); // disable vbo

	gl_checkError("GL3Test::initBuffer");
}

//------------------------------------------------------------------------------
//  GL helpers
//------------------------------------------------------------------------------

const GLvoid *gl_bufptr( GLsizei index )
{
   return (const GLvoid *) (((char *) NULL) + index);
}

bool gl_compile( GLuint shader, GLenum type, GLsizei count, const GLchar** string )
{
	glShaderSource( shader, count, string, NULL );
	gl_checkError("gl_compile");

	GLint status;
	glCompileShader( shader );
	glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
	if( status == GL_FALSE )
	{
		GLint infoLogLength;
		GLchar* infoLog;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &infoLogLength );
		infoLog = (GLchar*)malloc( infoLogLength );
		glGetShaderInfoLog( shader, infoLogLength, NULL, infoLog );
		std::cerr << "GL compile log: " << (char*)infoLog << std::endl;
		free( infoLog );
		return false;
	}	
	return true;
}

bool gl_link( GLuint program )
{
	GLint status;
	glLinkProgram( program );
	gl_checkError("gl_link");

	glGetProgramiv( program, GL_LINK_STATUS, &status );
	if( status == GL_FALSE )
	{
		GLint infoLogLength;
		GLchar* infoLog;
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &infoLogLength );
		infoLog = (GLchar*)malloc( infoLogLength );
		glGetProgramInfoLog( program, infoLogLength, NULL, infoLog );
		std::cerr << "GL linker log: " << (char*)infoLog << std::endl;
		free( infoLog );
		return false;
	}
	return true;
}

void gl_checkError( const char* funcname )
{
	GLenum error;
	while( (error=glGetError()) != GL_NO_ERROR )
	{
		std::string s;
		switch( error )
		{
		case GL_NO_ERROR         : s = "GL_NO_ERROR";          break;
		case GL_INVALID_ENUM     : s = "GL_INVALID_ENUM";      break;
		case GL_INVALID_VALUE    : s = "GL_INVALID_VALUE";     break;
		case GL_INVALID_OPERATION: s = "GL_INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW   : s = "GL_STACK_OVERFLOW";    break;
		case GL_STACK_UNDERFLOW  : s = "GL_STACK_UNDERFLOW";   break;
		case GL_OUT_OF_MEMORY    : s = "GL_OUT_OF_MEMORY";     break;
		default:
			s += "UNKNOWN GL ERROR";
		}

		std::cerr << "GL error " << error << " (" << s << ") detected in " 
			      << funcname << std::endl;
	}
}

void gl_info( std::ostream& os )
{
	os << "Vendor  : "<< (const char*)glGetString(GL_VENDOR)   << std::endl
	   << "Renderer: "<< (const char*)glGetString(GL_RENDERER) << std::endl
	   << "Version : "<< (const char*)glGetString(GL_VERSION)  << std::endl
	   << "GLSL    : "<< (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION) 
	   << std::endl;
	gl_checkError("gl_info");
}

//------------------------------------------------------------------------------
//  GLUT callbacks
//------------------------------------------------------------------------------

void destroy()
{
	// hopefully current GL context is still active
	if( g_globalApp )
		g_globalApp->destroy();
	gl_checkError("destroy()");
}

void idle()
{
	if( g_globalApp )
		g_globalApp->idle();
}

void reshape( GLint w, GLint h )
{
	if( g_globalApp )
		g_globalApp->reshape( w, h );
	else
	{
		float aspect = w/(float)h;
		glViewport( 0,0, w,h );
	}
	gl_checkError("reshape");
}

void display()
{
	if( g_globalApp )
		g_globalApp->render();
	else
	{
		glClearColor( 0,1,0,1 );
		glClear( GL_COLOR_BUFFER_BIT );
		glFlush();
		glutSwapBuffers();
	}
	gl_checkError("display");
}

void keyboard( unsigned char key, int x, int y )
{
	switch(key) {
		case 27: glutLeaveMainLoop(); break; // esc = quit
	}
}


//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	bool multisampling = true;
	unsigned int displaymode = GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA;
	if( multisampling ) 
		displaymode |= GLUT_MULTISAMPLE;

	// GL3 specific
	glutInitContextVersion(3,3);
	glutInitContextFlags( GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG );
	glutInitContextProfile( GLUT_CORE_PROFILE );

	glutInit( &argc, argv );
	glutInitDisplayMode( displaymode );
	glutInitWindowSize( 512, 512 );
	glutInitWindowPosition( 100, 100 );
	int windowHandle = glutCreateWindow( argv[0] );
	if( windowHandle < 1 )
	{
		std::cerr <<"Error: Could not create new rendering window!"<< std::endl;
		return -1;
	}
	gl_checkError("glutCreateWindow");

	glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE,
	               GLUT_ACTION_GLUTMAINLOOP_RETURNS );

	// GLEW
	glewExperimental = GL_TRUE; // needed for GL3 on my GLEW 1.5.4
	GLenum glew_err = glewInit();
	if( glew_err != GLEW_OK )
	{
		std::cerr << "GLEW error:" << glewGetErrorString(glew_err) << std::endl;
		throw InitException( "Error on initializing GLEW library!" );
	}
	std::cout << "Using GLEW " << glewGetString( GLEW_VERSION ) << std::endl;
	gl_checkError("glewInit");

	gl_info();

	if( g_globalApp )
		g_globalApp->init();

	if( multisampling )
		glEnable( GL_MULTISAMPLE );


	// callbacks
	glutDisplayFunc( display );
	glutReshapeFunc( reshape );
	glutIdleFunc( idle );
	glutKeyboardFunc( keyboard );
	glutCloseFunc( destroy ); // freeglut specific

	glutMainLoop();

	std::cout << "bye!" << std::endl;
	return 0;
}
