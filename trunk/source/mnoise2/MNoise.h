#ifndef MNOISE_H
#define MNOISE_H

//#define MNOISE_USE_IMPROVEDNOISE

#include "vector3.h"
#include "Frustum.h"
#include "MarchingCubes.h"
#include <stack>

/** 
  Marching Noise - 3D Perlin noise isosurface

  MNoise holds a complete octree used for frustum culling in which each leaf
  stores the mid-points needed by the marching cubes algorithm.
 */
class MNoise : public MarchingCubes
{
public:
	MNoise( int size_, float MCscale, int MCsize );
	~MNoise();
	
	int  build();
	void draw();
	
	/// Set viewing frustum for culling
	/// Although passed as pointer, Frustum is guaranteed to not be changed.
	void set_frustum( Frustum* f ) { frust = f; };
	
	// sample function for marchingcubes
	float sample( float x, float y, float z ); // FIXME: public?
	
	int   get_cubecount() { return cubecount; };

	void  set_scale( float s ) { scale = s; };
	float get_scale() { return scale; };

	void  set_posz( float z ) { posz = z; };
	float get_posz() const { return posz; };
	void  set_posx( float x ) { posx = x; };
	float get_posx() const  { return posx; };
	void  set_posy( float y ) { posy = y; };
	float get_posy() const  { return posy; };
	
	void  set_octaves    ( int o ) { octaves = o; };
	int   get_octaves    () const  { return octaves; };
	void  set_persistance( float per ) { persistance = per; };
	float get_persistance() const { return persistance; };
	
	/**
	 Set noise process mode:
	 - mode 0: default, no extra processing
	 - mode 1: softly cut-out sphere centered around viewer
	 - mode 2: Jabberwokky!
	 - mode 3: (undocumented)
	*/
	void  set_mode( int m ) { mode = m%4; };
	
protected:
	float fabsnoise( float x, float y, float z );	
	

protected:
	/// Basic octree node type
	struct Node
	{
		vector3 aabb_min, aabb_max;
	};

	typedef std::stack<Node> Leaves;

	/// Recursively defined octree (full)
	class Octree : public Node
	{
		friend class MNoise;

		Octree* sons[8];

	public:
		Octree( vector3 const min, vector3 const max ) 
		{ 
			aabb_min = min;
			aabb_max = max; 
			for( int i=0; i < 8; i++ ) sons[i] = NULL;
		};
	
		~Octree()
		{
			for( int i=0; i < 8; i++ )
				if( sons[i] ) { delete sons[i]; sons[i]=NULL; }
		};
	
		/// create all 8 sons until level==0
		void build_complete( int level );
	
		/// draw all leafs in this subtree
		void draw_leaves( Frustum* frust=NULL );

		bool is_leaf() {
			return( !sons[0]&&!sons[1]&&!sons[2]&&!sons[3]&&
					!sons[4]&&!sons[5]&&!sons[6]&&!sons[7] );
		};

	protected:
		void give_visible_leaves( Frustum* frust, 
			                      std::stack<Node>* leaves );
	};


private:
	Octree*  root;          // root of octree
	Frustum* frust;         // viewing frustum
	int   size;		        // size as power of two (2^size)
	float scale;	
	int   cubecount;        // numer of cubes drawn	
	float posx,posy,posz;
	int   octaves;
	float persistance;
	int   mode;
};

#endif
