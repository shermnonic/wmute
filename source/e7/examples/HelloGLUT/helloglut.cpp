#include <Engines/GLUT/EngineGLUT.h>
#include <iostream>
#include <exception>
#include <GL/glew.h>

class HelloGLUT : public EngineGLUT
{
public:
	void render();
};

void HelloGLUT::render()
{
	glClearColor( 74./255., 189./255., 237./255., 1 );
	glClear( GL_COLOR_BUFFER_BIT );

	glBegin( GL_QUADS );
	glVertex3f( -1,-1,0 );
	glVertex3f(  1,-1,0 );
	glVertex3f(  1, 1,0 );
	glVertex3f( -1, 1,0 );
	glEnd();
}

int main( int argc, char* argv[] )
{
	using namespace std;
	
	HelloGLUT hello;
	try
	{
		hello.set_window_title("Hello GLUT!");
		hello.run( argc, argv );
	}
	catch( exception& e )
	{
		cerr << "Exception caught:" << endl << e.what() << endl;
	}	
	return 0; // never reached, put cleanup code into EngineGLUT::destroy()
}
