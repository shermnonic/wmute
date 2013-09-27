#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>
#include <GLFW/glfw3.h>
#include <e8/viewer/ViewerGLFW.h>
#include <FMODAudio.h>

#include "DemoRenderer.h"

int main( int argc, char* argv[] )
{
	using namespace std;

	// --- Setup GLFW window & OpenGL context

	GLFWwindow* window;

	if( !glfwInit() )
		return -1;

	window = glfwCreateWindow( 1280, 720, "demo", NULL, NULL );
	if( !window )
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// --- Setup ViewerGLFW

	ViewerGLFW viewer( window );

	DemoRenderer renderer;
	viewer.setRenderer( &renderer );

	// --- Setup FMODAudio

	FMODAudio audio;
	audio.init();
#if 1
	// Record from microphone input (no playback)
	cout << "Available record devices (the first one will be used):" << endl;
	for( int i=0; i < audio.get_record_driver_info().size(); i++ )
		cout << i+1 << ": " << audio.get_record_driver_info().at(i) << endl;
	audio.set_record_driver( 0 );
	audio.set_record_length( 1./25.f );
	audio.start_record();
#else
	// Playback MP3 music file
	audio.create_music("G:\\386dx25\\Res\\after work.mp3");
	audio.play_music();
#endif

	// --- Main loop

	double time0 = glfwGetTime();

	while( !glfwWindowShouldClose( window ) )
	{
		// Delta time
		double time1 = glfwGetTime();
		double dt = time1 - time0;
		time0 = time1;

		// Update demo
		renderer.setSpectrumData( &audio.get_wavedata()[0],
		                          (int)audio.get_wavedata().size() );
		//renderer.setSpectrumData( &audio.get_spectrum()[0], 
		//                          (int)audio.get_spectrum().size() );
		renderer.update( (float)dt );
		renderer.render();

		// Update GLFW
		glfwSwapBuffers( window );
		glfwPollEvents();

		// Update FMOD
		audio.update();
	}
	
	// --- Clean up & exit

	audio.destroy();
	glfwTerminate();
	return EXIT_SUCCESS;
}
