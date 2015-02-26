#include "Screenshot2.h"
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstring> // for memcpy()
#include <exception>

//-----------------------------------------------------------------------------
//  Helper functions
//-----------------------------------------------------------------------------
std::string autoName( std::string prefix, std::string postfix )
{
	std::stringstream ss;
	ss << prefix << time(0) << postfix;
	return ss.str();
}

//-----------------------------------------------------------------------------
//  GLGrabFramebuffer
//-----------------------------------------------------------------------------
struct GLGrabFramebuffer
{
	unsigned char* data;
	unsigned int size;
	unsigned int bytes_per_pixel;
	int viewport[4];

	enum Options { None = 0, ForceWidthMultiple4 = 1 };
	
	GLGrabFramebuffer( GLenum format=GL_RGB, int opts=None, int headerSize=0 )
	: data(NULL),size(0),bytes_per_pixel(0)
	{
		using namespace std;

		glGetIntegerv( GL_VIEWPORT, viewport );
		
		if( opts & ForceWidthMultiple4 )
		{
			// force width to be multiple of 4 (e.g. required for TGA format)
			viewport[2] -= viewport[2]%4;
		}
		
		// allocate buffer
		switch( format ) 
		{
			case GL_DEPTH_COMPONENT:
			case GL_RED:
			case GL_GREEN:
			case GL_BLUE:
			case GL_LUMINANCE: 
			case GL_ALPHA:                  bytes_per_pixel = 1;	break;
			case GL_LUMINANCE_ALPHA:        bytes_per_pixel = 2;	break;
			case GL_RGB:
			case GL_BGR:                    bytes_per_pixel = 3;	break;
			case GL_RGBA:
			case GL_BGRA:                   bytes_per_pixel = 4;	break;
		}
		size = viewport[2]*viewport[3]*bytes_per_pixel + headerSize;
		try
		{
			data = new unsigned char[ size ];
		}
		catch( bad_alloc& )
		{
			cerr << "Error: Couldn't allocate buffer of size " << size << " in GLGrabFramebuffer()!" << endl;
			return;
		}
		
		
		// read back framebuffer
		glPixelStorei( GL_PACK_ALIGNMENT, 1 );
		glReadPixels( viewport[0], viewport[1], viewport[2], viewport[3],
					  format, GL_UNSIGNED_BYTE, data + headerSize );
	}
	
	int width() const { return viewport[2]; };
	int height() const { return viewport[3]; };
	
	~GLGrabFramebuffer()
	{
		if( data ) delete [] data;
	}
};

//-----------------------------------------------------------------------------
//  grabTGA()
//-----------------------------------------------------------------------------
void grabTGA( std::string filename )
{
	using namespace std;
	
	struct TGA_HEADER
	{
		unsigned char lengthIdentificationField;  // 0 to omit
		unsigned char colorMapType;               // 0 for no color map included
		unsigned char imageTypeCode;              // 2 for unmapped rgb(a), 3 for b+w
		unsigned char colorMapSpecification[5];   // ignored
		unsigned char xOrigin[2];
		unsigned char yOrigin[2];
		unsigned char width [2];
		unsigned char height[2];
		unsigned char pixelSize;           // 24 or 32
		unsigned char imageDescriptorByte; // 0
	};	
	
	ofstream file;
	file.open( filename.c_str(), ios::out | ios::binary );
	if( file.is_open() )
	{
		GLGrabFramebuffer grabfb( GL_BGR, 
		                          GLGrabFramebuffer::ForceWidthMultiple4,
								  sizeof(TGA_HEADER) );
		if( !grabfb.data )
		{
			cerr << "Unexpected error in grabTGA()!" << endl;
			return;
		}

		TGA_HEADER header = { 0,0,2, {0,0,0,0,0}, 
			{0,0},{0,0},
			{grabfb.width ()%256,grabfb.width ()/256},
			{grabfb.height()%256,grabfb.height()/256},
			24,0 };
		memcpy( grabfb.data, &header, sizeof(TGA_HEADER) );

		file.write( (char*)grabfb.data, grabfb.size );
		file.close();
	}
	else
	{
		cerr << "Error: Unable to open " << filename << " in saveTGA()!" << endl;
	}	
}

//-----------------------------------------------------------------------------
//  Screenshot2
//-----------------------------------------------------------------------------
Screenshot2::Screenshot2()
	: m_initialized(false),
	  m_width(-1), m_height(-1), m_prefix("screenshot-"), 
      m_texid(-1), m_texunit(0), m_fbo(-1)
{}

void Screenshot2::setup( int width, int height, std::string prefix )
{
	if( m_initialized ) destroy();
	
	m_width  = width;
	m_height = height;
	m_prefix = prefix;
	
	// create texture
	glGenTextures( 1, &m_texid );
	glActiveTexture( GL_TEXTURE0 + m_texunit );
	glBindTexture( GL_TEXTURE_2D, m_texid );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width,height, 0, GL_RGB, GL_FLOAT,
	              NULL );
	
	// create frame buffer object
	glGenFramebuffers( 1, &m_fbo );
	glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );	
	// attach texture	
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
	                        GL_TEXTURE_2D, m_texid, 0 );

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
							
	m_initialized = true;
}

void Screenshot2::destroy()
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glDeleteTextures( 1, &m_texid );
	glDeleteFramebuffers( 1, &m_fbo );
	m_initialized = false;	
}

void Screenshot2::begin()
{
	glPushAttrib( GL_ALL_ATTRIB_BITS ); // push also client attribs ?

	// bind texture
	glActiveTexture( GL_TEXTURE0 + m_texunit );
	glBindTexture( GL_TEXTURE_2D, m_texid );	

	// bind fbo
	glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );	
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
	                        GL_TEXTURE_2D, m_texid, 0 );
	glDrawBuffer( GL_COLOR_ATTACHMENT0 );	
}

void Screenshot2::end()
{
	using namespace std;

	string filename = autoName(m_prefix,".tga");
	m_lastFilename = filename;

	// save screenshot
	grabTGA( filename );

	cout << "Screenshot saved to " << filename << "." << endl;
	
	// cleanup
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glPopAttrib();
}

void Screenshot2::render( void (*renderFunc)(), void (*reshapeFunc)(int,int) )
{
	if( !m_initialized )
	{
		int viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );
		setup( viewport[2], viewport[3], m_prefix );
	}
	
	begin();
	
	// render to texture
	glViewport( 0, 0, m_width, m_height );
	if( reshapeFunc ) 
		reshapeFunc( m_width, m_height );
	renderFunc();
	
	end();
}
