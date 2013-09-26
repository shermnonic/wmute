#ifndef DEMORENDERER_H
#define DEMORENDERER_H

#include <e8/base/AbstractRenderer.h>

class DemoRenderer : public AbstractRenderer
{
public:
	void initialize();
	void update( float t );
	void render();
	void resize( int w, int h );	
};

#endif // DEMORENDERER_H
