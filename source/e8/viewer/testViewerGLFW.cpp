#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <GLFW/glfw3.h>
#include <e8/viewer/ViewerGLFW.h>
#include <e8/base/AbstractRenderer.h>


class MyRenderer : public AbstractRenderer
{
public:
	void initialize()
	{
	}

	void update( float t )
	{
	}

	void render()
	{
		glClearColor( 0,1,0,1 );
		glClear( GL_COLOR_BUFFER_BIT );
	}

	void resize( int w, int h )
	{
		glViewport(0, 0, w, h);
	}
};

int main( int argc, char* argv[] )
{
	// --- Setup GLFW window & OpenGL context

	GLFWwindow* window;

	if( !glfwInit() )
		return -1;

	window = glfwCreateWindow( 512, 512, "e8 ViewerGLFW", NULL, NULL );
	if( !window )
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// --- Setup ViewerGLFW

	ViewerGLFW viewer( window );

	MyRenderer renderer;
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
