/*
	matrix4x4.cpp

	matrix class

	mnemonic 2004
*/

#include "matrix4x4.h"
#include <iostream>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS / DESTRUCTOR

matrix4x4::matrix4x4()
{
	// set identity
	for( int row=0; row < 4; row++ )
		for( int col=0; col < 4; col++ )
		{
			if( row==col ) mat[col][row]=1;
			else mat[col][row]=0;
		}
}

matrix4x4::matrix4x4( float m[4][4] )
{
	for( int row=0; row < 4; row++ )
		for( int col=0; col < 4; col++ )
		{
			mat[col][row] = m[col][row];
		}
}

matrix4x4::matrix4x4( float m[16] )
{
	// m is an opengl-matrix:
	//   m[0]  m[4]  m[ 8]  m[12]
	//   m[1]  m[5]  m[ 9]  m[13]
	//   m[2]  m[6]  m[10]  m[14]
	//   m[3]  m[7]  m[11]  m[15]	
	
	int i=0; 
	for( int row=0; row < 4; row++ )
		for( int col=0; col < 4; col++ )
		{			
			mat[col][row] = m[i];
			i++;
		}
}

matrix4x4::~matrix4x4()
{
}

////////////////////////////////////////////////////////////////////////////////
// ASSIGN

matrix4x4 &matrix4x4::operator = ( const matrix4x4 &A )
{
	// make this matrix a copy of matrix A
	for( int row=0; row < 4; row++ )
		for( int col=0; col < 4; col++ )		
		{
			mat[col][row] = A.mat[col][row];
		}

	return *this;
}

////////////////////////////////////////////////////////////////////////////////
// MATRIX MULTIPLICATION

matrix4x4 operator * ( matrix4x4 A, matrix4x4 B )
{
	float C[4][4];

	for( int row=0; row < 4; row++ )
	{
		for( int col=0; col < 4; col++ )
		{
			C[col][row] = 0;
			for( int i=0; i < 4; i++ )
			{
				C[col][row] += A.mat[i][row] * B.mat[col][i];
			}			
		}
	}

	return matrix4x4( C );
}

////////////////////////////////////////////////////////////////////////////////

void matrix4x4::debug_out()
{
	for( int row=0; row < 4; row++ )
	{
		for( int col=0; col < 4; col++ )
		{
			cout << mat[col][row];
			if( col < 3 ) cout << ", ";
		}
		cout << endl;
	}
}
