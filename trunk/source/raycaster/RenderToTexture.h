#ifndef RENDERTOTEXTURE_H
#define RENDERTOTEXTURE_H

#include "GLConfig.h"

class RenderToTexture
{
public:
	bool init( int width, int height );
	void deinit();

	void begin( GLint texid );
	void end();

private:
	GLuint m_fbo;              ///< Framebuffer object	
	int m_width, m_height;     ///< Texture size
	GLint m_viewport[4];
};

#endif // RENDERTOTEXTURE_H
