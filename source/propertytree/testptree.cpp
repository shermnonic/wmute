#include <iostream>
#include "ParameterTypes.h"
#include "ParameterIO.h"

// --- Parameter test program ---

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

	void test()
	{
		save_params( "foobar.xml", parms );

		load_params( "foobar.xml", parms );
	}

	DoubleParameter d0;
	DoubleParameter d1;
	IntParameter    index;
	StringParameter title;

	ParameterList   parms;
};

int main( int argc, char* argv[] )
{
	using namespace std;

	Foo foo;
	foo.test();

	return EXIT_SUCCESS;
}
