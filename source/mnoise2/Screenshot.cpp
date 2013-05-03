#include "Screenshot.h"
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstring>
#include <exception>

#include "GL/GLError.h"

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

namespace Screenshot
{
std::string autoName( std::string prefix, std::string postfix )
{
	stringstream ss;
	ss << prefix << time(0) << postfix;
	return ss.str();
}

struct GLFramebufferGrab
{
	unsigned char* data;
	unsigned int size;
	unsigned int bytes_per_pixel;
	int viewport[4];

	GLFramebufferGrab( GLenum format=GL_RGB )
	: data(NULL),size(0),bytes_per_pixel(0)
	{
		glGetIntegerv( GL_VIEWPORT, viewport );
		
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
		size = viewport[2]*viewport[3]*bytes_per_pixel;
		data = new unsigned char[ size ];
		
		// read back framebuffer
		//glReadBuffer(GL_FRONT);
		glPixelStorei( GL_PACK_ALIGNMENT, 1 );
		glReadPixels( viewport[0], viewport[1], viewport[2], viewport[3],
					  format, GL_UNSIGNED_BYTE, data );
		
		#if 0
		if( GL::checkGLError("GLFramebufferGrab() glReadPixels()") )
		{
			// failed
			delete [] data; data=NULL; size=0; bytes_per_pixel=0;
		}
		#endif
	}
	
	~GLFramebufferGrab()
	{
		if( data ) delete [] data;
	}
};

void saveTGA( std::string filename )
{
	ofstream file;
	file.open( filename.c_str(), ios::out | ios::binary );
	if( file.is_open() )
	{
		int viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );
	  #if 1
		cout << "Debug: Screenshot::saveTGA(), viewport=("
		     << viewport[0] << "," << viewport[1] << "," << viewport[2] << "," << viewport[3] << ")" << endl;
	  #endif

		viewport[2]-=viewport[2]%4; // force width to be multiple of 4!

		int size = sizeof(TGA_HEADER) + viewport[2]*viewport[3]*3;
		unsigned char* data=0;
		try
		{
			data = new unsigned char[ size ];
		}
		catch( bad_alloc& )
		{
			cerr << "Error: Couldn't allocate buffer of size " << size << "!" << endl;
			return;
		}

		TGA_HEADER header = { 0,0,2, {0,0,0,0,0}, 
			{0,0},{0,0},
			{viewport[2]%256,viewport[2]/256},
			{viewport[3]%256,viewport[3]/256},
			24,0 };
		memcpy( data, &header, sizeof(TGA_HEADER) );

		//glReadBuffer(GL_FRONT);
		glPixelStorei( GL_PACK_ALIGNMENT, 1 );
		glReadPixels( viewport[0], viewport[1], viewport[2], viewport[3],
					  /*GL_RGB*/GL_BGR, GL_UNSIGNED_BYTE,
					  data + sizeof(TGA_HEADER) );

		if( GL::checkGLError( "Screenshot::saveTGA()" ) )
		{
			file.write( (char*)data, size );
		}

		file.close();
		delete [] data;
	}
	else
	{
		cerr << "Error: Unable to open " << filename << endl;
	}	
}

//______________________________________________________________________________
#ifdef SCREENSHOT_SUPPORT_PNG
#include <png.h>
//#include <csetjmp>

// based on code from http://zarb.org/~gc/html/libpng.html 
// as well as the libpng man page
bool savePNG( std::string filename )
{
	GL::checkGLError( "Screenshot::savePNG() *before*" );

	GLFramebufferGrab grab;	
	if( !grab.data )
	{
		cerr << "[Screenshot::savePNG] Grabbing OpenGL framebuffer failed!"
			 << endl;
		return false;
	}
		
	
	// image props
	int width  = grab.viewport[2], 
	    height = grab.viewport[3], 
	    bit_depth  = 8, 
	    color_type = PNG_COLOR_TYPE_RGB;
		int channels = 3;
	/* PNG_COLOR_TYPE_GRAY (bit depths 1, 2, 4, 8, 16) 
	   PNG_COLOR_TYPE_GRAY_ALPHA (bit depths 8, 16) 
	   PNG_COLOR_TYPE_PALETTE (bit depths 1, 2, 4, 8) 
	   PNG_COLOR_TYPE_RGB (bit_depths 8, 16) 
	   PNG_COLOR_TYPE_RGB_ALPHA (bit_depths 8, 16)
	*/
	bool retval = false;
	png_structp png_ptr;
	png_infop info_ptr;
	
	// image data
//	png_byte *row_pointers[] = new png_byte*[height];

	/* create file */
	FILE *fp = fopen( filename.c_str(), "wb");
	if (!fp) {
		cerr << "[Screenshot::savePNG] File " << filename 
			 << " could not be opened for writing" << endl;
		return false;
	};

	/* initialize stuff */
	png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	
	if (!png_ptr) {
		cerr << "[Screenshot::savePNG] png_create_write_struct failed" << endl;
		goto abort_;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		cerr << "[Screenshot::savePNG] png_create_info_struct failed" << endl;
		goto abort_;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		cerr << "[Screenshot::savePNG] Error during init_io" << endl;
		goto abort_;
	}

	png_init_io( png_ptr, fp );

	/* set the zlib compression level */ 
	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	/* set other zlib parameters */ 
	png_set_compression_mem_level(png_ptr, 8); 
	png_set_compression_strategy(png_ptr, Z_DEFAULT_STRATEGY); 
	png_set_compression_window_bits(png_ptr, 15); 
	png_set_compression_method(png_ptr, 8);
	
	
	/* write header */
	if (setjmp(png_jmpbuf(png_ptr))) {
		cerr << "[Screenshot::savePNG] Error during writing header" << endl;
		goto abort_;
	}

	png_set_IHDR( png_ptr, info_ptr, 
			width, height,
			bit_depth, 
			color_type, 
			PNG_INTERLACE_NONE, 
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );
			//PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );
	
	/*
	   png_set_text(png_ptr, info_ptr, text_ptr, num_text); 
	   text_ptr - array of png_text holding image comments 
	   text_ptr[i]->key - keyword for comment. 
	   text_ptr[i]->text - text comments for current keyword. 
	   text_ptr[i]->compression - type of compression used on "text" 
	     PNG_TEXT_COMPRESSION_NONE or PNG_TEXT_COMPRESSION_zTXt 
	   num_text - number of comments in text_ptr	
	*/
	
	/*
	   In PNG files, the alpha channel in an image is the level of opacity. If 
	   your data is supplied as a level of transparency, you can invert the 
	   alpha channel before you write it, so that 0 is fully transparent and 255 
	   (in 8-bit or paletted images) or 65535 (in 16-bit images) is fully 
	   opaque, with

	   png_set_invert_alpha(png_ptr);	
	*/	

	png_write_info(png_ptr, info_ptr);


	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr))) {
		cerr << "[Screenshot::savePNG] Error during writing bytes" << endl;
		goto abort_;
	}
#if 0
	png_write_image(png_ptr, row_pointers);
	/*
	If you are just writing one row at a time, you can do this with row_pointers:
		png_bytep row_pointer = row;	
		png_write_row(png_ptr, &row_pointer);
	*/
#else
	// not working, only to produce glitch art 
	//for( int y=0; y < height; ++y ) 
	for( int y=height-1; y >= 0; --y )
	{
		png_byte* row_pointer = (png_byte*)(&grab.data[width*y*channels]);
		png_write_row(png_ptr, row_pointer);
	}
#endif


	/* end write */
	if (setjmp(png_jmpbuf(png_ptr))) {
		cerr << "[Screenshot::savePNG] Error during end of write" << endl;
		goto abort_;
	}

	png_write_end(png_ptr, NULL);

	retval = true;
abort_:
/*
When you are done, you can free all memory used by libpng like this:
png_destroy_write_struct(&png_ptr, &info_ptr);
You must free any data you allocated for info_ptr, such as comments, palette, or
histogram, before the call to png_destroy_write_struct();
*/	
	/* cleanup heap allocation */
	//for (int y=0; y<height; y++)
	//	free(row_pointers[y]);
	//free(row_pointers);
	////delete [] row_pointers;

	fclose(fp);	
	return retval;
}

#endif

//______________________________________________________________________________
#ifdef SCREENSHOT_SUPPORT_SDL
#include <SDL/SDL.h>

void saveBMP( std::string filename )
{
	int viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
  #if 1
	cout << "Debug: Screenshot::saveBMP(), viewport=("
		 << viewport[0] << "," << viewport[1] << "," << viewport[2] << "," << viewport[3] << ")" << endl;
  #endif
	viewport[2]-=viewport[2]%4; // force width to be multiple of 4!
	
	SDL_Surface *screen = SDL_CreateRGBSurface( SDL_SWSURFACE,
		viewport[2],viewport[3], 24, 0x0000FF, 0x00FF00, 0xFF0000, 0 );
	if( !screen )
	{
		cerr << "Error: Couldn't create SDL_Surface! (Screenshot::saveBMP())" << endl;
		return;
	}
	
	unsigned char* data=0;
	try
	{
		data = new unsigned char[ viewport[2]*viewport[3]*3 ];
	}
	catch( bad_alloc& )
	{
		cerr << "Error: Couldn't allocate buffer of size " << viewport[2]*viewport[3]*3 << "!" << endl;
		return;
	}

	glReadBuffer(GL_FRONT);
	glReadPixels( viewport[0], viewport[1], viewport[2], viewport[3],
				  GL_RGB, GL_UNSIGNED_BYTE, data );

	if( !GLUtils::checkGLErrors( "Screenshot::saveBMP()" ) )
	{
		SDL_FreeSurface( screen );
		delete [] data;		
		return;
	}

#if 1
	// copy&flip
	for( int i=0; i < viewport[3]; ++i )
		memcpy( (unsigned char*)screen->pixels + (viewport[3]-i-1)*viewport[2]*3,
				data + i*viewport[2]*3,
				viewport[2]*3 );
#else
	memcpy( (unsigned char*)screen->pixels, data, viewport[2]*viewport[3]*3 );
#endif
	
	if( SDL_SaveBMP( screen, filename.c_str() ) < 0 )
	{
		cerr << "Error: Couldn't write bitmap '" << filename << "'!" << endl;
	}
	SDL_FreeSurface( screen );

	delete [] data; data=0;
}
#endif // SCREENSHOT_SUPPORT_SDL


} // namespace Screenshot

