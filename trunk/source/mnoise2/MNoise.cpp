#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include "MNoise.h"
#include "primitives.h"		// draw_aabb( min,max )

#ifdef MNOISE_USE_IMPROVEDNOISE
	#include "ImprovedNoise.h"
#else
	#include "PerlinNoise.h"
#endif

/******************************************************************************/
/**
 Example settings for MCscale and MCsize for power of two grid size
 \verbatim
 (1./16., 16)   correct 16^3 field
 (1./8  , 16)   'painterly effect'
 (1./32., 16)   'checkerboard'
 (1.    , 16)   'grid lost chaos'
 \endverbatim

 For correct rendering the parameters wrt. grid size are:
 MC size = 1/2 power of two size
 scale = (MC size)^(-1)

 Also try:
 \code
  MNoise::MNoise():MarchingCubes( 0.5, 1.0/16.0, 16 )
  MNoise::MNoise():MarchingCubes( 0.5, 1.0/32.0, 10 )
	MNoise::MNoise():MarchingCubes( 0.5, 17.0/16.0, 32 )
	MNoise::MNoise():MarchingCubes( 0.5, 1.0/32.0, 32 )
 \endcode
*/
MNoise::MNoise( int size_, float MCscale, int MCsize )
	: MarchingCubes( 0.5, MCscale, MCsize ),
	  root(NULL),
	  frust(NULL)
{
	root = new Octree( vector3(-1,-1,-1), vector3(1,1,1) );
	size = size_; 
	posz = posy = posz = 1.23456789f;
	scale = 1;	
	octaves = 2;
	persistance = 0.75;
	mode = 1;

#ifndef MNOISE_USE_IMPROVEDNOISE	
	PerlinNoise::reseed();
#endif	
}

/******************************************************************************/
MNoise::~MNoise()
{
	if( root ) delete root; root=0;
}

/******************************************************************************/
void MNoise::draw()
{
//	root->draw_leafs( frust );
//	glColor3f( 1,0,0 );
//	draw_marchingcubes();	

	Leaves leaves;

	root->give_visible_leaves( frust, &leaves );

	cubecount=0;
	while( !leaves.empty() )
	{
		Node l = leaves.top();
		
		marchcube( l.aabb_min[0], l.aabb_min[1], l.aabb_min[2] );
		
		cubecount++;
		
		leaves.pop();
	}	
	
/*
 		for( int x=0; x < datasize; x++ )
		for( int y=0; y < datasize; y++ )
		for( int z=0; z < datasize; z++ )
			marchcube( (x-datasize/2)*scale, 
			           (y-datasize/2)*scale,
			           (z-datasize/2)*scale );
 */
}

/******************************************************************************/
int MNoise::build()
{
	Octree* node = root;
	vector3 min, max;
	
	// build complete (means all leafs are in the same depth) octree
	root->build_complete( size );
	
	return 0;
}

/******************************************************************************/

/* better in class lvl_noise */
float MNoise::fabsnoise( float x, float y, float z )
{
	int i;
	float amplitude = 1.0;
	float result = 0.0;
//	int octaves = 3;
//	float persistance = 0.75;
	for( i=0; i < octaves; i++ ) 
	{
	  #ifdef MNOISE_USE_IMPROVEDNOISE		
		result += ImprovedNoise::noise(x*scale, y*scale, z*scale) * amplitude;
	  #else
		result += PerlinNoise::noise(x, y, z) * amplitude;
	  #endif
		amplitude *= persistance;
		//x *= 2.0; y *= 2.0; z *= 2.0;
	}
	return result;
}
	
float MNoise::sample( float x, float y, float z )
{
	float eps = 0.001f;
	if( (fabs(x)<=eps) && (fabs(y)<=eps) && (fabs(z)<=eps) ) return 0;

#if 0
	// test simple noise without octaves, unscaled
	float noise = ImprovedNoise::noise(x+posx,y+posy,z+posz);
#else
	float noise = fabsnoise( x+posx,y+posy,z+posz );
#endif
	float ret = noise;

	switch( mode )
	{
		case 1:	// center-sphere cut-out
			ret =  fabs(noise) - ( 0.1f / (x*x + y*y + z*z));
			break;
		
		case 2: // jabberwokky
			ret = sin(noise*4) - ( 0.01f / (x*x + y*y + z*z));
			break;
		
		// test alternative modes
		case 3: 
			ret = sin(acos(noise*4));
			break;
		
		default:
			ret = noise;
	}
	
#if 0
	if( noise < -1 ) noise = -1;
	if( noise >  1 ) noise =  1;
#endif

	return ret;
}

/******************************************************************************/
void MNoise::Octree::draw_leaves( Frustum* frust )
{
	// test visibility (frustum clipping)
	if( frust )
	{
		int vis = frust->clip_aabb( aabb_min.get(), aabb_max.get() );
		
		// node outside of viewing frustum
		if( vis < 0 ) 
#ifdef DEBUG_CUBE
			glColor3f( .2f,0,0 ); else glColor3f( 1,0,0 );
#else
			return;
#endif
	}
	
	if( is_leaf() )
	{
		draw_aabb( aabb_min, aabb_max );
	}
	else
	{
		for( int i=0; i < 8; i++ )  if( sons[i] )
		{
			sons[i]->draw_leaves( frust );
		}
	}
}

/******************************************************************************/
void MNoise::Octree::give_visible_leaves( Frustum* frust, 
                                          std::stack<Node>* leaves )
{
	// test visibility (frustum clipping)
	if( frust )
	{
		int vis = frust->clip_aabb( aabb_min.get(), aabb_max.get() );
		
		// node outside of viewing frustum
		if( vis < 0 ) return;
	}
	
	if( is_leaf() )
	{
		//leaf l; l.aabb_min = aabb_min; l.aabb_max = aabb_max;		
		//leaves->push( l );
		leaves->push( (Node)*this );
	}
	else
	{
		for( int i=0; i < 8; i++ )  if( sons[i] )
		{
			sons[i]->give_visible_leaves( frust, leaves );
		}
	}
}

/******************************************************************************/
void MNoise::Octree::build_complete( int level )
{
	if( level==0 ) return;
		
	vector3 d = (aabb_max - aabb_min) / 2.f;
	vector3 center = aabb_min + d;
		
	sons[0] = new Octree( aabb_min, center );
	sons[1] = new Octree( aabb_min + vector3( d[0],   0,  0 ) , center + vector3( d[0],   0,   0 ) );
	sons[2] = new Octree( aabb_min + vector3( d[0],d[1],  0 ) , center + vector3( d[0],d[1],   0 ) );
	sons[3] = new Octree( aabb_min + vector3(    0,d[1],  0 ) , center + vector3(    0,d[1],   0 ) );
	sons[4] = new Octree( aabb_min + vector3(    0,   0,d[2] ), center + vector3(    0,   0,d[2] ) );
	sons[5] = new Octree( aabb_min + vector3( d[0],   0,d[2] ), center + vector3( d[0],   0,d[2] ) );
	sons[6] = new Octree( aabb_min + vector3( d[0],d[1],d[2] ), center + vector3( d[0],d[1],d[2] ) );
	sons[7] = new Octree( aabb_min + vector3(    0,d[1],d[2] ), center + vector3(    0,d[1],d[2] ) );

	for( int i=0; i < 8; i++ )
		sons[i]->build_complete( level - 1 );		
}

