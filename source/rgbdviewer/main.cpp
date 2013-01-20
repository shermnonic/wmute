// rgbdviewer - replay raw depth+color images acquired by rgbd-demo
// Max Hermann, January 14, 2013
#include <QApplication>
#include "RGBDViewerMainWindow.h"
#include <iostream>

int main( int argc, char* argv[] )
{
	Q_INIT_RESOURCE( rgbdviewer );

	QApplication app( argc, argv );

	RGBDViewerMainWindow viewer;
	viewer.show();

#ifdef DEBUG
	std::cout << "Entering Qt application loop: app.exec()" << std::endl;
#endif
	int ret = app.exec();
	QApplication::closeAllWindows();
	return ret;
}

