#include <GLFW/glfw3.h>
#include <iostream>



int main( int argc, char* argv[] )
{
	using namespace std;

	// --- Setup GLFW window & OpenGL context

	GLFWwindow* window;

	if( !glfwInit() )
		return -1;

	window = glfwCreateWindow( 512, 512, "Hello demo!", NULL, NULL );
	if( !window )
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// --- Main loop

	while( !glfwWindowShouldClose( window ) )
	{

		glfwSwapBuffers( window );
		glfwPollEvents();
	}
	
	// --- Exit
	
	glfwTerminate();
	return EXIT_SUCCESS;
}
