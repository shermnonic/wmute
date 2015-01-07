#ifndef IMAGEMODULE_H
#define IMAGEMODULE_H

#include <glutils/GLTexture.h>
#ifdef GL_NAMESPACE
using GL::GLTexture;
#endif

#include "RenderSet.h" // for ModuleRenderer

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

	///@name ModuleRenderer implementation
	///@{
	void render();
	int  target() const { return m_target.GetID(); }
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
