#include <QApplication>
#include <QWidget>
#include "MainWindow.h"

int main( int argc, char* argv[] )
{
	Q_INIT_RESOURCE( meshspace );

	QApplication app( argc, argv );

	MainWindow mw;
	mw.show();

	return app.exec();;
}
