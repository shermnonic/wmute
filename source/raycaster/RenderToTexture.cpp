#include "RenderToTexture.h"

bool RenderToTexture::init( int width, int height )
{
	// Create framebuffer object, will be initialized on first bind
	glGenFramebuffers( 1, &m_fbo );
	
	m_width  = width;
	m_height = height;

	return true;
}

void RenderToTexture::deinit()
{
	// Create framebuffer object, will be initialized on first bind
	glDeleteFramebuffers( 1, &m_fbo );	
}

void RenderToTexture::begin( GLint texid )
{
	// Bind FBO
	glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
	
	// Attach texture	
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
	                        GL_TEXTURE_2D, texid, 0 );	

	// Set viewport to match texture size
	glGetIntegerv( GL_VIEWPORT, m_viewport );
	glViewport( 0, 0, m_width, m_height );
}

void RenderToTexture::end()
{
	// Unbind FBO
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	
	// Restore old viewport
	glViewport( m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3] );
}
