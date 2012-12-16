// Max Hermann, March 21, 2010
#ifndef IMAGE_H
#define IMAGE_H

#include <string>
#include <map>

//------------------------------------------------------------------------------
// 	Image
//------------------------------------------------------------------------------

/// 2D image restricted to 1,3 or 4 channels and 8 bits per channel
///
/// Use \a ImageFactory::ref().create_image(ext) to get a concrete \a Image 
/// object for the filetype with the uppercase extension string \a ext.
///
/// The "TGA" format is supported by default (no additional dependencies).
///
/// \TODO
///	  - provide some error handling functionality (at least cout some info)
///   - add ImagePNG.h (see libpng example http://zarb.org/~gc/html/libpng.html)
///   - use boost::shared_ptr<> for image data?
class Image
{
public:
	Image(): m_buffer(0), m_width(0),m_height(0),m_channels(0) 
	{}

	~Image()
	{
		if( m_buffer ) delete [] m_buffer;
	}

	/// Return uppercase extension string 
	/// This can be used as parameter to ImageFactory::create_image().
	static std::string get_extension( const char* fname );

	/// Load image
	virtual bool load( const char* fname )=0;

	/*
	void create_from_buffer( unsigned char* buffer, int width,int height, 
	                         int channels )
	{
		set_image( buffer, width, height, channels );
	}
	*/	

	/// Return pointer to raw data
	unsigned char* ptr() { return m_buffer; }

	int width ()   const { return m_width;  }
	int height()   const { return m_height; }	
	int channels() const { return m_channels; }

protected:
	/// Internal convenience function for derived classes
	void set_image( unsigned char* buffer, int width,int height, int channels )
	{
		m_buffer = buffer;
		m_width  = width;
		m_height = height;
		m_channels = channels;
	}

private:
	unsigned char* m_buffer;
	int m_width, m_height;
	int m_channels;
};


//------------------------------------------------------------------------------
//	ImageFactory
//------------------------------------------------------------------------------
class ImageFactory
{
public:
	/// Return singleton instance
	static ImageFactory& ref()
	{
		static ImageFactory singleton;
		return singleton;
	}

	typedef Image* (*CreateImageCallback)();

	/// Return image class for given uppercase extension string (e.g. "TGA")
	/// Returns NULL if extension is not supported.
	Image* create_image( std::string ext );

	/// Register new image class for specific extension
	/// Returns true if registration was succesfull
	/// Identifier is uppercase extension string (e.g. "TGA")
	bool register_format( std::string ext, CreateImageCallback cb );

private:
	typedef std::map<std::string, CreateImageCallback> CallbackMap;
	CallbackMap m_callbacks;

	// make c'tors private for singleton
	ImageFactory() {}
	~ImageFactory() {}
	ImageFactory( const ImageFactory& ) {}
	ImageFactory& operator = ( const ImageFactory& ) { return *this; }
};


//------------------------------------------------------------------------------
// 	ImageTGA
//------------------------------------------------------------------------------
class ImageTGA : public Image
{
public:
	/// \a Image implementation, load TGA image (restricted to simple TGA types)
	/// Always returns RGBA data!
	bool load( const char* fname );
private:
};

#endif

