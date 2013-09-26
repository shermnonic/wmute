#include "DemoRenderer.h"
#include <e8/base/gl.h>

void DemoRenderer::initialize()
{
}

void DemoRenderer::update( float t )
{
}

void DemoRenderer::render()
{
	glClearColor( 0,1,0,1 );
	glClear( GL_COLOR_BUFFER_BIT );
}

void DemoRenderer::resize( int w, int h )
{
	glViewport(0, 0, w, h);
}
