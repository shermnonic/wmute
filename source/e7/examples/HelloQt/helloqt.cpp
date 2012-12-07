#include <QApplication>
#include <QIcon>
#include <QPixmap>
#include <Engines/Qt/EngineQt.h>
#include <iostream>
#include <exception>
#include <GL/glew.h>

class HelloQt : public EngineQt
{
public:
	void render();
};

void HelloQt::render()
{
	glClearColor( 74./255., 189./255., 237./255., 1 );
	glClear( GL_COLOR_BUFFER_BIT );

	loadCameraMatrix();

	glColor3f( 1,1,1 );
	glBegin( GL_QUADS );
	glVertex3f( -1,-1,0 );
	glVertex3f(  1,-1,0 );
	glVertex3f(  1, 1,0 );
	glVertex3f( -1, 1,0 );
	glEnd();

	glFlush();
}

int main( int argc, char* argv[] )
{
	using namespace std;
	
	Q_INIT_RESOURCE( helloqt );
	
	QApplication app( argc, argv );	
	
	HelloQt hello;	
	hello.setWindowIcon( QIcon(QPixmap(":/data/icons/icon.png")) );
	hello.show();
	hello.run( argc, argv ); // EngineQt::run() will return immediately
	
	// launch Qt main loop
	int ret = app.exec();
	QApplication::closeAllWindows();
	return ret;
}
