/*
	vector3.h

	vector class

	mnemonic 2004
*/
#ifndef __VECTOR3
#define __VECTOR3

#include <math.h>	// sqrt()

class vector3
{
public:
	vector3();
	vector3( float vec[3] );
	vector3( float x, float y, float z );
	~vector3();

	// assign
	vector3 &operator = ( const vector3 &vec );

	// access
	// pay attention that coord is 0,1 or 2 !!
	float operator [] ( int coord );

	// add/sub/scalar operations
	friend vector3 operator + ( vector3 a, vector3 b );
	friend vector3 operator - ( vector3 a, vector3 b );
	friend vector3 operator * ( vector3 a, float scalar );
	friend vector3 operator / ( vector3 a, float scalar );

	// cross product *
	friend vector3 operator * ( vector3 a, vector3 b );

	// dot product %
	friend float operator % ( vector3 a, vector3 b );

	// returns pointer to float[3] coordinate field
	float* const get()
	{
		return v;
	};

	void set( const float x, const float y, const float z )
	{
		v[0] = x;
		v[1] = y;
		v[2] = z;
	};

	void set( const float vec[3] )
	{
		v[0] = vec[0];
		v[1] = vec[1];
		v[2] = vec[2];
	};
	
	// returns |v|
	float magnitude()
	{
		return( (float) sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] ) );
	};
	
	void normalize()
	{
		float m = magnitude();
		
		if( m != 0 )
		{
			v[0] /= m;
			v[1] /= m;
			v[2] /= m;
		}		
	};
	
private:
	float v[3];
};

#endif
