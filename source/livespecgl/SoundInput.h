#ifndef SOUNDINPUT_H
#define SOUNDINPUT_H

#include <bass.h>

/// Small wrapper for BASS library providing sample input from mp3 or microphone
class SoundInput
{
public:
	enum InputType { NoInput, RecordInput, StreamInput };
	
	SoundInput()
	: m_mode(NoInput), m_stream(0), m_pause(false)
	{}

	bool setupDevice();
	void shutdownDevice();

	/// Use file as sound input and play it as well
	bool openFile( const char* filename );

	/// Record input from any source (e.g. microphone)
	bool openInput();

	/// Start sound input and eventual playback, requires an open input or file
	bool startInput()
	{
		return BASS_ChannelPlay( m_stream, TRUE );
	}

	/// Poll max. len bytes sample data, returns number of bytes written to buffer.
	/// Assumes unsigned short samples.
	int pollSampleData( void* buffer, int len )
	{		
		return (int)BASS_ChannelGetData( m_stream, buffer, len*sizeof(short) );
	}
	
	/// Returns pointer to 512 float FFT data
	float* pollFFT()
	{
		DWORD err = 
			BASS_ChannelGetData( m_stream, &m_fft[0], BASS_DATA_FFT1024 );
		return &m_fft[0];
	}
	
	/// Pause music playback
	void togglePause()
	{		
		if( m_mode==StreamInput )
		{			
			if( m_pause )
				BASS_ChannelPlay( m_stream, FALSE );
			else
				BASS_ChannelPause( m_stream );
			m_pause = !m_pause;
		}
	}
	
	// Return percentual playing position if playing from file
	double getStreamProgress()
	{
		if( m_mode==StreamInput )
		{			
			static QWORD len = BASS_ChannelGetLength( m_stream, BASS_POS_BYTE );
			QWORD pos = BASS_ChannelGetPosition( m_stream, BASS_POS_BYTE );
			return pos/(double)len;				
		}
	}

private:
	int     m_mode;
	DWORD   m_stream; // recording channel (HRECORD) or file stream (HSTREAM)
	bool    m_pause;
	float   m_fft[512];
};

#endif // SOUNDINPUT_H
