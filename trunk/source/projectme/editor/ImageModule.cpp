#include "ImageModule.h"
#include <glutils/GLError.h>
#ifdef GL_NAMESPACE
using GL::checkGLError;
#endif
#include <QImage>
#include <QColor>
#include <cmath>
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
bool ImageModule::createImage( int width, int height )
{
	if( width<0 || height<0 ) return false;

	// Allocate RGBA image data
	unsigned char* cbuf = new unsigned char[ width * height * 4 ];
	
	// Store data
	if( m_data ) delete [] m_data;
	m_data   = cbuf;
	m_width  = width;
	m_height = height;
	m_filename = std::string("");
	
	m_dirty = true; // Upload texture in next render() call		
	return true;
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
bool ImageModule::saveImage( const char* filename ) const
{
    if( !m_data ) return false;

    // Since RGBA is not supported by Qt we explicitly copy into RGB888
    // And we have to invert y!
    unsigned char* buffer = new unsigned char[ m_width*m_height * 3 ];
    {
        unsigned char* pdst = buffer;
        for( int y=0; y < m_height; y++ )
        {
            const unsigned char* psrc = m_data + m_width*(m_height-y-1)*4;
            for( int x=0; x < m_width; x++ )
            {
                (*pdst++) = (*psrc++); // R
                (*pdst++) = (*psrc++); // G
                (*pdst++) = (*psrc++); // B
                psrc++; // A
            }
        }
    }

    // Save via QImage
    // (own scope to guarantee buffer stays valid during lifetime of img)
    bool success = false;
    {
        QImage img( buffer, m_width, m_height, QImage::Format_RGB888 );
        success = img.save( filename );
    }

    delete [] buffer;
    return success;
}

//----------------------------------------------------------------------------
void ImageModule::fillImage( unsigned char R, unsigned char G, unsigned char B, unsigned char A )
{
	if( !m_data ) return;
	unsigned ofs = 0;
	for( int y=0; y < m_height; y++ )
		for( int x=0; x < m_width; x++ )
		{
			m_data[ofs++] = R;
			m_data[ofs++] = G;
			m_data[ofs++] = B;
			m_data[ofs++] = A;
		}

	m_dirty = true; // Upload texture in next render() call
}

//----------------------------------------------------------------------------

float smoothstep( float x, float e0, float e1 )
{
    float t = (x-e0)/(e1-e0);
    if( t>1.f ) t=1.f;
    if( t<0.f ) t=0.f;
    return t*t*(3.f-2.f*t);
}

void ImageModule::paint( int x0, int y0, unsigned char R, unsigned char G, unsigned char B, unsigned char A, int radius )
{
	if( radius <= 0 ) return;

    // WORKAROUND: Derive blend mode from R value
    bool blend = R > 128;

    float r0 = .25f*radius, // inner edge (where smoothing starts)
          r1 = 1.f*radius; // outer edge (where smoothing approaches 0)

	int r = radius;
	for( int y = y0-r; y < y0+r; ++y )
	{
		if( y < 0 || y >= m_height )
			continue;

		unsigned ofs = (y*m_width + x0-r)*4;
		for( int x = x0-r; x < x0+r; ++x, ofs+=4 )
		{
			if( x < 0 || x >= m_width )
				continue;

            // Smooth edge
            float tt = (float)( (x-x0)*(x-x0) + (y-y0)*(y-y0) );
            float alpha = 1.f - smoothstep( tt, r0*r0, r1*r1 );

            unsigned char aR = (unsigned char)std::floor(alpha*R),
                          aG = (unsigned char)std::floor(alpha*G),
                          aB = (unsigned char)std::floor(alpha*B);

            if( tt <= r*r )
            {
                if( blend )
                {
                    // max
                    m_data[ofs+0] = std::max( aR, m_data[ofs+0] );
                    m_data[ofs+1] = std::max( aG, m_data[ofs+1] );
                    m_data[ofs+2] = std::max( aB, m_data[ofs+2] );
                }
                else
                {
                    // replace
                    m_data[ofs+0] = aR;
                    m_data[ofs+1] = aG;
                    m_data[ofs+2] = aB;
                }
                m_data[ofs+3] = A;
			}


			// TODO: Implement smoothing
		}
	}

	m_dirty = true; // Upload texture in next render() call
}

//----------------------------------------------------------------------------
void ImageModule::updateTexture()
{
	if( !m_initialized && !init() )
		return;
	
	GLint internalFormat = GL_RGBA8;
	m_target.image( 0, internalFormat, m_width,m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 
		(void*)m_data );
}

//----------------------------------------------------------------------------
bool ImageModule::init()
{
	// Create texture
	if( !m_target.create(GL_TEXTURE_2D) )
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
	m_target.destroy();
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
