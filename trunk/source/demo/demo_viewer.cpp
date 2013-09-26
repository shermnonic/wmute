#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <GLFW/glfw3.h>
#include <e8/viewer/ViewerGLFW.h>
#include <FMODAudio.h>

#include "DemoRenderer.h"

int main( int argc, char* argv[] )
{
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
	audio.create_music("G:\\386dx25\\Res\\after work.mp3");
	audio.play_music();

	// --- Main loop

	while( !glfwWindowShouldClose( window ) )
	{
		renderer.setSpectrumData( &audio.get_spectrum()[0], audio.get_spectrum().size() );

		// Update demo
		renderer.update( 0.1f );
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
