// demo_editor
#include <QApplication>
#include <iostream>
#include <param/PropertyTreeWidget.h>
#include <param/ParameterTypes.h>
#include "Oscilloscope.h"

int main( int argc, char* argv[] )
{
	//Q_INIT_RESOURCE( demo_editor );

	QApplication app( argc, argv );

	Oscilloscope oscilloscope;

	PropertyTreeWidget w;
	w.setParameters( &oscilloscope.paramlist() );
	w.show();

#ifdef DEBUG
	std::cout << "Entering Qt application loop: app.exec()" << std::endl;
#endif
	int ret = app.exec();
	QApplication::closeAllWindows();
	return ret;
}
