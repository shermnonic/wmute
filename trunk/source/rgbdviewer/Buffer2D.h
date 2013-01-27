#ifndef BUFFER2D_H
#define BUFFER2D_H
#include <vector>

/// Simple 2D buffer abstraction mapping (x,y) to #channels many T values
template <class T>
struct Buffer2D
{
	Buffer2D(): width(0),height(0), channels(0) {}
	int width, height, channels;
	std::vector<T> data;
	void update( int w, int h, int ch )
	{
		// If size has not changed and data is allocated, leave it as it is
		if( width==w && height==h && channels==ch && data.size()==w*h*ch )
			return;
		// Else we have to allocate data anew
		data.resize( w*h*ch );
		width    = w;
		height   = h;
		channels = ch;
	}
	T* ptr( int x, int y )
	{
		return &data[channels*(y*width + x)];
	}
};

#endif // BUFFER2D_H
