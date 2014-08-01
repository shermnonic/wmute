#include <QApplication>
#include <QWidget>
#include "MainWindow.h"

int main( int argc, char* argv[] )
{
	Q_INIT_RESOURCE( projectme );

	QApplication app( argc, argv );

	MainWindow mw;
	mw.show();

	return app.exec();;
}
