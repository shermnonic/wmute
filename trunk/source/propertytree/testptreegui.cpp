// testptreegui - GUI for my Parameter library
// Max Hermann, September 2013
#include <QApplication>
#include "PropertyTreeWidget.h"
#include "ParameterTypes.h"
#include <iostream>

struct Foo
{
	Foo()
		: d0( "d0" ),
		  d1( "d1" ),
		  index( "index" ),
		  title( "title" )
	{
		parms.push_back( &d0 );
		parms.push_back( &d1 );
		parms.push_back( &index );
		parms.push_back( &title );

		d0.setValue( 42.3 );
		d1.setValue( 23.7 );
		index.setValue( 77 );
		title.setValue( "Foobar" );
	}

	DoubleParameter d0;
	DoubleParameter d1;
	IntParameter    index;
	StringParameter title;

	ParameterList   parms;
};

int main( int argc, char* argv[] )
{
	//Q_INIT_RESOURCE( testptreegui );

	QApplication app( argc, argv );

	Foo foo;

	PropertyTreeWidget w;
	w.setParameters( &foo.parms );
	w.show();

#ifdef DEBUG
	std::cout << "Entering Qt application loop: app.exec()" << std::endl;
#endif
	int ret = app.exec();
	QApplication::closeAllWindows();
	return ret;
}
