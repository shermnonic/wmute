#include "SoundInput.h"
#include <bass.h>

#include <cstdlib>
#include <cstdio>
#include <iostream>

using namespace std;

#pragma warning(disable:4800) // BOOL warning

// Recording callback - not doing anything with the data
BOOL CALLBACK DuffRecording(HRECORD handle, const void *buffer, DWORD length, void *user)
{
	return TRUE; // continue recording
}

void printBASSInfo()
{
	// available recording devices
	BASS_DEVICEINFO info;
	printf("List of recording devices:\n");
	for( int i=0; BASS_RecordGetDeviceInfo(i, &info); ++i )
	{
		printf("  Device number %2d\n"
		       "    Name  : %s\n"
		       "    Driver: %s\n"
			   "    Status: %s\n",
			i, info.name, info.name, 
			(info.flags & BASS_DEVICE_ENABLED)?"enabled":"disabled");
	}
	printf("\n");

	// current recording inputs
	int n;
	for( n=0; BASS_RecordGetInputName(n); n++ ) 
	{
		float vol;
		int s=BASS_RecordGetInput(n, &vol);
		printf("%s [%s : %g]\n", 
				BASS_RecordGetInputName(n), s&BASS_INPUT_OFF?"off":"on", vol);
	} 
	if( n==0 )
		printf("No available input sources found!\n");
	printf("\n");
}

void checkBASSError( unsigned long err )
{
	if( err==0xFFFFFFFF )
	{
		cerr << "BASS error: ";
		err=BASS_ErrorGetCode();
		if( err != BASS_OK )
		{
			switch( err )
			{
			case BASS_ERROR_HANDLE  : cerr << "Music stream is not a valid handle! ";
			case BASS_ERROR_NOPLAY  : cerr << "Channel is not playing! ";
			case BASS_ERROR_ILLPARAM: cerr << "Buffer length is invalid! ";
			case BASS_ERROR_MEM     : cerr << "Insufficient memory! ";
			case BASS_ERROR_BUFLOST : cerr << "Memory leak detected! ";
			default:
				cerr << "Error code " << err;
			}
		}
		cerr << endl;
	}
}



SoundInput::SoundInput()
  : m_initialized(false),
    m_deviceInitialized(false),
    m_mode(NoInput), 
    m_streamPtr(0), 
	m_pause(false)
{
	m_streamPtr = malloc( sizeof(DWORD) );
	m_initialized = setupDevice();
}

SoundInput::~SoundInput()
{
	shutdownDevice();
	free( m_streamPtr );
}

bool SoundInput::setupDevice()
{
	// check the correct BASS was loaded
	if( HIWORD(BASS_GetVersion()) != BASSVERSION ) 
	{
		cerr << "Error: An incorrect version of BASS.DLL was loaded (2.4 is required)." << endl;
		return false;
	}
	
	printBASSInfo();
	return true;
}

void SoundInput::shutdownDevice()
{
	BASS_Stop();  // stop any output
	BASS_StreamFree( *(DWORD*)m_streamPtr );
	BASS_RecordFree();
	BASS_Free();  // close sound system
}

bool SoundInput::openFile( const char* filename )
{	
	// Use mp3 file as input and play it as well
	HSTREAM stream;

	// Initialize default device, 44100hz, stereo, 16 bits, no syncs used
	if( !m_deviceInitialized )
	{
		// Do this only once
		if( !BASS_Init( 1, 44100, 0, 0, NULL) ) 
		{
			cerr << "Error: Can't initialize digital sound system!" << endl;
			return false;
		}
		m_deviceInitialized = true;
	}

	BASS_Stop();
	BASS_StreamFree( *(DWORD*)m_streamPtr );
	if( !(stream = BASS_StreamCreateFile( FALSE, filename, 0,0,BASS_SAMPLE_MONO )) )
	{
		cerr << "Error: Can't create Music stream!" << endl;
		return false;
	}

	// Start output system
	BASS_Start();

	*(DWORD*)m_streamPtr = (DWORD)stream;
	m_mode = StreamInput;
	return true;
}

bool SoundInput::openInput()
{	
	HRECORD rec;
	
	if( !BASS_RecordInit(-1) )
	{
		cerr << "Error: BASS can't initialize device!" << endl;
		return false;
	}

	// start recording (44100hz mono 16-bit)
	if( !(rec=BASS_RecordStart(44100,1,0,&DuffRecording,0)) ) 
	{
		cerr << "Error: BASS can't start recording!" << endl;
		return false;
	}

	*(DWORD*)m_streamPtr = rec;
	m_mode = RecordInput;
	return true;
}


bool SoundInput::startInput()
{
	return BASS_ChannelPlay( *(DWORD*)m_streamPtr, TRUE );
}

int SoundInput::pollSampleData( void* buffer, int len )
{		
	return (int)BASS_ChannelGetData( *(DWORD*)m_streamPtr, buffer, len*sizeof(short) );
}

float* SoundInput::pollFFT()
{
	DWORD err = 
		BASS_ChannelGetData( *(DWORD*)m_streamPtr, &m_fft[0], BASS_DATA_FFT1024 );
	return &m_fft[0];
}

void SoundInput::togglePause()
{		
	playPause( !m_pause );
}

void SoundInput::playPause( bool play )
{
	if( m_mode==StreamInput )
	{			
		if( play )
			BASS_ChannelPlay( *(DWORD*)m_streamPtr, FALSE );
		else
			BASS_ChannelPause( *(DWORD*)m_streamPtr );
		m_pause = !play;
	}		
}

void SoundInput::restart()
{
	if( m_mode==StreamInput ) 
	{
		BASS_ChannelPlay( *(DWORD*)m_streamPtr, TRUE );
		m_pause = false;
	}
}
	
double SoundInput::getStreamProgress()
{
	if( m_mode==StreamInput )
	{			
		static QWORD len = BASS_ChannelGetLength( *(DWORD*)m_streamPtr, BASS_POS_BYTE );
		QWORD pos = BASS_ChannelGetPosition( *(DWORD*)m_streamPtr, BASS_POS_BYTE );
		return pos/(double)len;				
	}
	return 0.;
}
