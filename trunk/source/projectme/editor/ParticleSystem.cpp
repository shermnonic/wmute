#include "ParticleSystem.h"

#include <glutils/GLError.h>
#include <cstdlib> // rand(), srand(), RAND_MAX
#include <ctime>   // time() to initialize srand()
using std::cerr;
using std::endl;
using GL::checkGLError;

//----------------------------------------------------------------------------
// Random numbers
//----------------------------------------------------------------------------

/// Seed std::rand() with given positive number, or time(NULL) if n < 0
void seed( int n=-1 )
{
	srand( (unsigned)((n<0) ? time(NULL) : n) );
}

/// Return uniform random float in [0,1]
float frand()
{
	return (rand() / (float)RAND_MAX);
}

//----------------------------------------------------------------------------
// GLSL 110 / GLSL ES 2.0 shader 
//----------------------------------------------------------------------------
const std::string particleShaderPreamble =
"#version 120\n"
"uniform sampler2D iPos;\n"
"uniform sampler2D iVel;\n"
"varying vec2 vTexCoord;\n"
"varying vec4 vColor;\n"
"\n";

const std::string particleVertexShader =
particleShaderPreamble + 
"void main(void)\n"
"{\n"
//"    vTexCoord = gl_MultiTexCoord0.xy;\n"
//"    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"    vTexCoord = gl_Vertex.xy;\n"
"    vColor = gl_Color;\n"
"    gl_Position = gl_Vertex;\n"
"}\n";

const std::string particleFragmentShader =
particleShaderPreamble + 
"void main(void)\n"
"{\n"
//"   vec2 tc = gl_TexCoord[0].xy;\n"
"   vec2 tc = vColor.xy;\n"//"vTexCoord;\n" 
//"   vec2 uv = gl_FragCoord.xy / vec2(256.0,256.0);\n"
//"   vec4 pos = texture2D( iPos, tc );\n"
//"   vec4 vel = texture2D( iVel, tc );\n"
"   gl_FragData[0] = vColor;\n"//vec4( tc.x, tc.y, 1.0, 1.0 );\n" //"vec4( pos.xyz, 1.0 );\n"
"   gl_FragData[1] = vec4( tc.x, tc.y, 0.0, 1.0 );\n" //"vec4( vel.xyz, 1.0 );\n"
"}";


//----------------------------------------------------------------------------
#include <fstream>
#include <sstream>
bool loadShader( const char* filename, std::string& source )
{
	using namespace std;

	ifstream f( filename );
	if( !f.good() )
		return false;

	stringstream ss;
	string line;
	while( getline(f,line) )
		ss << line << "\n";
	f.close();

	source = ss.str();

	return true;
}

//----------------------------------------------------------------------------
ParticleSystem::ParticleSystem()
: m_initialized( false ),
  m_shader( NULL ),
  m_vshader( particleVertexShader ),
  m_fshader( particleFragmentShader ),
  m_width ( 256 ),
  m_height( 256 ),
  m_curTargetBuf( 1 )
{
}

//----------------------------------------------------------------------------
void ParticleSystem::setup()
{
	initGL();
	seedParticlePositions();
	seedParticleVelocities();	
}

//----------------------------------------------------------------------------
bool ParticleSystem::compile()
{
	return compile( m_vshader, m_fshader );
}

//----------------------------------------------------------------------------
bool ParticleSystem::compile( std::string vshader, std::string fshader )
{
	if( !m_shader ) return false;
	if( !m_shader->load( vshader, fshader ) )
	{
		cerr << "ParticleSystem::compile() : Compilation of shaders failed!" << endl;
		return false;
	}	
	return checkGLError( "ParticleSystem::compile()" );
}

//----------------------------------------------------------------------------
void ParticleSystem::reloadShaderFromDiskHack()
{
	// Load from disk (hardcoded paths!)
	std::string vs, fs;
	if( !loadShader( "shader/particle.vs", vs ) ||
	    !loadShader( "shader/particle.fs", fs ) )
	{
		std::cerr << "ParticleSystem::reloadShaderFromDiskHack() : "
			<< "Could not load particle shaders!" << std::endl;
		return;
	}

	// Test compile
	if( !compile( vs, fs ) )
	{
		// Compilation failed, switch back to last working configuration
		std::cerr << "ParticleSystem::reloadShaderFromDiskHack() : "
			<< "Compilation failed, switching back to last working version!" << std::endl;

		compile();
		return;
	}
	else
	{
		// Success! Replace working shader set.
		m_vshader = vs;
		m_fshader = fs;
	}
}

//----------------------------------------------------------------------------
bool ParticleSystem::initGL()
{	
	// TODO: Sanity checks!
	// We require
	// - GLSL 101
	// - Framebuffer object
	// - Multiple render targets
	
	//........................................................................
	
	// Create shader
	m_shader = new GLSLProgram();
	if( !m_shader )
	{
		cerr << "ParticleSystem::init() : Creation of GLSL shader failed!" << endl;
		return false;
	}
	
	// Compile shader
	if( !compile() )
		// On this call we currently require the shader to compile correctly!
		return false;	
	
	if(	!checkGLError( "ParticleSystem::init() : GL error after shader setup!" ) )
		return false;	

	//........................................................................

	// Init textures
	glGenTextures( 2, m_texPos );
	glGenTextures( 2, m_texVel );	
	
	// Setup textures
	GLuint tex[4] = { m_texPos[0], m_texPos[1], m_texVel[0], m_texVel[1] };
	for( int i=0; i < 4; i++ )
	{
		// Allocate
		glBindTexture( GL_TEXTURE_2D, tex[i] );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, m_width,m_height, 0, GL_RGBA, GL_FLOAT, NULL );
		
		// Default parameters
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	}
	
	if(	!checkGLError( "ParticleSystem::init() : GL error after texture setup!" ) )
		return false;

	//........................................................................
	
	// Init frame buffer
	glGenFramebuffers( 1, &m_fbo );
	glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );	
	
	// Attach textures
	glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texPos[1], 0 );
	glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_texVel[1], 0 );	
	
	GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if( status != GL_FRAMEBUFFER_COMPLETE )
	{
		cerr << "ParticleSystem::init() : Framebuffer status not complete!" << endl;
		return false;
	}

	m_initialized = checkGLError( "ParticleSystem::init() : GL error at exit!" );
	return m_initialized;
}

//----------------------------------------------------------------------------
void ParticleSystem::destroyGL()
{
	delete m_shader; m_shader=NULL;
	glDeleteFramebuffers( 1, &m_fbo );
	glDeleteTextures( 2, m_texPos );
	glDeleteTextures( 2, m_texVel );
}

//----------------------------------------------------------------------------
void ParticleSystem::swapParticleBuffers()
{
	m_curTargetBuf = (m_curTargetBuf+1)%2;
}

//----------------------------------------------------------------------------
void ParticleSystem::advectParticles()
{
	glPushAttrib( GL_VIEWPORT_BIT );
	checkGLError("ParticleSystem::advectParticles() : initial glPushAttrib() at the beginning");
	// NOTE:
	// glPushAttrib() can not be called here with GL_COLOR_BUFFER_BIT and will
	// cause an GL_INVALID_OPERATION on the glPopAttrib(). Maybe the buffer
	// pointers can not be restored correctly when rendering to a framebuffer?

	// Bind shader
	m_shader->bind();
	checkGLError("ParticleSystem::advectParticles() : After shader->bind()");

	// Set shader uniforms / samplers	
	GLint iPos = m_shader->getUniformLocation("iPos");
	GLint iVel = m_shader->getUniformLocation("iVel");
	glUniform1i( iPos, 0 ); // Texture unit 0
	glUniform1i( iVel, 1 ); // Texture unit 1
	checkGLError("ParticleSystem::advectParticles() : After setting shader uniforms");


	// Bind FBO
	glBindFramebuffer( GL_FRAMEBUFFER, 0 ); // Unbind first (sanity)
	glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
	checkGLError("ParticleSystem::advectParticles() : After glBindFramebuffer()");

	// NOTE: 
	// Can we do this somehow with out re-attaching the textures?
	// Does re-attaching have relevant impact on the performance?
	
	// Attach textures
	glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texPos[m_curTargetBuf], 0 );
	glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_texVel[m_curTargetBuf], 0 );
	checkGLError("ParticleSystem::advectParticles() : After glFramebufferTexture()");
	
	// NOTE on render targets:
	// (See http://zach.in.tu-clausthal.de/teaching/cg_literatur/frame_buffer_objects.html)
	// "An FBO remembers the last render target it was told to use, as such by 
	// doing this while the FBO is bound we can set this at startup and not have 
	// to worry about doing so during the main rendering loop."
	GLenum mrt[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers( 2, mrt );
	checkGLError("ParticleSystem::advectParticles() : After glDrawBuffers()");	

	// SANITY
	if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
	{
		std::cerr << "ParticleSystem::advectParticles() : Framebuffer incomplete!" << std::endl;
		glPopAttrib();
		return;
	}


	// Bind textures
	unsigned srcBuf = (m_curTargetBuf+1)%2;
	glActiveTexture( GL_TEXTURE0 + 0 );
	glBindTexture( GL_TEXTURE_2D, m_texVel[srcBuf] );
	glActiveTexture( GL_TEXTURE0 + 1 );
	glBindTexture( GL_TEXTURE_2D, m_texPos[srcBuf] );
	checkGLError("ParticleSystem::advectParticles() : After texture bind");

	
	// Generate fragment stream for whole particle buffer
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	
	glViewport( 0,0, m_width,m_height );
	//glClearColor( 0.f,0.f,0.f,1.f );
	//glClear( GL_COLOR_BUFFER_BIT ); // Clear should not be required?

	glBegin( GL_QUADS );
	glColor3f( 1,1,1 );
	glMultiTexCoord2f( GL_TEXTURE0, 0.f, 0.f );	glVertex3i(-1, -1, 0);
	glColor3f( 0,1,0 );
	glMultiTexCoord2f( GL_TEXTURE0, 1.f, 0.f );	glVertex3i(1, -1, 0);
	glColor3f( 0,0,1 );
	glMultiTexCoord2f( GL_TEXTURE0, 1.f, 1.f );	glVertex3i(1, 1, 0);
	glColor3f( 1,0,0 );
	glMultiTexCoord2f( GL_TEXTURE0, 0.f, 1.f );	glVertex3i(-1, 1, 0);
	glEnd();
	
	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );	
	
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	checkGLError("ParticleSystem::advectParticles() : undbinding framebuffer at the end");

	m_shader->release();
	checkGLError("ParticleSystem::advectParticles() : releasing shader at the end");

	glPopAttrib();
	checkGLError("ParticleSystem::advectParticles() : final glPopAttrib() at the end");
}

//-----------------------------------------------------------------------------
void ParticleSystem::seedParticlePositions()
{
	// Max. number of particles
	unsigned N = (unsigned)m_width*m_height;

	// Temporary RGBA buffer
	float* buf = new float[ N*4 ];

	// Seed random number generator
	seed();

	float* ptr = &buf[0];
	for( unsigned i=0; i < N; i++ )
	{
		// Random (x,y) position in [0,1]
		*ptr = frand(); ptr++;
		*ptr = frand(); ptr++;
		// z = 0, alpha = 1
		*ptr = 0.f; ptr++;
		*ptr = 1.f; ptr++;
	}

	// Download to GPU
	for( int i=0; i < 2; i++ )
	{
		glBindTexture( GL_TEXTURE_2D, m_texPos[i] );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, m_width,m_height, 0, 
					  GL_RGBA, GL_FLOAT, (void*)buf );
	}

	// Free temporary buffer
	delete [] buf;
}

//-----------------------------------------------------------------------------
void ParticleSystem::seedParticleVelocities()
{
	// Max. number of particles
	unsigned N = (unsigned)m_width*m_height;

	// Temporary RGBA buffer
	float* buf = new float[ N*4 ];

	// Seed random number generator
	seed();

	float* ptr = &buf[0];
	for( unsigned i=0; i < N; i++ )
	{
		// Relative position in [0,1]
		float dy = (i/m_width) / (float)(m_height-1),
			  dx = (i%m_width) / (float)(m_width-1);

		float scale = 5.f;

		// Some angular velocity in (x,y) around center of rotation at (.5,.5)
		*ptr = -scale*(dy-.5f); ptr++;
		*ptr =  scale*(dx-.5f); ptr++;
		// z = 0, alpha = 1
		*ptr = 0.f; ptr++;
		*ptr = 1.f; ptr++;
	}

	// Download to GPU
	for( int i=0; i < 2; i++ )
	{
		glBindTexture( GL_TEXTURE_2D, m_texVel[i] );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, m_width,m_height, 0, 
					  GL_RGBA, GL_FLOAT, (void*)buf );
	}

	// Free temporary buffer
	delete [] buf;
}
