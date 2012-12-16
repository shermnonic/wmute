#include "Image.h"
#include <algorithm> // for: transform()
#include <cctype>    // for: toupper()

std::string Image::get_extension( const char* fname )
{
	using namespace std;

	string fn( fname );

	// extension (chars after last dot) in uppercase
	string ext = fn.substr( fn.find_last_of('.')+1, string::npos );
	//transform( ext.begin(), ext.end(), ext.begin(), toupper );
	for( string::iterator it=ext.begin(); it!= ext.end(); ++it )
		(*it) = toupper(*it);

	return ext;
}

//------------------------------------------------------------------------------
//	ImageFactory
//------------------------------------------------------------------------------

Image* ImageFactory::create_image( std::string ext )
{
	CallbackMap::const_iterator it = m_callbacks.find( ext );
	if( it == m_callbacks.end() )
	{
		// not found
		return NULL;
	}

	// invoke creation function
	return (it->second)();
}

bool ImageFactory::register_format( std::string ext, CreateImageCallback cb )
{
	return m_callbacks.insert( CallbackMap::value_type( ext, cb ) ).second;
}

//------------------------------------------------------------------------------
// 	ImageTGA
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#define IMAGETGA_NO_VFLIP
//#define IMAGETGA_BGR

//---- Some helper functions ----
namespace {
	unsigned short readWord(FILE *f)
	{
	  unsigned short a;
	  fread(&a,1,2,f);

	// if you are using a machine with big-endian code (eg. macintosh) swap hi:lo byte of "a"
	#ifdef BIGENDIAN
	  a= ((a & 255)<<8) + ((a>>8) & 255);
	#endif

	  return a;
	}

	unsigned char readByte(FILE *f)
	{
	  unsigned char a;
	  fread(&a,1,1,f);
	  return a;
	}
}

bool ImageTGA::load( const char* fname )
{
	int           i,j,c,x,y,bpp,depth;
	unsigned char a=255,r,g,b;
	unsigned int  *dst;

	#if defined(_MSC_VER) && _MSC_VER >= 1400
	FILE *f = NULL;
	fopen_s(&f, fname, "rb");
	#else
	FILE *f = fopen(fname, "rb");
	#endif
	if (!f) return false;

	for (i=0;i<12;i++) c= readByte(f);
	x= readWord(f);
	y= readWord(f);
	bpp=readByte(f);
	c= readByte(f);

	if (bpp!=24 && bpp!=32) return false; // bitdpeth mismatch
	if (x<=0 || y<=0) return false;       // size-mismatch

	depth = bpp / 8; // TODO: adjust below hardcoded 4-channels to real depth

	// data will be stored as 32bit ARGB
	dst= (unsigned int*)malloc(x*y*4);
	for (j=0;j<y;j++)
	{
		for (i=0;i<x;i++)
		{
		  #ifdef IMAGETGA_BGR
			b= readByte(f);
			g= readByte(f);
			r= readByte(f);
		  #else
			r= readByte(f);
			g= readByte(f);
			b= readByte(f);
		  #endif
			if (bpp==32) a= readByte(f);

		  #ifdef IMAGETGA_NO_VFLIP
			dst[j*x+i] = (a<<24)+(r<<16)+(g<<8)+b; // was: *dst++= ...
		  #else
			// flip vertically
			dst[(y-j-1)*x+i] = (a<<24)+(r<<16)+(g<<8)+b;
		  #endif
		}
	}

	fclose(f);

	// set Image data
	set_image( (unsigned char*)dst, x, y, 4 );

	return true;
}

// Factory registration
namespace {
	Image* create_ImageTGA()
	{
		return new ImageTGA;
	}

	const bool registered = ImageFactory::ref().register_format(
		"TGA", create_ImageTGA );
}

