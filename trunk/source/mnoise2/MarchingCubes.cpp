#include "MarchingCubes.h"
#include "MarchingCubes_tables.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

// example sample function
float MarchingCubes::sample( float x, float y, float z )
{
	double result = 0.0;

	struct ball
	{
		float x,y,z,radius;
	};
	
	const ball s[3] =
	{
		{ 0.5,0.5,0.5, 1.5 },
		{ 0.3,0.5,0.2, 1.0 },
		{ 1,-1,1, 1.5 }
	};
	
	double dx,dy,dz;
	
	
	for( int i=0; i < 3; i++ )
	{
		dx = x - s[i].x;
		dy = y - s[i].y;
		dz = z - s[i].z;
		result += s[i].radius / (dx*dx + dy*dy + dz*dz);	
	}	
	
	return result;
}

// get_offset finds the approximate point of intersection of the surface
// between two points with the values val1 and val2
float MarchingCubes::get_offset( float val1, float val2, float desired )
{
	double delta = val2 - val1;
	
	if( delta == 0.f )
	{
		return 0.5f;
	}
	
	return (float)(desired - val1) / (float) delta;
}

// perform marching cubes algorithm on a single cube
void MarchingCubes::marchcube( float x, float y, float z )
{
	float 	cube[8];
	int 	index=0;
	int		edgeflags;
	vector3	edgeverts[12];
	vector3 normals[12];
	int 	i,j;
	
	for( i=0; i < 8; i++ )
	{
		// local copy of cube values for intersection-calc
		cube[i] = sample( x + cube_ofs[i][0]*scale, 
		                  y + cube_ofs[i][1]*scale, 
		                  z + cube_ofs[i][2]*scale );

		// build index
		if( cube[i] < isovalue ) index |= 1<<i;
	}
	
	// find which edges are intersected by the surface
	edgeflags = edge_tab[ index ];
	
	// cube completely inside/outside -> no intersections
	if( edgeflags == 0 ) return;
	
	// find intersection surface-edge 
	for( i=0; i < 12; i++ ) if( edgeflags & (1<<i) )
	{
		float ofs = get_offset( cube[cube_con[i][0]], cube[cube_con[i][1]], 
		                        isovalue );
		
		edgeverts[i].set( 
		  x + ( cube_ofs[ cube_con[i][0] ][0] + ofs * cube_dir[i][0] ) * scale,
		  y + ( cube_ofs[ cube_con[i][0] ][1] + ofs * cube_dir[i][1] ) * scale,
		  z + ( cube_ofs[ cube_con[i][0] ][2] + ofs * cube_dir[i][2] ) * scale
		);
		
		float delta = 0.001;
		
		if( compute_normals )
		{
			normals[i].set(
			  sample( edgeverts[i][0]-delta, edgeverts[i][1], edgeverts[i][2] ) 
		    - sample( edgeverts[i][0]+delta, edgeverts[i][1], edgeverts[i][2] ),

			  sample( edgeverts[i][0], edgeverts[i][1]-delta, edgeverts[i][2] ) 
		 	- sample( edgeverts[i][0], edgeverts[i][1]+delta, edgeverts[i][2] ),

			  sample( edgeverts[i][0], edgeverts[i][1], edgeverts[i][2]-delta )
			- sample( edgeverts[i][0], edgeverts[i][1], edgeverts[i][2]+delta )		  
			);
			
			normals[i].normalize();
		}
	}
	
#if 1
	// draw triangles
	glBegin( GL_TRIANGLE_STRIP );
	for( i=0; i < 5; i++ ) if( tri_tab[index][3*i] >= 0 )
	{		
		for( j=0; j < 3; j++ )
		{
			int vert = tri_tab[index][3*i+j];
			
			if( compute_normals )
				glNormal3fv( normals[vert].get() );
			glVertex3fv( edgeverts[vert].get() );
		}
	}
	glEnd();
#else
	// Debug visualization: Show sample points		
	glPointSize( 5.f );
	glBegin( GL_POINTS );
	for( i=0; i < 8; i++ )
	{
		// build index
		if( cube[i] < isovalue ) 
		{
			glColor3f( 0.6,0.6,1 );
		}
		else
		{
			glColor3f( 0.1,0.1,.4 );
		}

		// local copy of cube values for intersection-calc
		glVertex3f( x + cube_ofs[i][0]*scale, 
		            y + cube_ofs[i][1]*scale, 
		            z + cube_ofs[i][2]*scale );
	}
	glEnd();
#endif
}
