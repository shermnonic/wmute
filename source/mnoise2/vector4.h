/*
	vector4.h

	vector class to work with 4x4 matrices

	mnemonic 2004
*/
#ifndef __VECTOR4
#define __VECTOR4

#include "matrix4x4.h"

class vector4
{
public:
	vector4();
	vector4( float vec[4] );
	vector4( float x, float y, float z, float t );
	~vector4();

	// assign
	vector4 &operator = ( const vector4 &vec );
//	vector4 &operator = ( const vector3 &vec );

	// access
	// pay attention that coord is 0,1,2 or 3 !!
	float operator [] ( int coord );

	// add/sub/scalar operations
	friend vector4 operator + ( vector4 a, vector4 b );
	friend vector4 operator - ( vector4 a, vector4 b );
	friend vector4 operator * ( vector4 a, float scalar );
	friend vector4 operator / ( vector4 a, float scalar );

	// mult with matrix
	friend vector4 operator * ( vector4 v, matrix4x4 m );

	// dot product %
	friend float operator % ( vector4 a, vector4 b );

	// returns pointer to float[4] coordinate field
	float* get()
	{
		return v;
	};

	void set( float x, float y, float z, float t )
	{
		v[0] = x;
		v[1] = y;
		v[2] = z;
		v[3] = t;
	};

	void set( float vec[4] )
	{
		v[0] = vec[0];
		v[1] = vec[1];
		v[2] = vec[2];
		v[3] = vec[3];
	};

private:
	float v[4];
};

#endif
