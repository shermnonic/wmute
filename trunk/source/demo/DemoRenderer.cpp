#include "DemoRenderer.h"
#include <e8/base/gl.h>
#include <algorithm>

void DemoRenderer::initialize()
{
}

void DemoRenderer::update( float t )
{
}

void DemoRenderer::render()
{
	glDisable( GL_DEPTH_TEST );

	glClearColor( 0,0,0,1 );
	glClear( GL_COLOR_BUFFER_BIT );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();	
	gluOrtho2D( -1, 1, -1, 1 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glColor3f( 1,1,1 );

	// Simple VU meter
	// See also: http://katyscode.wordpress.com/2013/01/16/cutting-your-teeth-on-fmod-part-4-frequency-analysis-graphic-equalizer-beat-detection-and-bpm-estimation/

	if( m_specBuf && m_specLen>0 )
	{
		float max_ = *std::max_element( m_specBuf, m_specBuf+m_specLen );

		glBegin( GL_LINE_STRIP );
		for( unsigned i=0; i < m_specLen; i++ )
		{
			float x = 2.f*(float)i/(m_specLen-1)-1.f;
			float y = m_specBuf[i] / max_;
			glVertex2f( 2.f*(float)i/(m_specLen-1)-1.f, m_specBuf[i] / max_ );
		}
		glEnd();
	}

	glFlush();
}

void DemoRenderer::resize( int w, int h )
{
	glViewport(0, 0, w, h);
}
