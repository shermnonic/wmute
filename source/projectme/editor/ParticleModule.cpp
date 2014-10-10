#include "ParticleModule.h"
#include <glutils/GLError.h>
#include <iostream>
using std::cerr;
using std::endl;
using GL::checkGLError;

//----------------------------------------------------------------------------
ParticleModule::ParticleModule()
: ModuleRenderer( "ParticleModule" ),
  m_initialized( false ),
  m_update( true ),
  m_width(512), m_height(512)
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

	m_initialized = true;
	return true;
}

//----------------------------------------------------------------------------
void ParticleModule::render() 
{ 
	if( !m_initialized ) 	
	{
		init();
	}

	m_ps.update();

	if( m_r2t.bind( m_target.GetID() ) )
	{
		glColor4f( 1.f,1.f,1.f,1.f );

		// Simply render position texture (for debugging)
		glEnable( GL_TEXTURE_2D );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, m_ps.getPositions() );

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
