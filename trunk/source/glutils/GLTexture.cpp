#include "GLTexture.h"
#include "GLError.h"
#ifdef _WIN32
#include <windows.h>   // used to check if Gl context is active
#endif
#include <cassert>
#include <iostream>
#include <iomanip>

#ifdef GL_NAMESPACE
using namespace GL;
#endif

GLTexture::GLTexture()
: m_valid( false ),
  m_id    ( 0 ),
  m_width ( 0 ),
  m_height( 0 ),
  m_depth ( 0 )
{
  #ifdef DEBUG
	std::cout << "GLTexture::GLTexture() new m_texture = 0x" << std::hex << m_texture << std::endl;
  #endif
}

GLTexture::~GLTexture()
{
  #ifdef DEBUG
	std::cout << "GLTexture::GLTexture() delete m_texture = 0x" << std::hex << m_texture << std::endl;
  #endif
}

bool GLTexture::setWrapMode( GLint wrapMode )
{
	bool ok = true;
	switch( m_target )
	{
	case GL_TEXTURE_3D:
		ok &= setParameter( GL_TEXTURE_WRAP_R, wrapMode );
	default:
	case GL_TEXTURE_2D:
		ok &= setParameter( GL_TEXTURE_WRAP_T, wrapMode );
	case GL_TEXTURE_1D:
		ok &= setParameter( GL_TEXTURE_WRAP_S, wrapMode );
	}
	return ok;
}

bool GLTexture::setFilterMode( GLint mode )
{
	// Set same filtering for MAG/MIN
	return setFilterMode( mode, mode );
}

bool GLTexture::setFilterMode( GLint min_mode, GLint mag_mode )
{
	bool ok = true;
	ok &= setParameter( GL_TEXTURE_MIN_FILTER, min_mode );
	ok &= setParameter( GL_TEXTURE_MAG_FILTER, mag_mode );	
	return ok;
}

bool GLTexture::create( GLenum target )
{
	glGenTextures(1, &m_id);
	m_target = target;
	m_valid = CheckGLError("GLTexture::create()");

	// Convenience: Set default parameters
	bool ok = true;
	ok &= setWrapMode( GL_CLAMP );
	ok &= setFilterMode( GL_LINEAR );	
	
	return m_valid & ok;
}

bool GLTexture::destroy()
{	
	if( m_valid )
	{
		glDeleteTextures( 1, &m_id );
		invalidate();		
	}	
	return CheckGLError("GLTexture::destroy()");
}

bool GLTexture::bind( int texunit )
{
	if( !m_valid ) return false;
	if( texunit >= 0 )
		glActiveTexture( GL_TEXTURE0 + texunit );
	glBindTexture( m_target, m_id );
	return CheckGLError("GLTexture::Bind()");
}

void GLTexture::unbind()
{
	glBindTexture( m_target, 0 );
}

bool GLTexture::image( GLint level, GLint internalformat,
	GLsizei width, GLsizei height, GLsizei depth, GLint border,
	GLenum format, GLenum type, void *data )
{
	if( !m_valid ) return true;
	if( !bind() ) return false;
	glTexImage3D( m_target, level, internalformat, width, height, depth, border, 
	             format, type, data );
	setSize( width, height, depth );
	return CheckGLError("GLTexture::Image()");
}

bool GLTexture::image( GLint level, GLint internalformat,
	GLsizei width, GLsizei height, GLint border,
	GLenum format, GLenum type, void *data )
{
	if( !m_valid ) return true;
	if( !bind() ) return false;
	glTexImage2D( m_target, level, internalformat, width, height, border, 
	              format, type, data);
	setSize( width, height );
	return CheckGLError("GLTexture::Image()");
}

bool GLTexture::image( GLint level, GLint internalformat,
	GLsizei width, GLint border, GLenum format, GLenum type, void *data )
{
	if( !m_valid ) return true;
	if( !bind() ) return false;
	glTexImage1D( m_target, level, internalformat, width, border, 
	              format, type, data );
	setSize( width );
	return CheckGLError("GLTexture::Image()");
}

bool GLTexture::subImage( GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
	GLsizei width, GLsizei height, GLsizei depth,
	GLenum format, GLenum type, void *data, bool doBind )
{
	if( !m_valid ) return true;	
	if( doBind && !bind() ) return false;
	glTexSubImage3D( m_target, level, xoffset, yoffset, zoffset,
		width, height, depth, format, type, data );
	return CheckGLError("GLTexture::SubImage()");
}

bool GLTexture::subImage( GLint level, GLint xoffset, GLint yoffset,
	GLsizei width, GLsizei height,
	GLenum format, GLenum type, void *data, bool doBind )
{
	if( !m_valid ) return true;	
	if( doBind && !bind() ) return false;
	glTexSubImage2D( m_target, level, xoffset, yoffset,
		width, height, format, type, data );
	return CheckGLError("GLTexture::SubImage()");
}

bool GLTexture::subImage( GLint level, GLint xoffset,
	GLsizei width, GLenum format, GLenum type, void *data, bool doBind )
{
	if( !m_valid ) return true;	
	if( doBind && !bind() ) return false;
	glTexSubImage1D( m_target, level, xoffset,
		width, format, type, data );
	return CheckGLError("GLTexture::SubImage()");
}

bool GLTexture::setParameter( GLenum pname, GLint value )
{
	if( !m_valid ) return true;	
	if( !bind() ) return false;
	glTexParameteri( m_target, pname, value );
	return CheckGLError("GLTexture::SetParameter()");
}

bool GLTexture::setParameter( GLenum pname, GLfloat value )
{
	if( !m_valid ) return true;	
	if( !bind() ) return false;
	glTexParameterf( m_target, pname, value );
	return CheckGLError("GLTexture::SetParameter()");
}

GLenum GLTexture::target() const
{
	return m_target;
}

GLuint GLTexture::name() const
{
	return m_id;
}
