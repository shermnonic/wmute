#ifndef DEMORENDERER_H
#define DEMORENDERER_H

#include <e8/base/AbstractRenderer.h>

class DemoRenderer : public AbstractRenderer
{
public:
	DemoRenderer()
		: m_specBuf(0),
		  m_specLen(0)
	{}

	// AbstractRenderer implementation
	void initialize();
	void update( float t );
	void render();
	void resize( int w, int h );	

	void setSpectrumData( float* buf, int len ) { m_specBuf=buf; m_specLen=len;}

private:
	float* m_specBuf;
	int    m_specLen;
};

#endif // DEMORENDERER_H
