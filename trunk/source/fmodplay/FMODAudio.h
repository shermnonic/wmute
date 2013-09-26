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

/// Simple Wrapper for FMOD to play a music stream and perform spectrum analysis.
/// Code adapted from FMOD API Play Stream example.
/// Maybe this class should be a singleton?
class FMODAudio
{
public:
	FMODAudio()
		: m_system(NULL),
		  m_music(NULL),
		  m_channel(NULL),
		  m_version(-1),
		  m_spectrum(2048,0.f)
		{}

	void init();
	void destroy();

	void update();

	void create_music( const char* filename );
	void play_music();

	std::vector<float>& get_spectrum() { return m_spectrum; }

protected:
	void check_and_handle_error( FMOD_RESULT result );
	void fatal_error( const char* format, ... );

private:
    FMOD::System     *m_system;
    FMOD::Sound      *m_music;
    FMOD::Channel    *m_channel;
	FMOD::DSP        *m_mydsp;
	FMOD::DSPConnection *m_mydspcon;
    unsigned int      m_version;

	std::vector<float> m_spectrum;
};

#endif // FMODAUDIO_H
