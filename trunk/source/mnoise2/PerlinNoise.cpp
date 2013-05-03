/*
	PerlinNoise.cpp

	mnemonic 2004

	based on code by matt zucker
*/

#include "PerlinNoise.h"

#include <math.h>
#include <time.h>

namespace {
#ifdef PERLINNOISE_USE_ICQ_FRAND
	// nice direct float RNG from Inigo Quilez
	// http://iquilezles.org/www/articles/sfrand/sfrand.htm
	float sfrand( int *seed )
	{
		float res;

		seed[0] *= 16807;

		*((unsigned int *) &res) = ( ((unsigned int)seed[0])>>9 ) | 0x40000000;

		return( res-3.0f );
	}
#else
	// deterministic RNG
	static unsigned long myrandnext = 1;
	
	int myrand()
	{
		myrandnext = myrandnext * 1103515245 + 12345;
		return((unsigned)(myrandnext/65536) % 32768); // RAND_MAX assumed 32767 
	}

	void mysrand( unsigned seed ) 
	{
		myrandnext = seed;
	}
#endif
} // anonymous namespace

////////////////////////////////////////////////////////////////////////////////
// initialize static variables

bool     PerlinNoise::initialized = false;
unsigned PerlinNoise::permutation_table[ PerlinNoise::WRAP_INDEX*2 + 2 ]= { 0 };
float PerlinNoise::gradient_table1d[ PerlinNoise::WRAP_INDEX*2 + 2 ]    = { 0 };
float PerlinNoise::gradient_table2d[ PerlinNoise::WRAP_INDEX*2 + 2 ][2] = { 0 };
float PerlinNoise::gradient_table3d[ PerlinNoise::WRAP_INDEX*2 + 2 ][3] = { 0 };
int PerlinNoise::cur_seed = 0;

////////////////////////////////////////////////////////////////////////////////

inline float PerlinNoise::frand()
{
#ifdef PERLINNOISE_USE_ICQ_FRAND	
	return sfrand( &cur_seed );
#else
	return (float)( (myrand() % (2*WRAP_INDEX)) - WRAP_INDEX ) / (float)WRAP_INDEX;
#endif
}

////////////////////////////////////////////////////////////////////////////////

void PerlinNoise::normalize2d( float v[2] )
{
	float length = (float) sqrt( (v[0]*v[0]) + (v[1]*v[1]) );
	v[0] /= length;
	v[1] /= length;
}

////////////////////////////////////////////////////////////////////////////////

void PerlinNoise::normalize3d( float v[3] )
{
	float length = (float) sqrt( (v[0]*v[0]) + (v[1]*v[1]) + (v[2]*v[2]) );
	v[0] /= length;
	v[1] /= length;
	v[2] /= length;
}

////////////////////////////////////////////////////////////////////////////////

// mnemonics used in the following 3 functions:
//   l or L = left		(-X direction)
//   r or R = right  	(+X direction)
//   d or D = down   	(-Y direction)
//   u or U = up     	(+Y direction)
//   b or B = backwards	(-Z direction)
//   f or F = forwards  (+Z direction)
//

// create 1d coherent noise

float PerlinNoise::noise1d( float p[1] )
{
	int grid_point_l=0, grid_point_r=0;
	float dist_from_l=0, dist_from_r=0, sX=0, t=0, u=0, v=0;

	if( !initialized ) 
	{
		reseed();
	}

	// find out neighboring grid points to pos and signed distances from pos to them
	setup_values( &t, 0, &grid_point_l, &grid_point_r, &dist_from_l, &dist_from_r, p );

	sX = ease_curve( dist_from_l );

	// u, v, are the vectors from the grid pts. times the random gradients for the grid points
	// they are actually dot products, but this looks like scalar multiplication
	u = dist_from_l * gradient_table1d[ permutation_table[ grid_point_l ] ];
	v = dist_from_r * gradient_table1d[ permutation_table[ grid_point_r ] ];

	// return the linear interpretation between u and v (0 = u, 1 = v) at sX.
	return linear_interp( sX, u, v );
}

////////////////////////////////////////////////////////////////////////////////
// create 2d coherent noise

float PerlinNoise::noise2d( float pos[2] ) 
{
	int grid_point_l=0, grid_point_r=0, grid_point_d=0, grid_point_u=0;
	int indexLD=0, indexRD=0, indexLU=0, indexRU=0;
	float dist_from_l=0, dist_from_r=0, dist_from_d=0, dist_from_u=0;
	float *q=0, sX=0, sY=0, a=0, b=0, t=0, u=0, v=0;
	register int indexL, indexR;

	if (! initialized) { reseed(); }  

	// find out neighboring grid points to pos and signed distances from pos to them.
	setup_values( &t, 0, &grid_point_l, &grid_point_r, &dist_from_l, &dist_from_r, pos );
	setup_values( &t, 1, &grid_point_d, &grid_point_u, &dist_from_d, &dist_from_u, pos );

	// Generate some temporary indexes associated with the left and right grid values
	indexL = permutation_table[ grid_point_l ];
	indexR = permutation_table[ grid_point_r ];

	// Generate indexes in the permutation table for all 4 corners
	indexLD = permutation_table[ indexL + grid_point_d ];
	indexRD = permutation_table[ indexR + grid_point_d ];
	indexLU = permutation_table[ indexL + grid_point_u ];
	indexRU = permutation_table[ indexR + grid_point_u ];
	
	// Get the s curves at the proper values
	sX = ease_curve(dist_from_l);
	sY = ease_curve(dist_from_d);

	// Do the dot products for the lower left corner and lower right corners.
	// Interpolate between those dot products value sX to get a.
	q = gradient_table2d[indexLD]; u = dot2( dist_from_l, dist_from_d, q );
	q = gradient_table2d[indexRD]; v = dot2( dist_from_r, dist_from_d, q );
	a = linear_interp(sX, u, v);

	// Do the dot products for the upper left corner and upper right corners.
	// Interpolate between those dot products at value sX to get b.
	q = gradient_table2d[indexLU]; u = dot2( dist_from_l, dist_from_u, q );
	q = gradient_table2d[indexRU]; v = dot2( dist_from_r, dist_from_u, q );
	b = linear_interp( sX, u, v );

	// Interpolate between a and b at value sY to get the noise return value.
	return linear_interp( sY, a, b );
}

////////////////////////////////////////////////////////////////////////////////
// create 3d coherent noise

float PerlinNoise::noise3d( float pos[2] )
{
	int grid_point_l=0, grid_point_r=0, grid_point_d=0, grid_point_u=0, grid_point_b=0, grid_point_f=0;
	int indexLD=0, indexLU=0, indexRD=0, indexRU=0;
	float dist_from_l=0, dist_from_r=0, dist_from_d=0, dist_from_u=0, dist_from_b=0, dist_from_f=0;
	float *q=0, sX=0, sY=0, sZ=0, a=0, b=0, c=0, d=0, t=0, u=0, v=0;
	register int indexL, indexR;

	if (! initialized) { reseed(); }  

	// find out neighboring grid points to pos and signed distances from pos to them.
	setup_values(&t, 0, &grid_point_l, &grid_point_r, &dist_from_l, &dist_from_r, pos);
	setup_values(&t, 1, &grid_point_d, &grid_point_u, &dist_from_d, &dist_from_u, pos);
	setup_values(&t, 2, &grid_point_b, &grid_point_f, &dist_from_b, &dist_from_f, pos);
  
	indexL = permutation_table[ grid_point_l ];
	indexR = permutation_table[ grid_point_r ];

	indexLD = permutation_table[ indexL + grid_point_d ];
	indexRD = permutation_table[ indexR + grid_point_d ];
	indexLU = permutation_table[ indexL + grid_point_u ];
	indexRU = permutation_table[ indexR + grid_point_u ];

	sX = ease_curve(dist_from_l);
	sY = ease_curve(dist_from_d);
	sZ = ease_curve(dist_from_b);

	q = gradient_table3d[indexLD+grid_point_b]; u = dot3(dist_from_l, dist_from_d, dist_from_b, q);
	q = gradient_table3d[indexRD+grid_point_b]; v = dot3(dist_from_r, dist_from_d, dist_from_b, q);
	a = linear_interp(sX, u, v);

	q = gradient_table3d[indexLU+grid_point_b]; u = dot3(dist_from_l, dist_from_u, dist_from_b, q);
	q = gradient_table3d[indexRU+grid_point_b]; v = dot3(dist_from_r, dist_from_u, dist_from_b, q);
	b = linear_interp(sX, u, v);

	c = linear_interp(sY, a, b);

	q = gradient_table3d[indexLD+grid_point_f]; u = dot3(dist_from_l, dist_from_d, dist_from_f, q);
	q = gradient_table3d[indexRD+grid_point_f]; v = dot3(dist_from_r, dist_from_d, dist_from_f, q);
	a = linear_interp(sX, u, v);

	q = gradient_table3d[indexLU+grid_point_f]; u = dot3(dist_from_l, dist_from_u, dist_from_f, q);
	q = gradient_table3d[indexRU+grid_point_f]; v = dot3(dist_from_r, dist_from_u, dist_from_f, q);
	b = linear_interp(sX, u, v);

	d = linear_interp(sY, a, b);

	return linear_interp(sZ, c, d);
}

////////////////////////////////////////////////////////////////////////////////

float PerlinNoise::noise( float p1 )
{
	return noise1d( &p1 );
}

float PerlinNoise::noise( float p1, float p2 )
{
	float p[2] = { p1, p2 };
	return noise2d( p );
}

float PerlinNoise::noise( float p1, float p2, float p3 )
{
	float p[3] = { p1, p2, p3 };
	return noise3d( p );
}

////////////////////////////////////////////////////////////////////////////////

void PerlinNoise::reseed()
{
#ifdef PERLINNOISE_USE_ICQ_FRAND
	cur_seed = (unsigned int)time(NULL);
#else
	cur_seed = (unsigned int) (time(NULL) + myrand());
	mysrand( cur_seed );
#endif
	gen_luts();
}

void PerlinNoise::reseed( unsigned seed )
{
	cur_seed = seed;
	gen_luts();
}

////////////////////////////////////////////////////////////////////////////////

void PerlinNoise::gen_luts()
{
	unsigned i,j,tmp;

	for( i=0; i < WRAP_INDEX; i++ )
	{
		// put index into permutation_table[index], we will shuffle later
		permutation_table[i] = i;

		gradient_table1d[i] = frand();

		for( j=0; j < 2; j++ )
		{
			gradient_table2d[i][j] = frand();
			normalize2d( gradient_table2d[i] );
		}

		for( j=0; j < 3; j++ )
		{
			gradient_table3d[i][j] = frand();
			normalize3d( gradient_table3d[i] );
		}
	}

	// shuffle permutation table up to WRAP_INDEX
	for( i=0; i < WRAP_INDEX; i++ )
	{
		j = rand() & MOD_MASK;
		tmp = permutation_table[i];
		permutation_table[i] = permutation_table[j];
		permutation_table[j] = tmp;
	}

	// Add the rest of the table entries in, duplicating 
	// indices and entries so that they can effectively be indexed
	// by unsigned chars.  I think.  Ask Perlin what this is really doing.

	for( i=0; i < WRAP_INDEX+2; i++ )
	{
		permutation_table[ WRAP_INDEX + i ] = permutation_table[i];
		gradient_table1d[ WRAP_INDEX + i ] = gradient_table1d[i];

		for( j=0; j < 2; j++ )
		{
			gradient_table2d[ WRAP_INDEX + i ][j] = gradient_table2d[i][j];
		}

		for( j=0; j < 3; j++ )
		{
			gradient_table3d[ WRAP_INDEX + i ][j] = gradient_table3d[i][j];
		}
	}

	initialized = true;
}
