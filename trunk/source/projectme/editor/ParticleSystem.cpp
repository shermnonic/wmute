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
"#version 110\n"
"uniform sampler2D iPos;\n"
"uniform sampler2D iVel;\n"
"\n";

const std::string particleVertexShader =
particleShaderPreamble + 
"void main(void)\n"
"{\n"
"    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"    gl_Position = gl_Vertex;\n"
"}\n";

const std::string particleFragmentShader =
particleShaderPreamble + 
"void main(void)\n"
"{\n"
"   vec2 tc = gl_TexCoord[0].xy;\n"
"   vec4 pos = texture2D( iPos, tc );\n"
"   vec4 vel = texture2D( iVel, tc );\n"
"   gl_FragData[0] = vec4( pos.xyz, 1.0 );\n"
"   gl_FragData[1] = vec4( vel.xyz, 1.0 );\n"
"}";


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
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texPos[1], 0 );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_texVel[1], 0 );	
	
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
	glPushAttrib( GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT );
	checkGLError("ParticleSystem::advectParticles() : initial glPushAttrib() at the beginning");

	// Bind FBO
	glBindFramebuffer( GL_FRAMEBUFFER, 0 ); // Unbind first (sanity)
	glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
	checkGLError("ParticleSystem::advectParticles() : After glBindFramebuffer()");

	// NOTE: 
	// Can we do this somehow with out re-attaching the textures?
	// Does re-attaching have relevant impact on the performance?
	
	// Attach textures
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texPos[m_curTargetBuf], 0 );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_texVel[m_curTargetBuf], 0 );
	checkGLError("ParticleSystem::advectParticles() : After glFramebufferTexture2D()");
	
	// NOTE on render targets:
	// (See http://zach.in.tu-clausthal.de/teaching/cg_literatur/frame_buffer_objects.html)
	// "An FBO remembers the last render target it was told to use, as such by 
	// doing this while the FBO is bound we can set this at startup and not have 
	// to worry about doing so during the main rendering loop."
	GLenum mrt[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers( 2, mrt );
	checkGLError("ParticleSystem::advectParticles() : After glDrawBuffers()");
	
	glViewport( 0,0, m_width,m_height );

	// Bind textures
	unsigned srcBuf = (m_curTargetBuf+1)%2;
	glActiveTexture( GL_TEXTURE0 + 0 );
	glBindTexture( GL_TEXTURE_2D, m_texVel[srcBuf] );
	glActiveTexture( GL_TEXTURE0 + 1 );
	glBindTexture( GL_TEXTURE_2D, m_texPos[srcBuf] );
	checkGLError("ParticleSystem::advectParticles() : After texture bind");

	// Bind shader
	m_shader->bind();
	checkGLError("ParticleSystem::advectParticles() : After shader->bind()");

	// Set shader uniforms / samplers	
	GLint iPos = m_shader->getUniformLocation("iPos");
	GLint iVel = m_shader->getUniformLocation("iVel");
	glUniform1i( iPos, 0 ); // Texture unit 0
	glUniform1i( iVel, 1 ); // Texture unit 1
	checkGLError("ParticleSystem::advectParticles() : After setting shader uniforms");
	
	// Generate fragment stream for whole particle buffer
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS );
	glTexCoord2f( 0, 0 );	glVertex3i(-1, -1, 0);
	glTexCoord2f( 1, 0 );	glVertex3i(1, -1, 0);
	glTexCoord2f( 1, 1 );	glVertex3i(1, 1, 0);
	glTexCoord2f( 0, 1 );	glVertex3i(-1, 1, 0);
	glEnd();

	m_shader->release();
	checkGLError("ParticleSystem::advectParticles() : releasing shader at the end");
	
	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );	
	
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	checkGLError("ParticleSystem::advectParticles() : undbinding framebuffer at the end");

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
