#include "hraw.h"

#include <fstream>
#include <iostream>

using std::ifstream;
using std::ofstream;
using std::cout;
using std::cerr;
using std::endl;

#ifdef _MSC_VER
    //typedef __uint32 uint32_t;
	//typedef unsigned short uint32_t;
	#include <Windows.h>
	typedef UINT32 uint32_t;
#else
   #include <stdint.h>
#endif

typedef uint32_t UInt; // internal hraw unsigned integer type

//------------------------------------------------------------------------------
bool read_hraw( const char* filename, float*& data, unsigned* size )
{	
	data = NULL;	
	
	// Open file
	ifstream f( filename, ifstream::binary );
	if( !f.is_open() )
	{
		cerr << "Error in read_hraw():"
		        "Could not open file \"" << filename << "\"!" << endl;
		return false;
	}
	
	// Read header <dims size[0] ... size[dims]> as uint32	
	UInt dims;
	f.read(	(char*)&dims, sizeof(UInt) );
	
	if( dims<=0 || dims > 3 )
	{
		f.close();
		cerr << "Error in read_hraw():"
		        "Invalid dimensionality (dims=" << dims << ")!" << endl;
		return false;
	}
	
	UInt size_[3] = { 0, 1, 1 };
	f.read( (char*)size_, dims*sizeof(UInt) );
	
	size[0] = size_[0];
	size[1] = size_[1];
	size[2] = size_[2];	
	
	// Read data
	unsigned n = size[0]*size[1]*size[2];
	data = new float[ n ];
	f.read( (char*)data, n*sizeof(float) );
	
	if( !f )
	{
		cerr << "Error in read_hraw():"
		        "Only " << f.gcount()/sizeof(float) << " / " << n/sizeof(float) 
				<< " floats could be read from \"" << filename << "\"!" << endl;
	}
	
	f.close();
	return true;
}

//------------------------------------------------------------------------------
void write_hraw( const char* filename, const float* data, 
                 unsigned sizeX, unsigned sizeY, unsigned sizeZ )
{
	unsigned size[3];
	size[0] = sizeX;
	size[1] = sizeY;
	size[2] = sizeZ;
	write_hraw( filename, data, size );
}


//------------------------------------------------------------------------------
void write_hraw( const char* filename, const float* data, const unsigned* size )
{
	// Open file
	ofstream f( filename, ofstream::binary );
	if( !f.is_open() )
	{
		cerr << "Error in write_hraw():"
		        "Could not open file \"" << filename << "\"!" << endl;
		return;
	}
	
	// Write header
	UInt dims;
	dims = (size[0]>1?1:0) + (size[1]>1?1:0) + (size[2]>1?1:0);
	
	UInt size_[3];
	size_[0] = size[0];
	size_[1] = size[1];
	size_[2] = size[2];	
	
	f.write( (char*)dims , sizeof(UInt)   );
	f.write( (char*)size_, sizeof(UInt)*3 );
	
	// Write data
	unsigned n = size[0]*size[1]*size[2];
	f.write( (char*)data, n*sizeof(float) );
	
	f.close();
}
