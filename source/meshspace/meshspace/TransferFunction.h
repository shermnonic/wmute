#ifndef TRANSFERFUNCTION_H
#define TRANSFERFUNCTION_H

#include <glutils/GLTexture.h>

class TransferFunction
{
public:
	// create() and destroy() require valid OpenGL context
	bool create();
	void destroy();	

	void bind( int tex_unit=0 );
	void release();

private:
	GL::GLTexture m_tex;
};

#endif // TRANSFERFUNCTION_H
