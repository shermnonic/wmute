#include "PotentialFromImageModule.h"
#include "DistanceTransformFelzenszwalb.h"
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
MODULEFACTORY_REGISTER( PotentialFromImageModule, "PotentialFromImageModule" )

//----------------------------------------------------------------------------
PotentialFromImageModule::PotentialFromImageModule()
: ModuleRenderer( "PotentialFromImageModule" ),
  m_initialized( false ),
  m_width(0),
  m_height(0),
  m_data( NULL ),
  m_dirty(false)
{
}

//----------------------------------------------------------------------------
bool PotentialFromImageModule::loadImage( const char* filename )
{
	// Load image from disk
	QImage* img = new QImage;
	if( !(img->load(QString( filename ))) )
	{
		delete img;
		return false;
	}
	
	// Convert QImage to scalar float buffer
	int width  = img->width(),
	    height = img->height();
	const float threshold = 128.f;
	unsigned char* cbuf = new unsigned char[ width * height ];
	unsigned ofs = 0;
	for( int y=0; y < height; y++ )
		for( int x=0; x < width; x++, ofs++ )
		{
			// Just consider R channel for now
			QRgb color = img->pixel(x,y);						
			float scalar = qRed(color);
			// Threshold
			cbuf[ofs] = (scalar > threshold) ? 1 : 0;
		}
	
	// Apply distance transform to float buffer in-place
	float* buf = DistanceTransformFelzenszwalb::dt<unsigned char>( cbuf, width, height, 1 );
		
	// Compute gradient
	float scale = 1.0 / (float)std::max(width,height);
	float* grad = new float[ width * height * 4 ]; // RGB format
	memset( (void*)grad, width*height*2*sizeof(float), 0 ); // Zero border / z
	for( int y=1; y < (height-1); y++ )
		for( int x=1; x < (width-1); x++ )
		{
			// Finite differences
			ofs = y*width+x;
			unsigned stride = width;			
		  #if 1
			grad[4*ofs+0] = scale*0.5*(buf[ofs+1] - buf[ofs-1]); // dx
			grad[4*ofs+1] = scale*0.5*(buf[ofs+stride] - buf[ofs-stride]); // dy
			grad[4*ofs+2] = 0.0; // z
		  #else
			grad[4*ofs+0] = grad[4*ofs+1] = grad[4*ofs+2] = buf[ofs];
		  #endif
			grad[4*ofs+3] = 1.0; // alpha
		}
	
	// Free memory
	delete [] buf; buf=NULL;
	delete img;
		
	// Store data
	if( m_data ) delete [] m_data;
	m_data   = grad;
	m_width  = width;
	m_height = height;
	m_filename = std::string(filename);
		
	m_dirty = true; // Upload texture in next render() call
		
	return true;
}

//----------------------------------------------------------------------------
void PotentialFromImageModule::updateTexture()
{
	if( !m_initialized && !init() )
		return;
	
	GLint internalFormat = GL_RGBA32F;
	m_target.Image( 0, internalFormat, m_width,m_height, 0, GL_RGBA, GL_FLOAT, 
		(void*)m_data );
}

//----------------------------------------------------------------------------
bool PotentialFromImageModule::init()
{
	// Create texture
	if( !m_target.Create(GL_TEXTURE_2D) )
	{
		cerr << "PotentialFromImageModule::init() : Couldn't create 2D textures!" << endl;
		return false;
	}
	
	m_initialized = checkGLError( "ShaderModule::init() : GL error at exit!" );
	return m_initialized;	
}

//----------------------------------------------------------------------------
void PotentialFromImageModule::destroy()
{
	m_target.Destroy();
	delete [] m_data; m_data = NULL;
}

//----------------------------------------------------------------------------
void PotentialFromImageModule::render()
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
Serializable::PropertyTree& PotentialFromImageModule::serialize() const
{
	static Serializable::PropertyTree cache;
	cache = Super::serialize();
	
	cache.put("PotentialFromImageModule.Texture.Filename",m_filename);

	return cache;
}

//-----------------------------------------------------------------------------
void PotentialFromImageModule::deserialize( Serializable::PropertyTree& pt )
{
	Super::deserialize( pt );

	std::string filename = pt.get( "PotentialFromImageModule.Texture.Filename", "" );
	if( !filename.empty() )
	{
		if( !loadImage( filename.c_str() ) )
		{
			cerr << "PotentialFromImageModule::deserialize : "
				"Could not load texture \"" << filename << "\"!" << endl;
		}
	}
	else
	{
		cerr << "PotentialFromImageModule::deserialize : "
			"No texture filename given!" << endl;
	}
}
