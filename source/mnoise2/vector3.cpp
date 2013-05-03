/*
	vector3.cpp

	vector class

	mnemonic 2004
*/

#include "vector3.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

// CONSTRUCTORS / DESTRUCTOR

vector3::vector3()
{
}

vector3::vector3( float vec[3] )
{
	for( int i=0; i < 3; i++ )
		v[i] = vec[i];
}

vector3::vector3( float x, float y, float z )
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

vector3::~vector3()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////

// ASSIGN

vector3 &vector3::operator = ( const vector3 &vec )
{
	for( int i=0; i < 3; i++ )
		v[i] = vec.v[i];

	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

// ACCESS

float vector3::operator [] ( int coord )
{
	return v[coord];
}

/////////////////////////////////////////////////////////////////////////////////////////////////

// ADDITION / SUBTRACTION

vector3 operator + ( vector3 a, vector3 b )
{
	return vector3( a[0]+b[0], a[1]+b[1], a[2]+b[2] );
}

vector3 operator - ( vector3 a, vector3 b )
{
	return vector3( a[0]-b[0], a[1]-b[1], a[2]-b[2] );
}

// SCALAR OPERATIONS

vector3 operator * ( vector3 a, float scalar )
{
	return vector3( a[0]*scalar, a[1]*scalar, a[2]*scalar );
}

vector3 operator / ( vector3 a, float scalar )
{
	return vector3( a[0]/scalar, a[1]/scalar, a[2]/scalar );
}

/////////////////////////////////////////////////////////////////////////////////////////////////

// CROSS PRODUCT

vector3 operator * ( vector3 a, vector3 b )
{
	return vector3( a[1]*b[2] - a[2]*b[1],
					a[2]*b[0] - a[0]*b[2],
					a[0]*b[1] - a[1]*b[0] );
}

// DOT PRODUCT

float operator % ( vector3 a, vector3 b )
{
	return ( a[0]*b[0] + a[1]*b[1] + a[2]*b[2] );
}
