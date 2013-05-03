/*
	matrix4x4.h

	matrix class

	mnemonic 2004
*/
#ifndef __MATRIX4X4
#define __MATRIX4X4

class matrix4x4
{
public:
	matrix4x4();
	matrix4x4( float m[4][4] );
	matrix4x4( float m[16] );
	~matrix4x4();

	// asign
	matrix4x4 &operator = ( const matrix4x4 &A );

	// matrix multiplication
	friend matrix4x4 operator * ( matrix4x4 A, matrix4x4 B );
	
	// direct access (rw)
	float& operator() ( int col, int row )
	{
		return mat[col][row];
	};

	void debug_out();
	
private:
	float mat[4][4];
};

#endif
