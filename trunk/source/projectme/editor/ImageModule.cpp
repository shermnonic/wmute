#include "ImageModule.h"
#include <glutils/GLError.h>
#ifdef GL_NAMESPACE
using GL::checkGLError;
#endif
#include <QImage>
#include <QColor>
#include <iostream>
using std::cerr;
using std::endl;

//----------------------------------------------------------------------------
//  Factory registration
//----------------------------------------------------------------------------
#include "ModuleFactory.h"
MODULEFACTORY_REGISTER( ImageModule, "ImageModule" )

//----------------------------------------------------------------------------
ImageModule::ImageModule()
: ModuleRenderer( "ImageModule" ),
  m_initialized( false ),
  m_width(0),
  m_height(0),
  m_data( NULL ),
  m_dirty(false)
{
}

//----------------------------------------------------------------------------
bool ImageModule::loadImage( const char* filename )
{
	// Load image from disk
	QImage* img = new QImage;
	if( !(img->load(QString( filename ))) )
	{
		delete img;
		return false;
	}
	
	// Convert QImage to scalar RGBA float buffer
	int width  = img->width(),
	    height = img->height();
	const float threshold = 128.f;
	unsigned char* cbuf = new unsigned char[ width * height * 4 ];
	unsigned ofs = 0;
	for( int y=0; y < height; y++ )
		for( int x=0; x < width; x++ )
		{
			// Just consider R channel for now
			QRgb color = img->pixel(x,height-y-1); // Flip upside-down
			cbuf[ofs++] = qRed  (color);
			cbuf[ofs++] = qGreen(color);
			cbuf[ofs++] = qBlue (color);
			cbuf[ofs++] = qAlpha(color);
		}
		
	delete img;

	// Store data
	if( m_data ) delete [] m_data;
	m_data   = cbuf;
	m_width  = width;
	m_height = height;
	m_filename = std::string(filename);
		
	m_dirty = true; // Upload texture in next render() call		
	return true;
}

//----------------------------------------------------------------------------
void ImageModule::updateTexture()
{
	if( !m_initialized && !init() )
		return;
	
	GLint internalFormat = GL_RGBA8;
	m_target.Image( 0, internalFormat, m_width,m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 
		(void*)m_data );
}

//----------------------------------------------------------------------------
bool ImageModule::init()
{
	// Create texture
	if( !m_target.Create(GL_TEXTURE_2D) )
	{
		cerr << "ImageModule::init() : Couldn't create 2D textures!" << endl;
		return false;
	}
	
	m_initialized = checkGLError( "ShaderModule::init() : GL error at exit!" );
	return m_initialized;	
}

//----------------------------------------------------------------------------
void ImageModule::destroy()
{
	m_target.Destroy();
	delete [] m_data; m_data = NULL;
}

//----------------------------------------------------------------------------
void ImageModule::render()
{
	// Initialized on first render() call; by then we should have a valid 
	// OpenGL context.	
	if( !m_initialized && !init() ) return;	
	
	if( m_dirty )
	{
		updateTexture();
		m_dirty = false;
	}
}

//-----------------------------------------------------------------------------
Serializable::PropertyTree& ImageModule::serialize() const
{
	static Serializable::PropertyTree cache;
	cache = ModuleRenderer::serialize();
	
	cache.put("ImageModule.Texture.Filename",m_filename);

	return cache;
}

//-----------------------------------------------------------------------------
void ImageModule::deserialize( Serializable::PropertyTree& pt )
{
	ModuleRenderer::deserialize( pt );

	std::string filename = pt.get( "ImageModule.Texture.Filename", "" );
	if( !filename.empty() )
	{
		if( !loadImage( filename.c_str() ) )
		{
			cerr << "ImageModule::deserialize : "
				"Could not load texture \"" << filename << "\"!" << endl;
		}
	}
	else
	{
		cerr << "ImageModule::deserialize : "
			"No texture filename given!" << endl;
	}
}
