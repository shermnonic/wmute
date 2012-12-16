#ifdef ENABLE_PNG_SUPPORT

#include "ImagePNG.h"
#include <png.h>
#include <iostream>
#include <cstdlib>

using namespace std;

// Factory registration
namespace {
	Image* create_ImagePNG()
	{
		return new ImagePNG;
	}

	const bool registered = ImageFactory::ref().register_format(
		"PNG", create_ImagePNG );
}

/**
	Loading code adapted from example at http://zarb.org/~gc/html/libpng.html
*/
bool ImagePNG::load( const char* file_name )
{
	unsigned char* buffer=0;  // linear buffer (width*height*channels)
	int channels;  // number of color channels

	//--------------------------------------------------------------------------
	// load png 
	// code adapted from example at http://zarb.org/~gc/html/libpng.html

	int y;
	int width, height;
	png_byte color_type;
	png_byte bit_depth;

	png_structp png_ptr;
	png_infop info_ptr;
	int number_of_passes;
	png_bytep * row_pointers;

	png_byte header[8];	// 8 is the maximum size that can be checked

	/* open file and test for it being a png */
	FILE *fp = fopen(file_name, "rb");
	if (!fp) {
		cerr << "[read_png_file] File " << file_name 
		     << " could not be opened for reading" << endl;
		return false;
	}
	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8)) {
		cerr << "[read_png_file] File " << file_name 
		     << " is not recognized as a PNG file" << endl;
		return false;
	}

	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	
	if (!png_ptr) {
		cerr << "[read_png_file] png_create_read_struct failed" << endl;
		return false;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		cerr << "[read_png_file] png_create_info_struct failed" << endl;
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		cerr << "[read_png_file] Error during init_io" << endl;
		return false;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	width      = info_ptr->width;
	height     = info_ptr->height;
	color_type = info_ptr->color_type;
	bit_depth  = info_ptr->bit_depth;

	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	/* read file */
	if (setjmp(png_jmpbuf(png_ptr))) {
		cerr << "[read_png_file] Error during read_image" << endl;
		return false;
	}

	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	for (y=0; y<height; y++)
		row_pointers[y] = (png_byte*) malloc(info_ptr->rowbytes);

	png_read_image(png_ptr, row_pointers);

    fclose(fp);
	
	//--------------------------------------------------------------------------
	// copy into linear buffer

	// image type
	if( bit_depth != 8 ) {
		cerr << "Mismatching image format,only 8bit/channel supported!" << endl;
		goto ImagePNG_load_cleanup;
	}
	switch( color_type ) {
        case PNG_COLOR_TYPE_GRAY:       channels=1; break;
		case PNG_COLOR_TYPE_GRAY_ALPHA: channels=1; break;
        case PNG_COLOR_TYPE_RGB:        channels=3; break;
		case PNG_COLOR_TYPE_RGB_ALPHA:  channels=4; break;

		default:
        case PNG_COLOR_TYPE_PALETTE:
//      case PNG_COLOR_MASK_PALETTE:
//      case PNG_COLOR_MASK_COLOR:
//      case PNG_COLOR_MASK_ALPHA:
			cerr << "Unsupported PNG color type!" << endl;
			goto ImagePNG_load_cleanup;		
	}

	// allocate buffer
	buffer = new unsigned char[width*height*channels];

	// copy PNG rows into linear buffer
/*
cout << "channels = " << channels << endl
	<< "bit-depth = " << bit_depth << endl
	<< "rowbytes = " << info_ptr->rowbytes << endl
	<< "width = " << width << endl
	<< "height = " << height << endl;
*/
	for (y=0; y<height; ++y) {
		png_byte* row = row_pointers[y];
  #if 1
		memcpy( &buffer[y*width*channels], row, width*channels );
  #else
		for (int x=0; x<width; ++x) {
			png_byte* ptr = &(row[x*channels]);
			for (int c=0; c < channels; ++c)
				buffer[y*width*channels+x*channels+c] = ptr[c];
		}
  #endif
	}

	//--------------------------------------------------------------------------
	// set Image data
	set_image( (unsigned char*)buffer, width, height, channels );


	//--------------------------------------------------------------------------
	// cleanup
ImagePNG_load_cleanup:
    /* cleanup heap allocation */
	for (y=0; y<height; y++)
		free(row_pointers[y]);
	free(row_pointers);

	return buffer!=NULL;
}

#endif // ENABLE_PNG_SUPPORT
