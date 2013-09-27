#include <iostream>
#include "FMODAudio.h"

//-----------------------------------------------------------------------------
//  main
//-----------------------------------------------------------------------------
#include <conio.h> // kbhit(), getch()
int main( int argc, char* argv[] )
{
	using namespace std;

	FMODAudio audio;

	audio.init();
#if 1
	// Record from microphone input (no playback)
	cout << "Available record devices (the first one will be used):" << endl;
	for( int i=0; i < audio.get_record_driver_info().size(); i++ )
		cout << i+1 << ": " << audio.get_record_driver_info().at(i) << endl;
	audio.set_record_driver( 0 );
	audio.start_record();
#else
	// Playback MP3 music file
	audio.create_music("G:\\386dx25\\Res\\after work.mp3");
	audio.play_music();
#endif

	cout << "Hello FMOD!" << endl
		 << "Press any key to exit..." << endl;
	
	do {
		audio.update();
	} while( !kbhit() );

	audio.destroy();
	
	return EXIT_SUCCESS;
}
