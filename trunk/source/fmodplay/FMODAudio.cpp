#include "FMODAudio.h"

void FMODAudio::check_and_handle_error( FMOD_RESULT result )
{
	if( result != FMOD_OK )
		fatal_error("FMOD error %d - %s", result, FMOD_ErrorString(result));
}

void FMODAudio::fatal_error( const char* format, ... )
{
    char error[1024] = {0};

    va_list args;
    va_start(args, format);
    vsprintf(error, format, args);
    va_end(args);

	printf("%s\n", error);

    throw error;
}

void FMODAudio::destroy()
{	
	if( m_music  ) 
	{
		check_and_handle_error( m_music->release()  );
	}
	if( m_system ) 
	{
		check_and_handle_error( m_system->close()   );
		check_and_handle_error( m_system->release() );;
	}
}

void FMODAudio::init()
{
    FMOD_RESULT result;

    /*
        Create a System object and initialize.
    */
    result = FMOD::System_Create(&m_system);
	check_and_handle_error( result );

    result = m_system->getVersion(&m_version);
	check_and_handle_error( result );

    if (m_version < FMOD_VERSION)
    {
        fatal_error("FMOD lib version %08x doesn't match header version %08x", 
			m_version, FMOD_VERSION);
    }

    result = m_system->init(32, FMOD_INIT_NORMAL, 0/*extradriverdata*/);
    check_and_handle_error(result);
}

void FMODAudio::create_music( const char* filename )
{
/*
From the API doc:
-----------------
To specifically make a sound software mixed, you must use FMOD_SOFTWARE. This is 
necessary if you want to use things such as DSP effects, spectrum analysis, 
getwavedata, point to point looping and other more advanced techniques. 
*/

    FMOD_RESULT result;
    result = m_system->createSound(	filename, FMOD_SOFTWARE, 0, &m_music );
    check_and_handle_error(result);

#ifndef FMODAUDIO_USE_FMODEX
	/*
		Spectrum analysis DSP
	*/
	m_system->createDSPByType( FMOD_DSP_TYPE_FFT, &m_mydsp );
	m_channel->addDSP( 0, m_mydsp, &m_mydspcon );
#endif
}

void FMODAudio::play_music()
{
    FMOD_RESULT result;
    /*
        Play the sound.
    */
#ifdef FMODAUDIO_USE_FMODEX    
	result = m_system->playSound(FMOD_CHANNEL_FREE, m_music, false, &m_channel);
#else
	result = m_system->playSound(m_music, 0, false, &m_channel);
#endif
    check_and_handle_error(result);
}

void FMODAudio::update()
{
    FMOD_RESULT result;
    result = m_system->update();
    check_and_handle_error(result);

	// Playground for testing FMOD functionality...

	// Update spectrum analysis
#ifdef FMODAUDIO_USE_FMODEX
	result = m_channel->getSpectrum( &m_spectrum[0], m_spectrum.size(), 0, 
		                             FMOD_DSP_FFT_WINDOW_TRIANGLE );
	check_and_handle_error(result);
#else
	// FIXME: FMOD Studio FFT not implemented yet!
#endif
}
