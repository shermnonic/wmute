#include <iostream>
#include "param/ParameterTypes.h"
#include "param/ParameterIO.h"

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

		// shuffle ordering
		parms2.push_back( &d0 );
		parms2.push_back( &title );
		parms2.push_back( &d1 );
		parms2.push_back( &index );

		d0.setValue( 42.3 );
		d1.setValue( 23.7 );
		index.setValue( 77 );
		title.setValue( "Foobar" );
	}

	void test()
	{
		using namespace std;

		string filename("foobar.xml");

		cout << "Defined following list of parameters:" << endl;
		print_params( parms );

		cout << "Writing to " << filename << "..." << endl;
		save_params( filename.c_str(), parms );

		cout << "Loading again from disk (with changed ordering)..." << endl;
		load_params( filename.c_str(), parms2 );

		cout << "Loaded list of parameters (with changed ordering):" << endl;
		print_params( parms2 );
	}

	DoubleParameter d0;
	DoubleParameter d1;
	IntParameter    index;
	StringParameter title;

	ParameterList   parms;
	ParameterList   parms2; // different ordering
};

int main( int argc, char* argv[] )
{
	using namespace std;

	Foo foo;
	foo.test();

	return EXIT_SUCCESS;
}
