#include "ShaderModule.h"
#include <glutils/GLError.h>
#include <iostream>
#include <ctime>
using std::cerr;
using std::endl;
#ifdef GL_NAMESPACE
using GL::checkGLError;
#endif

//----------------------------------------------------------------------------
//  Factory registration
//----------------------------------------------------------------------------
#include "ModuleFactory.h"
MODULEFACTORY_REGISTER( ShaderModule, "ShaderModule" )

//----------------------------------------------------------------------------
// Default GLSL 120 / GLSL ES 2.0 shader 
//----------------------------------------------------------------------------
const std::string defaultUniforms =
"uniform vec3      iResolution;\n"           // viewport resolution (in pixels)
"uniform float     iGlobalTime;\n"           // shader playback time (in seconds)
"\n";
/* shadertoy uniforms
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform float     iChannelTime[4];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform samplerXX iChannel0..3;          // input channel. XX = 2D/Cube
uniform vec4      iDate;                 // (year, month, day, time in seconds)
*/

const std::string defaultVertexShader =
defaultUniforms + 
"void main(void)\n"
"{\n"
//"    gl_TexCoord[0] = gl_MultiTexCoord0;\n"; // required?
"    gl_Position = gl_Vertex;\n"
"}\n";

const std::string defaultFragmentShader =
defaultUniforms + 
"void main(void)\n"
"{\n"
"	vec2 uv = gl_FragCoord.xy / iResolution.xy;\n"
"	gl_FragColor = vec4(uv,0.5+0.5*sin(iGlobalTime),1.0);\n"
"}";

//----------------------------------------------------------------------------
ShaderModule::ShaderModule()
: ModuleRenderer( "ShaderModule" ),
  m_initialized(false),
  m_width(512), m_height(512),
  m_shader(0),
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
	if( !m_r2t.init( m_width,m_width, m_target.GetID(), false/* no depthbuffer*/ ) )
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
	if( !compile() )
		// On this call we currently require the shader to compile correctly!
		return false;

	m_initialized = checkGLError( "ShaderModule::init() : GL error at exit!" );
	return m_initialized;
}

//----------------------------------------------------------------------------
void ShaderModule::destroy()
{
	delete m_shader; m_shader = 0;
	m_r2t.deinit();
	m_target.Destroy();
}

//----------------------------------------------------------------------------
bool ShaderModule::compile()
{
	return compile( m_vshader, m_fshader );
}

//----------------------------------------------------------------------------
bool ShaderModule::compile( std::string vshader, std::string fshader )
{
	if( !m_shader ) return false;
	if( !m_shader->load( vshader, fshader ) )
	{
		cerr << "ShaderModule::compile() : Compilation of shaders failed!" << endl;
		return false;
	}	
	return checkGLError( "ShaderModule::compile()" );
}

//----------------------------------------------------------------------------
bool ShaderModule::loadShader( const char* filename )
{
	char* fshader_src = GL::GLSLProgram::read_shader_from_disk( filename );
	if( !fshader_src ) 
		return false;

	std::string fshader( fshader_src );
	if( !compile( m_vshader, fshader ) )
		return false;

	// Replace current fragment shader on succesfull compilation
	m_fshader = fshader;
	return checkGLError( "ShaderModule::loadShader() : GL error at exit!" );
}

//----------------------------------------------------------------------------
void ShaderModule::render()
{
	// Initialized on first render() call; by then we should have a valid 
	// OpenGL context.
	if( !m_initialized && !init() ) return;

	if( !m_shader ) return;
	
	// Time is measured w.r.t. to first render() call
	static clock_t t0 = clock();	
	
	// Bind shader and set default uniforms
	m_shader->bind();
	checkGLError( "ShaderModule::render() - After shader bind" );

	GLint iResolution = m_shader->getUniformLocation("iResolution");
	GLint iGlobalTime = m_shader->getUniformLocation("iGlobalTime");
	if( iResolution >= 0 )
	{
		GLfloat res[3]; 
		res[0] = (GLfloat)m_width; 
		res[1] = (GLfloat)m_height; 
		res[2] = (GLfloat)1.f;
		glUniform3f( m_shader->getUniformLocation( "iResolution" ), 
			(GLfloat)m_width, (GLfloat)m_height, (GLfloat)1.f );
		checkGLError( "ShaderModule::render() - After glUniform3f()" );
	}
	if( iGlobalTime >= 0 )
	{	
		float time = (float)(clock() - t0) / CLOCKS_PER_SEC;
		glUniform1f( m_shader->getUniformLocation( "iGlobalTime" ), time );
		checkGLError( "ShaderModule::render() - After glUniform1f()" );
	}
	checkGLError( "ShaderModule::render() - After shader uniform setup" );
	
	if( m_r2t.bind( m_target.GetID() ) )
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

		glViewport( viewport[0], viewport[1], viewport[2], viewport[3] );
	}
	else
	{
		cerr << "ShaderModule::render() : Could not bind framebuffer!" << endl;
	}
	
	m_shader->release();
	
	m_r2t.unbind();
}

//-----------------------------------------------------------------------------
Serializable::PropertyTree& ShaderModule::serialize() const
{
	static Serializable::PropertyTree cache = Super::serialize();
	
	cache.put("ShaderModule.Name",getName());
	cache.put("ShaderModule.FragmentShader",m_fshader);

	return cache;
}

//-----------------------------------------------------------------------------
void ShaderModule::deserialize( Serializable::PropertyTree& pt )
{
	Super::deserialize( pt );

	setName( pt.get("ShaderModule.Name", getDefaultName()) );
	std::string fshader = pt.get( "ShaderModule.FragmentShader", "" );
	if( !fshader.empty() )
		m_fshader = fshader;
	else
		cerr << "ShaderModule::deserialize : No fragment shader found!" << endl;

	compile();
}
