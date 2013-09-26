#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <GLFW/glfw3.h>
#include <e8/viewer/ViewerGLFW.h>

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

	// --- Main loop

	while( !glfwWindowShouldClose( window ) )
	{
		renderer.render();

		glfwSwapBuffers( window );
		glfwPollEvents();
	}
	
	// --- Exit
	
	glfwTerminate();
	return EXIT_SUCCESS;
}
