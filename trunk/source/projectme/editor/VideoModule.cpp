#include "VideoModule.h"
#include "VideoPlayer.h"

#include <glutils/GLError.h>
#ifdef GL_NAMESPACE
using GL::checkGLError;
#endif

#include <cstring> // memcpy
#include <iostream>
#include <cmath> // fabs, log
using std::cerr;
using std::endl;

//----------------------------------------------------------------------------
//  Factory registration
//----------------------------------------------------------------------------
#include "ModuleFactory.h"
MODULEFACTORY_REGISTER( VideoModule, "VideoModule" )


VideoModule::VideoModule()
: ModuleRenderer( "VideoModule" ),
  m_initialized( false ),
  m_videoPlayer( NULL )  
{
}

void VideoModule::setVideoPlayer( VideoPlayer* vp )
{
	m_videoPlayer = vp;
}

bool VideoModule::init()
{
	// Create texture
	if( !m_target.create(GL_TEXTURE_2D) )
	{
		cerr << "ImageModule::init() : Couldn't create 2D textures!" << endl;
		return false;
	}
	
	// Fix format and allocate arbitrary size
    // Note that format should match that of VideoPlayer!
    m_target.image( 0, GL_RGB, 512, 512, 0,
        GL_RGB, GL_UNSIGNED_BYTE, (void*)NULL );
	
	m_initialized = checkGLError( "VideoModule::init() : GL error at exit!" );
	return m_initialized;	
}

void VideoModule::destroy()
{
	m_target.destroy();
}

void VideoModule::render()
{
	// Initialized on first render() call; by then we should have a valid 
	// OpenGL context.	
	if( !m_initialized && !init() ) return;	
	
	if( m_videoPlayer )
	{
		m_videoPlayer->setTexture( m_target.name() );
		m_videoPlayer->poll();
	}
}
