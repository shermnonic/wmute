#include "ParticleModule.h"
#include <glutils/GLError.h>
#include <iostream>
using std::cerr;
using std::endl;
using GL::checkGLError;

//----------------------------------------------------------------------------
//  Factory registration
//----------------------------------------------------------------------------
#include "ModuleFactory.h"
MODULEFACTORY_REGISTER( ParticleModule, "ParticleModule" )

//----------------------------------------------------------------------------
ParticleModule::ParticleModule()
: ModuleRenderer( "ParticleModule" ),
  m_initialized( false ),
  m_update( true ),
  m_width(1024), m_height(1024)
{
}

//----------------------------------------------------------------------------
bool ParticleModule::init()
{
	// Create texture
	if( !m_target.Create(GL_TEXTURE_2D) )
	{
		cerr << "ParticleModule::init() : Couldn't create 2D textures!" << endl;
		return false;
	}	

	// Allocate GPU mem
	GLint internalFormat = GL_RGBA32F;
	m_target.Image(0, internalFormat, m_width,m_width, 0, GL_RGBA, GL_FLOAT, NULL );
	
	// Setup Render-2-Texture
	if( !m_r2t.init( m_width,m_width, m_target.GetID(), false/* no depthbuffer*/ ) )
	{
		cerr << "ParticleModule::init() : Couldn't setup render-to-texture!" << endl;
		return false;
	}

	// Setup particle system
	m_ps.setup();
	m_ps.reseed();

	m_initialized = true;
	return true;
}

//----------------------------------------------------------------------------
void ParticleModule::touch() 
{
	m_ps.touch(); 
	m_update=true; 
}

//----------------------------------------------------------------------------

void draw_debug_tex( int pos, GLuint texid )
{
	const GLfloat width = .35f;
	GLfloat ofs = pos*width;

	glBindTexture( GL_TEXTURE_2D, texid );

	glBegin( GL_QUADS );
	glTexCoord2f( 0, 0 );	glVertex3f( -1.f+ofs, -1.f, 0 );
	glTexCoord2f( 1, 0 );	glVertex3f( -1.f+ofs+width, -1.f, 0 );
	glTexCoord2f( 1, 1 );	glVertex3f( -1.f+ofs+width, -1.f+width, 0 );
	glTexCoord2f( 0, 1 );	glVertex3f( -1.f+ofs, -1.f+width, 0 );
	glEnd();
}

void ParticleModule::render() 
{ 
	if( !m_initialized ) 	
	{
		init();
	}

	m_ps.update();

	if( m_r2t.bind( m_target.GetID() ) )
	{
		glDisable( GL_DEPTH_TEST );

		glColor4f( 1.f,1.f,1.f,1.f );

		// Target resolution sized quad
		int viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );
		glViewport( 0,0, m_width,m_height );

		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		// Clear
#if 1
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
#else
		if( m_update )
		{
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			m_update = false;
		}
		else
		{
			//glEnable( GL_BLEND );
			//glBlendFunc( GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA );
			//glColor4f( 0.f, 0.f, 0.f, 0.01f );

			//glBegin( GL_QUADS );
			//glVertex3i(-1, -1, 0);			
			//glVertex3i(1, -1, 0);
			//glVertex3i(1, 1, 0);
			//glVertex3i(-1, 1, 0);
			//glEnd();

			//glDisable( GL_BLEND );
		}
#endif
		// Render particles
		glPointSize( 0.5 );
		glEnable( GL_POINT_SMOOTH );
		glDisable( GL_TEXTURE_2D );
		m_ps.render();

	  #if 0
		// Debug overlays
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		// Overlay position texture (for debugging)
		glEnable( GL_TEXTURE_2D );
		glActiveTexture( GL_TEXTURE0 );

		draw_debug_tex( 0, m_ps.getPositions() );
		draw_debug_tex( 1, m_ps.getPositions2() );
		draw_debug_tex( 2, m_ps.getVelocities() );
		draw_debug_tex( 3, m_ps.getForces() );
		draw_debug_tex( 4, m_ps.getBirthPositions() );
	  #endif

		// Reset viewport
		glViewport( viewport[0], viewport[1], viewport[2], viewport[3] );
	}
	else
	{
		cerr << "ParticleModule::render() : Could not bind framebuffer!" << endl;
	}
	
	m_r2t.unbind();
} 

//-----------------------------------------------------------------------------
Serializable::PropertyTree& ParticleModule::serialize() const
{
	static Serializable::PropertyTree cache;
	cache.clear();
	
	cache.put("ParticleModule.Name",getName());

	return cache;
}

//-----------------------------------------------------------------------------
void ParticleModule::deserialize( Serializable::PropertyTree& pt )
{
	setName( pt.get("ParticleModule.Name", getDefaultName()) );
}
