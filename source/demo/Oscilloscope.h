#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include <e8/base/AbstractRenderer.h>
#include <e8/base/Module.h>
#include <param/ParameterTypes.h>

/// Simple oscilloscope renderer module
class Oscilloscope : public AbstractRenderer, public Module
{
public:
	enum Mode {
		Waveform,
		Histogram
	};

	struct Parameters
	{
		Parameters()
			: mode( "Mode", "Waveform", "Histogram" ),
			  autoScale( "Auto scale", false ),
			  lineWidth( "Line width", 1.0, 0.01, 20.0 ) 
		{
			mode.setValue(0);
		}

		EnumParameter   mode;
		BoolParameter   autoScale;
		DoubleParameter lineWidth;
	};
	
	Oscilloscope( std::string moduleName="Oscilloscope" );

	///@{ AbstractRenderer implementation
	void initialize();
	void update( float t );
	void render();
	void resize( int w, int h );	
	///@}

	/// Set raw oscilloscope data
	void setData( float* buf, int len ) { m_buf=buf; m_buflen=len;}

	/// Access user parameters
	Parameters& params() { return m_params; }
	
protected:
	void drawWaveform( float* buf, int len );
	void drawHistogram( float* buf, int len );

private:	
	float* m_buf;
	int    m_buflen;

	Parameters m_params;
};

#endif // OSCILLOSCOPE_H
