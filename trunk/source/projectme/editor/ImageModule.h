#ifndef IMAGEMODULE_H
#define IMAGEMODULE_H

#include <glutils/GLTexture.h>
#ifdef GL_NAMESPACE
using GL::GLTexture;
#endif

#include "ModuleRenderer.h"

#include <string>

// Forwards
class QImage;

/**
	\class ImageModule
*/
class ImageModule : public ModuleRenderer
{
public:
	ImageModule();

	bool loadImage( const char* filename );
	bool createImage( int width, int height );
    bool saveImage( const char* filename ) const;

	///@name Image operations
	///@{
	void fillImage( unsigned char R, unsigned char G, unsigned char B, unsigned char A );
	void paint( int x0, int y0, unsigned char R, unsigned char G, unsigned char B, unsigned char A, int radius );
	///@}

	///@name ModuleRenderer implementation
	///@{
	void render();
	int  target() const { return m_target.name(); }
	void destroy();
	void touch() {}
	///@}
	
	/// @name Serialization
	///@{
	PropertyTree& serialize() const;
	void deserialize( Serializable::PropertyTree& pt );
	///@}
		
protected:
	// Invoked once in first render() call
	// Creates OpenGL texture
	bool init();

	// Uploads data to texture. Called in render() if dirty flag is set.
	void updateTexture();

private:
	bool        m_initialized;
	std::string m_filename;	
	int         m_width, m_height;	
	unsigned char* m_data; ///< RGBA image data
	GLTexture   m_target;
	bool        m_dirty; // updateTexture() required?
};

#endif // IMAGEMODULE_H
