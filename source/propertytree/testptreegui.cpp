// testptreegui - GUI for my Parameter library
// Max Hermann, September 2013
#include <QApplication>
#include "param/PropertyTreeWidget.h"
#include "param/ParameterTypes.h"
#include <iostream>

struct Foo
{
	Foo()
		: d0( "d0" ),
		  d1( "d1" ),
		  index( "index" ),
		  title( "title" ),
		  mode( "mode", "NoMode", "FooMode", "BarMode", "FoobarMode" ),
		  doit( "doit" )
	{
		parms.push_back( &d0 );
		parms.push_back( &d1 );
		parms.push_back( &index );
		parms.push_back( &title );
		parms.push_back( &mode );
		parms.push_back( &doit );

		d0.setValue( 42.3 ); d0.setLimits(0.0,100.0);
		d1.setValue( 23.7 ); d0.setLimits(-50.0,50.0);
		index.setValue( 77 ); index.setLimits(0,1000);
		title.setValue( "Foobar" );
		mode.setValue( 2 );
		doit.setValue( true );
	}

	DoubleParameter d0;
	DoubleParameter d1;
	IntParameter    index;
	StringParameter title;
	EnumParameter   mode;
	BoolParameter   doit;

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
