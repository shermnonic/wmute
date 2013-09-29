#include "Oscilloscope.h"
#include <e8/base/gl.h>
#include <algorithm>

Oscilloscope::Oscilloscope( std::string moduleName )
  : Module( moduleName )    
{
	paramlist().push_back( &params().mode );
	paramlist().push_back( &params().autoScale );
	paramlist().push_back( &params().lineWidth );
}

void Oscilloscope::initialize()
{
	glDisable( GL_DEPTH_TEST );
	glClearColor( 0,0,0,1 );
}


void Oscilloscope::resize( int w, int h )
{
	glViewport(0, 0, w, h);
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();	
	gluOrtho2D( -1, 1, -1, 1 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();	
}

void Oscilloscope::update( float t )
{
}

void Oscilloscope::render()
{
	glClear( GL_COLOR_BUFFER_BIT );
	glColor3f( 1,1,1 );

	if( !m_buf || m_buflen <= 0 )
		return;
	
	switch( m_params.mode.value() ) {
	default:
	case Waveform:   drawWaveform ( m_buf, m_buflen );   break;
	case Histogram:  drawHistogram( m_buf, m_buflen );  break;
	}
}

void Oscilloscope::drawWaveform( float* buf, int len )
{
	// Simple VU meter
	// See also: http://katyscode.wordpress.com/2013/01/16/cutting-your-teeth-on-fmod-part-4-frequency-analysis-graphic-equalizer-beat-detection-and-bpm-estimation/

	// Auto scale to max. value
	float max_ = 1.f;
	if( m_params.autoScale.value() == (int)true )
		max_ = *std::max_element( buf, buf+len );

	glBegin( GL_LINE_STRIP );
	for( int i=0; i < len; i++ )
	{
		float x = 2.f*(float)i/(len-1)-1.f;
		float y = buf[i] / max_;
		glVertex2f( 2.f*(float)i/(len-1)-1.f, buf[i] / max_ );
	}
	glEnd();
}

void Oscilloscope::drawHistogram( float* buf, int len )
{
}
