#include "ParticleSystem.h"
#include "hraw.h"

#include <glutils/GLError.h>
#include <cstdlib> // rand(), srand(), RAND_MAX
#include <ctime>   // time() to initialize srand()
#include <cmath>
#include <vector>
using std::cerr;
using std::endl;
using GL::checkGLError;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
// GL helpers
//----------------------------------------------------------------------------
bool checkGLFramebufferStatus( const char* msg )
{
	using namespace std;
	GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if( status != GL_FRAMEBUFFER_COMPLETE )
	{
		cerr << msg << " ";

		switch( status )
		{
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			cout << "(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)" << endl; break;
		//case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
        //	cout << "(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)" << endl; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			cout << "(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)" << endl; break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			cout << "(GL_FRAMEBUFFER_UNSUPPORTED)" << endl; break;
		default:
			cout << "(Unknown error code " << (int)status << "?!)" << endl;
		};

		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
ParticleSystem::ParticleSystem()
: m_initialized( false ),
  m_advectShader( NULL ),
  m_width ( 128 ),
  m_height( 128 ),
  m_texSprite( -1 ),
  m_curTargetBuf( 1 )
{
	m_targetSize[0] = 1024;
	m_targetSize[1] = 1024;
}

//----------------------------------------------------------------------------
void ParticleSystem::setup()
{
	initGL();
}

//----------------------------------------------------------------------------
void ParticleSystem::reseed()
{
	killAllParticles();
	seedParticlePositions();
	seedParticleVelocities();	
	//setSyntheticForceField();
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

	// Convert Matlab 3D array into 4-channel texture data and flip xy
	unsigned stride = size[0]*size[1];
	float* data = new float[ stride * 4 ];
	for( unsigned p=0; p < stride; p++ )
	{
		data[ p*4 + 0 ] = buffer[ p ];        // Fx
		data[ p*4 + 1 ] = buffer[ p+stride ]; // Fy
		data[ p*4 + 2 ] = 0.0;                // Fz
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
	using namespace std;

	// Check OpenGL requiremenets:
	// - GLSL 101
	// - Framebuffer object
	// - Multiple render targets (GL_ARB_draw_buffers, core since GL >= 2.0)
	static bool checkExtensions = true;
	if( checkExtensions )
	{
		if( glewIsSupported("GL_VERSION_2_1  GL_ARB_draw_buffers  GL_EXT_framebuffer_object  GL_ARB_texture_float") )
		{
		  #ifdef _DEBUG // Additional debug info
			GLint max_color_attachements;
			glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS, &max_color_attachements );
			cout << "ParticleSystem::initGL() : " <<
				"GL_MAX_COLOR_ATTACHMENTS = " << max_color_attachements << endl;
		  #endif
		}
		else
		{
			cerr << "ParticleSystem::initGL() : " <<
				"OpenGL minimum requirements not met!" << endl;
		}
		checkExtensions = false;
	}

	//........................................................................
	
	// Init Force texture
	glGenTextures( 1, &m_texForce );
	glBindTexture( GL_TEXTURE_2D, m_texForce );

	// Default RO parameters (no restrictions on filtering)
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	if(	!checkGLError( "ParticleSystem::initGL() : GL error after force texture setup!" ) )
		return false;

	//........................................................................

	loadForceTexture("shader/gradient2.hraw");

	if(	!checkGLError( "ParticleSystem::initGL() : GL error after loading force texture!" ) )
		return false;
	
	//........................................................................
	
	// Create shader
	m_advectShader = new GLSLProgram();
	m_renderShader = new GLSLProgram();
	if( !m_advectShader || !m_renderShader )
	{
		cerr << "ParticleSystem::initGL() : Creation of GLSL shader failed!" << endl;
		return false;
	}
	
	loadShadersFromDisk();	

	if(	!checkGLError( "ParticleSystem::initGL() : GL error after shader setup!" ) )
		return false;	

	//........................................................................

	// Init FBO and seed textures
	glGenTextures( 2, m_texPos );
	glGenTextures( 2, m_texVel );	
	glGenTextures( 1, &m_texBirthPos );
	glGenTextures( 1, &m_texBirthVel );
	
	// Setup FBO textures
#if 0
    GLuint tex[6] = { m_texPos[0], m_texPos[1], m_texVel[0], m_texVel[1],
	                  m_texBirthPos, m_texBirthVel };
#else
	std::vector<GLuint> tex;
	tex.push_back( m_texPos[0] );
	tex.push_back( m_texPos[1] );
	tex.push_back( m_texVel[0] );
	tex.push_back( m_texVel[1] );
	tex.push_back( m_texBirthPos );
	tex.push_back( m_texBirthVel );
#endif
	for( int i=0; i < 6; i++ )
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
	
	if(	!checkGLError( "ParticleSystem::initGL() : GL error after FBO textures setup!" ) )
		return false;

	//........................................................................
	
	// Init frame buffer
	glGenFramebuffers( 1, &m_fbo );
	glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );	

	if(	!checkGLError( "ParticleSystem::initGL() : GL error after FBO creation!" ) )
		return false;
	
	// Attach textures
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texPos[1], 0 );
    if( !checkGLFramebufferStatus( "ParticleSystem::initGL() : Framebuffer status not complete for GL_COLOR_ATTACHMENT0!" ) )
		return false;
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_texVel[1], 0 );
	if( !checkGLFramebufferStatus( "ParticleSystem::initGL() : Framebuffer status not complete for GL_COLOR_ATTACHMENT1!" ) )
		return false;	

	//........................................................................

	// Setup vertex buffer
	unsigned n = m_width*m_height;
	glGenBuffers( 1, &m_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
	glBufferData( GL_ARRAY_BUFFER, 3*sizeof(float)*n, NULL, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	if(	!checkGLError( "ParticleSystem::initGL() : GL error after VBO setup!" ) )
		return false;


	m_initialized = checkGLError( "ParticleSystem::initGL() : GL error at exit!" );
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
	glDeleteTextures( 1, &m_texForce );
	glDeleteTextures( 1, &m_texBirthPos );
	glDeleteTextures( 1, &m_texBirthVel );
}

//----------------------------------------------------------------------------
void ParticleSystem::update()
{
	if( m_initialized )
	{
		advectParticles();
		swapParticleBuffers();
	}
}

//----------------------------------------------------------------------------
void ParticleSystem::render()
{
	if( !m_initialized ) return;

	m_renderShader->bind();
	checkGLError("ParticleSystem::render() : After shader bind");

	// Uniforms
	GLint iPos = m_renderShader->getUniformLocation("iPos");
	GLint iVel = m_renderShader->getUniformLocation("iVel");
	GLint iBirthPos = m_renderShader->getUniformLocation("iBirthPos");
	GLint iDoSprite = m_renderShader->getUniformLocation("doSprite");
	GLint iSprite   = m_renderShader->getUniformLocation("sprite");
	GLint iPointSize= m_renderShader->getUniformLocation("pointSize");
	GLint iSize     = m_renderShader->getUniformLocation("iSize");
	GLint iTargetSize=m_renderShader->getUniformLocation("targetSize");
	glUniform1i( iPos,      0 ); // Texture unit 0 - Position
	glUniform1i( iVel,      1 ); // Texture unit 1 - Velocity	
	glUniform1i( iBirthPos, 2 ); // Texture unit 2 - Birth position 
	glUniform1i( iSprite,   3 ); // Texture unit 3 - Point Sprite (if any)
	glUniform1i( iDoSprite, m_texSprite >= 0 );
	glUniform1f( iPointSize, (m_texSprite>=0) ? 10.f * m_pointSize : m_pointSize );	
	glUniform3f( iSize, m_width,m_height,0.f );
	glUniform3f( iTargetSize, m_targetSize[0], m_targetSize[1], 0.f );

	checkGLError("ParticleSystem::render() : After setting shader uniforms");

	// Bind position and velocity texture
	GLint bufid = m_curTargetBuf;
	glActiveTexture( GL_TEXTURE0 + 0 );	glBindTexture( GL_TEXTURE_2D, m_texPos[bufid] );
	glActiveTexture( GL_TEXTURE0 + 1 );	glBindTexture( GL_TEXTURE_2D, m_texVel[bufid] );
	glActiveTexture( GL_TEXTURE0 + 2 );	glBindTexture( GL_TEXTURE_2D, m_texBirthPos );
	checkGLError("ParticleSystem::render() : After texture bind");

	if( m_texSprite >= 0 )
	{
		glPointSize( 10.f * m_pointSize ); // OBSOLETE: Replaced by gl_PointSize
		glEnable( GL_POINT_SPRITE );
		glTexEnvi( GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); // classical transparency, no pre-multiplied alpha
		//glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA ); // over-operator, pre-multiplied alpha

		glActiveTexture( GL_TEXTURE0 + 3 ); glBindTexture( GL_TEXTURE_2D, m_texSprite );
	}
	else
	{
		glDisable( GL_BLEND );
		glDisable( GL_POINT_SPRITE );
		glPointSize( m_pointSize ); // OBSOLETE: Replaced by gl_PointSize
	}
	glActiveTexture( GL_TEXTURE0 );
	checkGLError("ParticleSystem::render() : After point sprite setup");

	// GL states
	glDisable( GL_DEPTH_TEST );
	glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );
	checkGLError("ParticleSystem::render() : After state setup");


	// Generate vertex stream, position data will be replaced in shader
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
	glVertexPointer( 3, GL_FLOAT, 0, NULL );
	glEnableClientState( GL_VERTEX_ARRAY );
	glDrawArrays( GL_POINTS, 0, m_width*m_height ); // number of vertices

	glFlush(); // FIXME

	// Unbind textures
	for( int i=0; i < 3; i++ )
	{
		glActiveTexture( GL_TEXTURE0 + i );
		glBindTexture( GL_TEXTURE_2D, 0 );
	}
	glActiveTexture( GL_TEXTURE0 );
	
	// Reset states
	glDisable( GL_BLEND );
	glDisable( GL_POINT_SPRITE );
	glDisable( GL_VERTEX_PROGRAM_POINT_SIZE );

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
	GLint iPos0  = m_advectShader->getUniformLocation("iBirthPos");
	GLint iVel0  = m_advectShader->getUniformLocation("iBirthVel");
	glUniform1i( iPos,   0 ); // Texture unit 0 - Position
	glUniform1i( iVel,   1 ); // Texture unit 1 - Velocity
	glUniform1i( iForce, 2 ); // Texture unit 2 - Force
	glUniform1i( iPos0,  3 ); // Texture unit 3 - Birth position (reincarnation)
	glUniform1i( iVel0,  4 ); // Texture unit 4 - Birth velocity (reincarnation)
	checkGLError("ParticleSystem::advectParticles() : After setting shader uniforms");

	
	// Bind FBO
	glBindFramebuffer( GL_FRAMEBUFFER, 0 ); // Unbind first (sanity)
	glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
	checkGLError("ParticleSystem::advectParticles() : After glBindFramebuffer()");
	
	// Attach textures
	// NOTE: Can we avoid re-attaching the textures?
	//       Does re-attaching has relevant impact on the performance?
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texPos[m_curTargetBuf], 0 );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_texVel[m_curTargetBuf], 0 );
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
	glActiveTexture( GL_TEXTURE0 + 0 );	glBindTexture( GL_TEXTURE_2D, m_texPos[srcBuf] );
	glActiveTexture( GL_TEXTURE0 + 1 );	glBindTexture( GL_TEXTURE_2D, m_texVel[srcBuf] );
	glActiveTexture( GL_TEXTURE0 + 2 );	glBindTexture( GL_TEXTURE_2D, m_curTexForce ); // was: m_texForce
	glActiveTexture( GL_TEXTURE0 + 3 );	glBindTexture( GL_TEXTURE_2D, m_texBirthPos );
	glActiveTexture( GL_TEXTURE0 + 4 );	glBindTexture( GL_TEXTURE_2D, m_texBirthVel );
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
	glMultiTexCoord2f( GL_TEXTURE0, 0.f, 0.f );	glVertex3i(-1, -1, 0);
	glMultiTexCoord2f( GL_TEXTURE0, 1.f, 0.f );	glVertex3i(1, -1, 0);
	glMultiTexCoord2f( GL_TEXTURE0, 1.f, 1.f );	glVertex3i(1, 1, 0);
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


	// Unbind textures
	glActiveTexture( GL_TEXTURE0 + 0 );	glBindTexture( GL_TEXTURE_2D, 0 );
	glActiveTexture( GL_TEXTURE0 + 1 );	glBindTexture( GL_TEXTURE_2D, 0 );
	glActiveTexture( GL_TEXTURE0 + 2 );	glBindTexture( GL_TEXTURE_2D, 0 );
	glActiveTexture( GL_TEXTURE0 + 3 );	glBindTexture( GL_TEXTURE_2D, 0 );
	glActiveTexture( GL_TEXTURE0 + 4 );	glBindTexture( GL_TEXTURE_2D, 0 );
	glActiveTexture( GL_TEXTURE0 + 0 ); // Reset to active texture unit 0!
}


//-----------------------------------------------------------------------------
void ParticleSystem::killAllParticles()
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
		// Random (x,y) position in [-1,1]  (will be ignored for lifetime<=0)
		*ptr = 2.f*frand()-1.f; ptr++;
		*ptr = 2.f*frand()-1.f; ptr++;
		*ptr = 0.f; ptr++;  // z = 0
		*ptr = (float)i/500.f; ptr++;  // w = lifetime
		//*ptr = -1.f; ptr++;  // w = lifetime
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
	  #if 1
		// Random (x,y) position in [-1,1]
		*ptr = 2.f*frand()-1.f; ptr++;
		*ptr = 2.f*frand()-1.f; ptr++;
	  #else
		// Random (x,y) position on a ring
		float theta = frand()*2.f*(float)M_PI;
		float r = .8f + frand()*.2f;
		*ptr = r*sin(theta); ptr++;
		*ptr = r*cos(theta); ptr++;
	  #endif
		//*ptr = .0f; ptr++;  // z = 0
		*ptr = frand(); ptr++;  // z = point size (scale factor in [0,1])
		*ptr = .5f+frand(); ptr++;  // w = lifetime
	}

	// Download to GPU
	glBindTexture( GL_TEXTURE_2D, m_texBirthPos );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, m_width,m_height, 0, 
					GL_RGBA, GL_FLOAT, (void*)buf );

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
#if 0
	// Zero velocity
	memset( (void*)but, 0, N*4*sizeof(float) );
#else
	// Random velocity	
	seed(); // Seed random number generator
	float* ptr = &buf[0];
	for( unsigned i=0; i < N; i++ )
	{
		float scale = .2f + frand();
		float theta = frand() * 2.f* (float)M_PI;

		*ptr = scale*cos(theta); ptr++;
		*ptr = scale*sin(theta); ptr++;		
		*ptr = 0.f; ptr++; // z = 0
		*ptr = 1.f; ptr++; // w = 1
	}
#endif

	// Download to GPU
	glBindTexture( GL_TEXTURE_2D, m_texBirthVel );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, m_width,m_height, 0, 
					GL_RGBA, GL_FLOAT, (void*)buf );

	// Free temporary buffer
	delete [] buf;
}

//-----------------------------------------------------------------------------
void ParticleSystem::setSyntheticForceField()
{
	// Discretization of force field domain (not related to FBO size!)
	unsigned width = 1024, height = 1024;

	// Temporary RGBA buffer
	float* buf = new float[ width*height*4 ];

	// Seed random number generator
	seed();

	float* ptr = &buf[0];
	for( unsigned i=0; i < width*height; i++ )
	{
		// Relative position in [0,1]
		float dy = (i/width) / (float)(height-1),
			  dx = (i%width) / (float)(width-1);

		dx -= .5f;
		dy -= .5f;

		float scale = .2f + sqrt(dx*dx + dy*dy);

	  #if 1
		// Rotational field
		*ptr = -scale*dy; ptr++;
		*ptr =  scale*dx; ptr++;
	  #else
		// Central field
		*ptr = scale*dx; ptr++;
		*ptr = scale*dy; ptr++;
	  #endif
		// z = 0, alpha = 1
		*ptr = 0.f; ptr++;
		*ptr = 1.f; ptr++;
	}

	// Download to GPU
	glBindTexture( GL_TEXTURE_2D, m_texForce );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, width,height, 0, 
					GL_RGBA, GL_FLOAT, (void*)buf );

	// Free temporary buffer
	delete [] buf;
}
