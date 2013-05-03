/*
	PerlinNoise.h

	mnemonic 2004

	based on code by matt zucker
*/
#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#include <stdlib.h>

#define PERLINNOISE_USE_ICQ_FRAND

/**
  Perlin noise

  Based on code from Matt Zucker.
 */
class PerlinNoise 
{
public:
	static void reseed();
	static void reseed( unsigned int seed );

	static float noise1d( float p[1] );
	static float noise2d( float p[2] );
	static float noise3d( float p[3] );

	static float noise( float p1 );
	static float noise( float p1, float p2 );
	static float noise( float p1, float p2,float p3 );

	static int get_seed() { return cur_seed; };

private:
	enum{ WRAP_INDEX=256, MOD_MASK=255, LARGE_PWR2=4096 };
//	enum{ WRAP_INDEX=63111, MOD_MASK=17444, LARGE_PWR2=4096 };

	static bool initialized;
	static int  cur_seed;

	static unsigned permutation_table[ WRAP_INDEX*2 + 2 ];
	static float gradient_table1d[ WRAP_INDEX*2 + 2 ];
	static float gradient_table2d[ WRAP_INDEX*2 + 2 ][2];
	static float gradient_table3d[ WRAP_INDEX*2 + 2 ][3];

	static float frand();	// random float in [-1,1]
	static void normalize2d( float vector[2] );
	static void normalize3d( float vector[3] );
	static void gen_luts();	// generate look up tables

	//--------------------------------------------------
	// inline helpers
	
	static inline float ease_curve( float t )
	{
		return ( t * t * (3.f - 2.f * t) );
	};

	static inline float linear_interp( float t, float a, float b )
	{
		return ( a + t * (b - a) );
	};

	static inline float dot2( float rx, float ry, float q[2] )
	{
		return ( rx * q[0] + ry * q[1] );
	};

	static inline float dot3( float rx, float ry, float rz, float q[3] )
	{
		return ( rx * q[0] + ry * q[1] + rz * q[2] );
	};

	static inline void setup_values( float* t, unsigned axis, int* g0, int* g1, 
										float* d0, float* d1, float p[] )
	{
		*t = p[axis] + LARGE_PWR2;
		*g0 = ((int)*t) & MOD_MASK;
		*g1 = (*g0 + 1) & MOD_MASK;
		*d0 = *t - (int)*t;
		*d1 = *d0 - 1.f;
	};
};

#endif
