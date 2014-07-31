#include "ShaderModule.h"
#include <iostream>
#include <ctime>

using std::cerr;
using std::endl;

using GL::checkGLError;

//----------------------------------------------------------------------------
// Default GLSL 120 / GLSL ES 2.0 shader 
//----------------------------------------------------------------------------
const std::string defaultVertexShader =
"void main(void)\n"
"{\n"
//"    gl_TexCoord[0] = gl_MultiTexCoord0;\n"; // required?
"    gl_Position = gl_Vertex;\n"
"}\n";

/* shadertoy uniforms
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform float     iChannelTime[4];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform samplerXX iChannel0..3;          // input channel. XX = 2D/Cube
uniform vec4      iDate;                 // (year, month, day, time in seconds)
*/
const std::string defaultFragmentShader =
"uniform vec3      iResolution;\n"           // viewport resolution (in pixels)
"uniform float     iGlobalTime;\n"           // shader playback time (in seconds)
"\n"
"void main(void)\n"
"{\n"
"	vec2 uv = gl_FragCoord.xy / iResolution.xy;\n"
"	gl_FragColor = vec4(uv,0.5+0.5*sin(iGlobalTime),1.0);\n"
"}";

//----------------------------------------------------------------------------
ShaderModule::ShaderModule()
: m_width(512), m_height(512),
  m_vshader( defaultVertexShader ),
  m_fshader( defaultFragmentShader )
{
}

//----------------------------------------------------------------------------
bool ShaderModule::init()
{
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
	m_target.Image(0, internalFormat, m_width,m_width, 0, GL_RGBA, GL_FLOAT, NULL );
	
	// Setup Render-2-Texture
	if( !m_r2t.init( m_width,m_width, m_target.GetID(), true ) )
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
	
	// Compile shader
	compile();

	return checkGLError( "ShaderModule::init() : GL error at exit!" );
}

//----------------------------------------------------------------------------
void ShaderModule::destroy()
{
	m_r2t.deinit();
	m_target.Destroy();
}

//----------------------------------------------------------------------------
bool ShaderModule::compile()
{
	if( !m_shader ) return;
	if( !m_shader->load( m_vshader, m_fshader ) )
	{
		cerr << "ShaderModule::compile() : Compilation of shaders failed!" << endl;
		return false;
	}
	
	return checkGLError( "ShaderModule::compile()" );
}

//----------------------------------------------------------------------------
void ShaderModule::render()
{
	if( !m_shader ) return;	
	
	// Time is measured w.r.t. to first render() call
	static clock_t t0 = clock();	
	
	// Bind shader and set default uniforms
	m_shader->bind();
	
	GLfloat res[3]; 
	res[0] = (GLfloat)m_width; 
	res[1] = (GLfloat)m_height; 
	res[2] = (GLfloat)1.f;
	glUniform3f( m_shader->getUniformLocation( "iResolution" ), res );
	
	float time = (float)(clock() - t0) / CLOCKS_PER_SEC;
	glUniform1f( m_shader->getUniformLocation( "iGlobalTime" ), time );
	
	if( m_r2t.bind( m_target ) )
	{
		// Render target resolution sized quad		
		int viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );
		glViewport( 0,0, m_width,m_height );
		
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
		
		glBegin(GL_QUADS );
		glTexCoord2f( 0, 0 );	glVertex3i(-1, -1, 0);			
		glTexCoord2f( 1, 0 );	glVertex3i(1, -1, 0);
		glTexCoord2f( 1, 1 );	glVertex3i(1, 1, 0);
		glTexCoord2f( 0, 1 );	glVertex3i(-1, 1, 0);
		glEnd();
	}
	else
	{
		cerr << "ShaderModule::render() : Could not bind framebuffer!" << endl;
	}
	
	m_shader->release();
	
	m_r2t.unbind();
}
