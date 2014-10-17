#include "ParticleSystem.h"
#include "hraw.h"

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
  m_advectShader( NULL ),
  m_width ( 256 ),
  m_height( 256 ),
  m_curTargetBuf( 1 )
{
}

//----------------------------------------------------------------------------
void ParticleSystem::setup()
{
	initGL();
}

//----------------------------------------------------------------------------
void ParticleSystem::reseed()
{
	seedParticlePositions();
	seedParticleVelocities();	
}

//----------------------------------------------------------------------------
void ParticleSystem::loadForceTexture( const char* filename )
{
	// Read raw buffer
	float* buffer = NULL;
	unsigned size[3];
	if( !read_hraw( filename, buffer, size ) )
	{
		std::cerr << "ParticleSystem::loadForceTexture() : "
			<< "Could not load texture from \"" << filename << "\"!" << std::endl;
		return;
	}
	if( size[2] < 2 )
	{
		std::cerr << "ParticleSystem::loadForceTexture() : "
			<< "Raw data has too few dimensions, at least 2 are required!" << std::endl;
		return;
	}

	// Convert Matlab 3D array into 4-channel texture data
	unsigned stride = size[0]*size[1];
	float* data = new float[ stride * 4 ];
	for( unsigned p=0; p < stride; p++ )
	{
		data[ p*4 + 0 ] = buffer[ p ];        // x
		data[ p*4 + 1 ] = buffer[ p+stride ]; // y
		data[ p*4 + 2 ] = 0.0;                // z
		data[ p*4 + 3 ] = 1.0;                // w
	}

	// Download to GPU
	glBindTexture( GL_TEXTURE_2D, m_texForce );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, size[0],size[1], 0, 
					GL_RGBA, GL_FLOAT, (void*)data );

	// Free buffers
	delete [] data;
	delete [] buffer;
}

//----------------------------------------------------------------------------
void ParticleSystem::loadShadersFromDisk()
{
	// Load from disk (hardcoded paths!)

	if( !m_advectShader->load_from_disk( 
		"shader/particle_advect.vs", 
		"shader/particle_advect.fs" ) )
	{
		std::cerr << "ParticleSystem::loadShadersFromDisk() : "
			<< "Could not link particle advection shader!" << std::endl;
		return;
	}

	if( !m_renderShader->load_from_disk(
		"shader/particle_render.vs",
		"shader/particle_render.fs" ) )
	{
		std::cerr << "ParticleSystem::loadShadersFromDisk() : "
			<< "Could not link particle render shader!" << std::endl;
		return;
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
	
	// Init Force texture
	glGenTextures( 1, &m_texForce );
	glBindTexture( GL_TEXTURE_2D, m_texForce );

	// Default RO parameters (no restrictions on filtering)
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	if(	!checkGLError( "ParticleSystem::init() : GL error after force texture setup!" ) )
		return false;

	//........................................................................

	loadForceTexture("shader/gradient.hraw");

	if(	!checkGLError( "ParticleSystem::init() : GL error after loading force texture!" ) )
		return false;
	
	//........................................................................
	
	// Create shader
	m_advectShader = new GLSLProgram();
	m_renderShader = new GLSLProgram();
	if( !m_advectShader || !m_renderShader )
	{
		cerr << "ParticleSystem::init() : Creation of GLSL shader failed!" << endl;
		return false;
	}
	
	loadShadersFromDisk();	

	if(	!checkGLError( "ParticleSystem::init() : GL error after shader setup!" ) )
		return false;	

	//........................................................................

	// Init FBO textures
	glGenTextures( 2, m_texPos );
	glGenTextures( 2, m_texVel );	
	
	// Setup FBO textures
	GLuint tex[4] = { m_texPos[0], m_texPos[1], m_texVel[0], m_texVel[1] };
	for( int i=0; i < 4; i++ )
	{
		// Allocate
		glBindTexture( GL_TEXTURE_2D, tex[i] );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, m_width,m_height, 0, GL_RGBA, GL_FLOAT, NULL );
		
		// Default FBO parameters (only nearest filtering allowed!)
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	}
	
	if(	!checkGLError( "ParticleSystem::init() : GL error after FBO textures setup!" ) )
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

	//........................................................................

	// Setup vertex buffer
	unsigned n = m_width*m_height;
	glGenBuffers( 1, &m_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
	glBufferData( GL_ARRAY_BUFFER, 3*sizeof(float)*n, NULL, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	if(	!checkGLError( "ParticleSystem::init() : GL error after VBO setup!" ) )
		return false;


	m_initialized = checkGLError( "ParticleSystem::init() : GL error at exit!" );
	return m_initialized;
}

//----------------------------------------------------------------------------
void ParticleSystem::destroyGL()
{
	delete m_advectShader; m_advectShader=NULL;
	delete m_renderShader; m_renderShader=NULL;
	glDeleteFramebuffers( 1, &m_fbo );
	glDeleteTextures( 2, m_texPos );
	glDeleteTextures( 2, m_texVel );
}

//----------------------------------------------------------------------------
void ParticleSystem::update()
{
	advectParticles();
	swapParticleBuffers();
}

//----------------------------------------------------------------------------
void ParticleSystem::render()
{
	m_renderShader->bind();
	checkGLError("ParticleSystem::render() : After shader bind");

	// Bind position and velocity texture
	GLint bufid = m_curTargetBuf;
	glActiveTexture( GL_TEXTURE0 + 0 );
	glBindTexture( GL_TEXTURE_2D, m_texPos[bufid] );
	glActiveTexture( GL_TEXTURE0 + 1 );
	glBindTexture( GL_TEXTURE_2D, m_texVel[bufid] );
	glActiveTexture( GL_TEXTURE0 + 0 );
	checkGLError("ParticleSystem::render() : After texture bind");

	// Generate vertex stream, position data will be replaced in shader
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
	glVertexPointer( 3, GL_FLOAT, 0, NULL );
	glEnableClientState( GL_VERTEX_ARRAY );
	glDrawArrays( GL_POINTS, 0, m_width*m_height ); // number of vertices

	glFlush(); // FIXME

	m_renderShader->release();
	checkGLError("ParticleSystem::render() : After shader release");
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
	m_advectShader->bind();
	checkGLError("ParticleSystem::advectParticles() : After shader->bind()");

	// Set shader uniforms / samplers	
	GLint iPos   = m_advectShader->getUniformLocation("iPos");
	GLint iVel   = m_advectShader->getUniformLocation("iVel");
	GLint iForce = m_advectShader->getUniformLocation("iForce");
	glUniform1i( iPos,   0 ); // Texture unit 0 - Position
	glUniform1i( iVel,   1 ); // Texture unit 1 - Velocity
	glUniform1i( iForce, 2 ); // Texture unit 2 - Force
	checkGLError("ParticleSystem::advectParticles() : After setting shader uniforms");

	
	// Bind FBO
	glBindFramebuffer( GL_FRAMEBUFFER, 0 ); // Unbind first (sanity)
	glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
	checkGLError("ParticleSystem::advectParticles() : After glBindFramebuffer()");
	
	// Attach textures
	// NOTE: Can we avoid re-attaching the textures?
	//       Does re-attaching has relevant impact on the performance?
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
	glBindTexture( GL_TEXTURE_2D, m_texPos[srcBuf] );
	glActiveTexture( GL_TEXTURE0 + 1 );
	glBindTexture( GL_TEXTURE_2D, m_texVel[srcBuf] );
	glActiveTexture( GL_TEXTURE0 + 2 );
	glBindTexture( GL_TEXTURE_2D, m_texForce );
	glActiveTexture( GL_TEXTURE0 + 0 ); // Reset to active texture unit 0!
	checkGLError("ParticleSystem::advectParticles() : After texture bind");
	
	// Generate fragment stream for whole particle buffer
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	
	glViewport( 0,0, m_width,m_height );

	glBegin( GL_QUADS );
	glColor3f( 0,0,0 );
	glMultiTexCoord2f( GL_TEXTURE0, 0.f, 0.f );	glVertex3i(-1, -1, 0);
	glColor3f( 1,0,0 );
	glMultiTexCoord2f( GL_TEXTURE0, 1.f, 0.f );	glVertex3i(1, -1, 0);
	glColor3f( 1,1,0 );
	glMultiTexCoord2f( GL_TEXTURE0, 1.f, 1.f );	glVertex3i(1, 1, 0);
	glColor3f( 0,1,0 );
	glMultiTexCoord2f( GL_TEXTURE0, 0.f, 1.f );	glVertex3i(-1, 1, 0);
	glEnd();
	
	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );	

	checkGLError("ParticleSystem::advectParticles() : after fragment stream generation");


	// Tidy up

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	checkGLError("ParticleSystem::advectParticles() : undbinding framebuffer at the end");

	m_advectShader->release();
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
		// Random (x,y) position in [-1,1]
		*ptr = 2.f*frand()-1.f; ptr++;
		*ptr = 2.f*frand()-1.f; ptr++;
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
#if 1
	memset( (void*)ptr, 0, N*4*sizeof(float) );
#else
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
#endif

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
