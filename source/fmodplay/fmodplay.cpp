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
