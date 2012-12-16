#ifdef ENABLE_PNG_SUPPORT

#ifndef IMAGEPNG_H
#define IMAGEPNG_H

#include "Image.h"

class ImagePNG : public Image
{
public:
	bool load( const char* fname );
};

#endif // IMAGEPNG_H

#endif // ENABLE_PNG_SUPPORT