/*
	vector4.cpp

	vector class to work with 4x4 matrices

	mnemonic 2004
*/

#include "vector4.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS / DESTRUCTOR

vector4::vector4()
{
}

vector4::vector4( float vec[4] )
{
	for( int i=0; i < 4; i++ )
		v[i] = vec[i];
}

vector4::vector4( float x, float y, float z, float t )
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
	v[3] = t;
}

vector4::~vector4()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// ASSIGN

vector4 &vector4::operator = ( const vector4 &vec )
{
	for( int i=0; i < 4; i++ )
		v[i] = vec.v[i];

	return *this;
}

/*
// convert 3d-vector to 4d-vector by setting t-coordinate to 1
vector4 &vector4::operator = ( const vector3 &vec )
{
	for( int i=0; i < 3; i++ )
		v[i] = vec.v[i];

	v[3] = 1;

	return *this;
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////
// ACCESS

float vector4::operator [] ( int coord )
{
	return v[coord];
}

/////////////////////////////////////////////////////////////////////////////////////////////////

// ADDITION / SUBTRACTION

vector4 operator + ( vector4 a, vector4 b )
{
	return vector4( a[0]+b[0], a[1]+b[1], a[2]+b[2], a[3]+b[3] );
}

vector4 operator - ( vector4 a, vector4 b )
{
	return vector4( a[0]-b[0], a[1]-b[1], a[2]-b[2], a[3]-b[3] );
}

// SCALAR OPERATIONS

vector4 operator * ( vector4 a, float scalar )
{
	return vector4( a[0]*scalar, a[1]*scalar, a[2]*scalar, a[3]*scalar );
}

vector4 operator / ( vector4 a, float scalar )
{
	return vector4( a[0]/scalar, a[1]/scalar, a[2]/scalar, a[3]/scalar );
}

// MULT WITH MATRIX

vector4 operator * ( vector4 v, matrix4x4 m )
{
	return vector4(
			v[0]*m(0,0) + v[1]*m(1,0) + v[2]*m(2,0) + v[3]*m(3,0),
			v[0]*m(0,1) + v[1]*m(1,1) + v[2]*m(2,1) + v[3]*m(3,1),
			v[0]*m(0,2) + v[1]*m(1,2) + v[2]*m(2,2) + v[3]*m(3,2),
			v[0]*m(0,3) + v[1]*m(1,3) + v[2]*m(2,3) + v[3]*m(3,3)
		);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

// DOT PRODUCT

float operator % ( vector4 a, vector4 b )
{
	return ( a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3] );
}
