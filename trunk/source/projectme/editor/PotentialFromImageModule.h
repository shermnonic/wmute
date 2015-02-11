#ifndef POTENTIALFROMIMAGEMODULE_H
#define POTENTIALFROMIMAGEMODULE_H

#include <glutils/GLTexture.h>
#ifdef GL_NAMESPACE
using GL::GLTexture;
#endif

#include "ModuleRenderer.h"

#include <string>

// Forwards
class QImage;

/**
	\class PotentialFromImageModule
*/
class PotentialFromImageModule : public ModuleRenderer
{
public:
	typedef ModuleRenderer Super;	

	PotentialFromImageModule();

	bool loadImage( const char* filename );

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
	float*      m_data;
	GLTexture   m_target;
	bool        m_dirty; // updateTexture() required?
};

#endif // POTENTIALFROMIMAGEMODULE_H
