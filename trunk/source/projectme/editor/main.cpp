#include <QApplication>
#include <QWidget>
#include <QGLFormat>
#include "MainWindow.h"

int main( int argc, char* argv[] )
{
	Q_INIT_RESOURCE( projectme );

	QApplication app( argc, argv );

	// Set multisampling option for OpenGL.
	// Note that it still has to enabled in OpenGL via glEnable(GL_MULTISAMPLE)
    QGLFormat glf = QGLFormat::defaultFormat();
    glf.setSampleBuffers(true);
    glf.setSamples(4);
    QGLFormat::setDefaultFormat(glf);

	MainWindow mw;
	mw.show();

	return app.exec();;
}
