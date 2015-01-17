#ifndef SOUNDINPUT_H
#define SOUNDINPUT_H

/// Small wrapper for BASS library providing sample input from mp3 or microphone
class SoundInput
{
public:
	enum InputType { NoInput, RecordInput, StreamInput };
	
	SoundInput();
	~SoundInput();

	bool isInitialized() const { return m_initialized; }

	/// Use file as sound input and play it as well
	bool openFile( const char* filename );

	/// Record input from any source (e.g. microphone)
	bool openInput();

	/// Start sound input and eventual playback, requires an open input or file
	bool startInput();

	/// Poll max. len bytes sample data, returns number of bytes written to buffer.
	/// Assumes unsigned short samples.
	int pollSampleData( void* buffer, int len );
	
	/// Returns pointer to 512 float FFT data
	float* pollFFT();
	
	/// Pause music playback
	void togglePause();

	/// Pause music playback
	void playPause( bool play );

	/// Restart music playback (from beginning of sound file)
	void restart();
	
	// Return percentual playing position if playing from file
	double getStreamProgress();

protected:
	bool setupDevice();
	void shutdownDevice();

private:
	bool    m_initialized;
	bool    m_deviceInitialized;
	int     m_mode;
	void*   m_streamPtr; // WORKAROUND to no use DWORD to avoid inclusion of bass.h here in header
	// was: DWORD m_stream; // recording channel (HRECORD) or file stream (HSTREAM)
	bool    m_pause;
	float   m_fft[512];
};

#endif // SOUNDINPUT_H
