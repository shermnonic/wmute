#ifndef TRANSFERFUNCTION_H
#define TRANSFERFUNCTION_H

#include <glutils/GLTexture.h>


/** @addtogroup meshspace meshspace
  * @{ */

/// Color transfer function implemented as 1D OpenGL texture
class TransferFunction
{
public:
	// create() and destroy() require valid OpenGL context
	bool create();
	void destroy();	

	void bind( int tex_unit=-1 );
	void release();

	void draw() const;

	void getColor( float scalar, float& r, float &g, float &b ) const;

private:
	GL::GLTexture m_tex;
};

/** @} */ // end group

#endif // TRANSFERFUNCTION_H
