// meshvolume - Compute volume of 3D triangle mesh with consistent orientation.
// Max Hermann, Dec. 2013 (hermann@cs.uni-bonn.de)
#include <iostream>
#include "meshtools.h"

const char g_usage[] =
"meshvolume - Compute the volume of a 3D surface geometry. \n"
"Max Hermann 2013 (hermann@cs.uni-bonn.de)\n"
"\n"
"Usage: meshvolume <filename>\n"
"\n"
"The surface has to be given as 3D triangle mesh with consitent normal \n"
"orientation (consistent CW or CCW vertex ordering for all triangles). \n"
"Supported are all OpenMesh fileformats, e.g. OBJ, PLY, STL, OFF. \n"
"The volume units are cubic units of the input units, e.g. if the input file\n"
"is given in cm the volume measure will be in cm^3.\n";


int main( int argc, char* argv[] )
{
	using namespace meshtools;	

	// -- Parse command line		
	if( argc != 2 )
	{
		std::cout << g_usage;
		return 0;
	}
	
	// -- Load mesh
	Mesh mesh;
	if( !loadMesh( mesh, argv[1] ) )
		return -1;
	
	std::cout << "Loaded mesh '" << argv[1] << "'" << std::endl;	
	printMeshInfo( mesh );

	// -- (Re)compute normals
	updateMeshVertexNormals( &mesh );
	
	// -- Compute mesh volume
	double volume = computeMeshVolume( &mesh );
	std::cout << "Mesh volume = " << volume << std::endl;
	
	return 0;
}
