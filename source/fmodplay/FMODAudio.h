#ifndef FMODAUDIO_H
#define FMODAUDIO_H

// Backwards compatibility
// Enable the following define when using FMODEx instead of FMODStudio API.
// For now FFT spectrum analysis is only implemented for FMODEx.
#define FMODAUDIO_USE_FMODEX

#include <stdarg.h>
#include <stdio.h>
#include "fmod.hpp"
#include "fmod_errors.h"
#include <vector>
#include <string>

/// Simple Wrapper for FMOD to play a music stream and perform spectrum analysis.
/// Code adapted from FMOD API Play Stream example.
/// Maybe this class should be a singleton?
class FMODAudio
{
public:
	enum UpdateFlags {
		UpdateWavedata= 0x01,
		UpdateSpectrum= 0x02		
	};

	FMODAudio()
		: m_system (NULL),
		  m_music  (NULL),
		  m_channel(NULL),
#ifdef FMODAUDIO_USE_FMODEX
		  m_recordsound (NULL),
		  m_recorddriver(0),
		  m_recordlength(1./10.f),
#endif
		  m_outputrate(48000),
		  m_spectrum(2048,0.f),
		  m_wavedata(2049,0.f),
		  m_updateflags( UpdateWavedata | UpdateSpectrum ),
		  m_version(-1)
		{}

	void init();
	void destroy();

	/// Updates the FMOD system, should be called once per frame.
	void update();
	void set_update_flags( int flags ) { m_updateflags = flags; }

	/// Setup playback of a music file from disk
	void create_music( const char* filename );
	/// Start music playback (no guarantees about lag)
	void play_music();

#ifdef FMODAUDIO_USE_FMODEX
	/// Enumerate record devices by name
	std::vector<std::string> get_record_driver_info() const;
	/// Set record device number, see also \a get_record_driver_info()
	void set_record_driver( int );
	/// Set size of record buffer in seconds, determines lag of get_wavedata()
	void set_record_length( float sec );
	/// Start recording
	void start_record();
#endif

	std::vector<float>& get_spectrum() { return m_spectrum; }
	std::vector<float>& get_wavedata() { return m_wavedata; }

protected:
	void check_and_handle_error( FMOD_RESULT result ) const;
	void fatal_error( const char* format, ... ) const;

private:
    FMOD::System     *m_system;
    FMOD::Sound      *m_music;
    FMOD::Channel    *m_channel;

#ifndef FMODAUDIO_USE_FMODEX
	// If not using FMODex bud FMOD Studio we have to set our own DSP filter
	// for FFT spectrum analysis.
	FMOD::DSP           *m_mydsp;
	FMOD::DSPConnection *m_mydspcon;
#endif

#ifdef FMODAUDIO_USE_FMODEX
	// Recording input is only available for FMODex but not FMOD Studio (?)
	FMOD_CREATESOUNDEXINFO  m_exinfo; // record info
	FMOD::Sound            *m_recordsound;
	int                     m_recorddriver;
	float                   m_recordlength;
#endif

	int                m_outputrate;
	std::vector<float> m_spectrum;
	std::vector<float> m_wavedata;
	int                m_updateflags;
    unsigned int       m_version;
};

#endif // FMODAUDIO_H
