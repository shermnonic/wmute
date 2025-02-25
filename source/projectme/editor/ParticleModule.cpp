#include "ParticleModule.h"
#include <glutils/GLError.h>
#include <iostream>
#include <ctime>
using std::cout;
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
  m_target_initialized(false),
  m_r2t_initialized   (false),
  m_ps_initialized    (false),
  m_update( true )
{
	// Add parameters
    parameters().push_back( &m_params.fraction  );
	parameters().push_back( &m_params.pointSize );
    parameters().push_back( &m_params.blendMode );
    parameters().push_back( &m_params.animation );
    parameters().push_back( &m_params.animSpeed );
    parameters().push_back( &m_params.timestep  );
	// Add options
	options().push_back( &m_opts.width  );
	options().push_back( &m_opts.height );
}

//----------------------------------------------------------------------------
void ParticleModule::setChannel( int idx, int texId ) 
{ 
	if( idx==1 )
		m_ps.setSpriteTexture( texId );
	else
		m_ps.setForceTexture(texId); 
}

int ParticleModule::channel( int idx ) const 
{ 
	return idx==1 ? m_ps.getSpriteTexture() : m_ps.getForceTexture(); 
}

//----------------------------------------------------------------------------
bool ParticleModule::init()
{
	// Create texture
	if( !m_target_initialized )
	{
		if( !m_target.create(GL_TEXTURE_2D) )
		{
			cerr << "ParticleModule::init() : Couldn't create 2D textures!" << endl;
			return false;
		}	
		m_target_initialized = true;
	}

	// Allocate GPU mem
	GLint internalFormat = GL_RGBA32F;
	m_target.image(0, internalFormat, 
	               m_opts.width.value(), m_opts.height.value(), 
				   0, GL_RGBA, GL_FLOAT, NULL );
	
	// Setup Render-2-Texture
	if( !m_r2t_initialized )
	{
		if( !m_r2t.init_no_depthbuffer( m_target.name() ) )
		{
			cerr << "ParticleModule::init() : Couldn't setup render-to-texture!" << endl;
			return false;
		}
		m_r2t_initialized = true;
	}

	// Setup particle system
	if( !m_ps_initialized )
	{
		m_ps.setup();
		m_ps_initialized = true;
	}
	m_ps.reseed();

	m_initialized = checkGLError( "ParticleModule::init() : GL error at exit!" );
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

    m_ps.setTimestep( (float)(m_params.timestep.value()/100.0) );
	m_ps.update();

	if( m_r2t.bind( m_target.name() ) )
	{
		glDisable( GL_DEPTH_TEST );

		glColor4f( 1.f,1.f,1.f,1.f );

		// Target resolution sized quad
		int viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );
		glViewport( 0,0, m_opts.width.value(),m_opts.height.value() );

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

        // Animation
        static clock_t t0 = clock(); // Time is measured w.r.t. to last render() call
        double deltaTime = 0.001*(double)(clock() - t0) / CLOCKS_PER_SEC;
        if( m_params.animation.value()==0 )
        {
            // No animation
        }
        else
        if( m_params.animation.value()==1 )
        {
            // Smoothly increase number of simulated particles
            m_params.fraction.setValue(
                m_params.fraction.valueRef() + m_params.animSpeed.value() * deltaTime );

            if( m_params.fraction.value() >= 1.0 )
            {
                cout << "Particle animation blend in finished." << endl;
                m_params.animation.setValue( 0 );
                m_params.fraction.setValue( 1.0 );
            }
        }
        else
        if( m_params.animation.value()==2 )
        {
            // Smoothly decrease number of simulated particles
            m_params.fraction.setValue(
                m_params.fraction.valueRef() - m_params.animSpeed.value() * deltaTime );

            if( m_params.fraction.value() <= 0.0 )
            {
                cout << "Particle animation blend out finished." << endl;
                m_params.animation.setValue( 0 );
                m_params.fraction.setValue( 0.0 );
            }
        }

		// Render particles
        m_ps.setFraction( m_params.fraction.value() );
        m_ps.setSpriteBlending( m_params.blendMode.value() );
		m_ps.setPointSize( (float)m_params.pointSize.value() );
		m_ps.setTargetSize( (float)m_opts.width.value(), (float)m_opts.height.value() );
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
	cache = Super::serialize();	

	// Nothing to serialize yet!

	return cache;
}

//-----------------------------------------------------------------------------
void ParticleModule::deserialize( Serializable::PropertyTree& pt )
{
	Super::deserialize( pt );

	// Nothing to deserialize yet!
}
