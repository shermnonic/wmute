#include <cstdlib>       // EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>
#include <stdexcept>     // std::runtime_error
#include <GL/glew.h>     // include before glfw3.h or gl.h
#include <GLFW/glfw3.h>
#include <e8/viewer/ViewerGLFW.h>
#include <FMODAudio.h>

//#include "DemoRenderer.h"
#include "Oscilloscope.h"

int main( int argc, char* argv[] )
{
	using namespace std;

	// --- Setup GLFW window & OpenGL context

	GLFWwindow* window;

	if( !glfwInit() )
		throw std::runtime_error( "Error on initializing GLFW library!" );

	window = glfwCreateWindow( 1280, 720, "demo", NULL, NULL );
	if( !window )
	{
		glfwTerminate();
		throw std::runtime_error( "Error creating window with GLFW library!" );
	}

	glfwMakeContextCurrent(window);

	// --- Setup GLEW

	glewExperimental = GL_TRUE;	
	GLenum glew_err = glewInit();
	if( glew_err != GLEW_OK )
	{
		cerr << "GLEW error:" << glewGetErrorString(glew_err) << endl;
		throw std::runtime_error( "Error on initializing GLEW library!" );
	}

	// --- Setup ViewerGLFW

	ViewerGLFW viewer( window );

	Oscilloscope oscilloscope;
	viewer.setRenderer( &oscilloscope );

	// --- Setup FMODAudio

	FMODAudio audio;
	audio.init();
#if 1
	// Record from microphone input (no playback)
	cout << "Available record devices (the first one will be used):" << endl;
	for( int i=0; i < audio.get_record_driver_info().size(); i++ )
		cout << i+1 << ": " << audio.get_record_driver_info().at(i) << endl;
	audio.set_record_driver( 0 );
	audio.set_record_length( 1.f/25.f );
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
		oscilloscope.setData( &audio.get_wavedata()[0],
		                     (int)audio.get_wavedata().size() );
		//oscilloscope.setSpectrumData( &audio.get_spectrum()[0], 
		//                              (int)audio.get_spectrum().size() );
		oscilloscope.update( (float)dt );
		oscilloscope.render();

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
