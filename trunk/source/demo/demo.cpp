#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

//static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
//{
//	if( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
//		glfwSetWindowShouldClose(window, GL_TRUE);
//
//	std::cout << "key " << key << " scancode " << scancode 
//		 << " action " << action << " mods " << mods << std::endl;
//}

class Window
{
public:
	Window()
		: m_window(NULL)
	{}

	bool init( int width, int height, std::string title )
	{
		if( m_window )
			return false;

		if( !glfwInit() )
			return -1;

		m_window = glfwCreateWindow( width, height, title.c_str(), NULL, NULL );
		if( !m_window )
		{
			glfwTerminate();
			return false;
		}

		glfwMakeContextCurrent( m_window );
		glfwSetKeyCallback( m_window, Window::key_callback );

		return true;
	}

	int run()
	{
		while( !glfwWindowShouldClose( m_window ) )
		{

			glfwSwapBuffers( m_window );
			glfwPollEvents();
		}

		glfwTerminate();

		return EXIT_SUCCESS;
	}

protected:
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		std::cout << "key " << key << " scancode " << scancode 
			 << " action " << action << " mods " << mods << std::endl;
	}

private:
	GLFWwindow* m_window;

};

#if 1
int main( int argc, char* argv[] )
{
	Window w;
	if( !w.init(512,512,"Hello GLFW and C++!") )
		return -1;

	return w.run();
}
#else
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

	glfwSetKeyCallback(window, key_callback);

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
#endif