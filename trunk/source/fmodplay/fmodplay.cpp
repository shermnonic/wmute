#include <iostream>

//-----------------------------------------------------------------------------
//  FMODAudio
//-----------------------------------------------------------------------------

#include <stdarg.h>
#include <stdio.h>
#include "fmod.hpp"
#include "fmod_errors.h"

// Code adapted from FMOD API Play Stream example.
// Maybe this class should be a singleton?
class FMODAudio
{
public:
	FMODAudio()
		: m_system(NULL),
		  m_music(NULL),
		  m_channel(NULL),
		  m_version(-1)
		{}

	void init();
	void destroy();

	void update();

	void create_music( const char* filename );
	void play_music();

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
};

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

	/*
		Spectrum analysis DSP
	*/
	m_system->createDSPByType( FMOD_DSP_TYPE_FFT, &m_mydsp );
	m_channel->addDSP( 0, m_mydsp, &m_mydspcon );
}

void FMODAudio::play_music()
{
    FMOD_RESULT result;
    /*
        Play the sound.
    */
    result = m_system->playSound(m_music, 0, false, &m_channel);
    check_and_handle_error(result);
}

void FMODAudio::update()
{
    FMOD_RESULT result;
    result = m_system->update();
    check_and_handle_error(result);

	// Playground for testing FMOD functionality...
}

//-----------------------------------------------------------------------------
//  main
//-----------------------------------------------------------------------------
#include <conio.h> // kbhit(), getch()
int main( int argc, char* argv[] )
{
	using namespace std;

	FMODAudio audio;

	audio.init();
	audio.create_music("G:\\386dx25\\Res\\after work.mp3");
	audio.play_music();

	cout << "Hello FMOD!" << endl
		 << "Press any key to exit..." << endl;
	
	do {
		audio.update();
	} while( !kbhit() );

	audio.destroy();
	
	return EXIT_SUCCESS;
}
