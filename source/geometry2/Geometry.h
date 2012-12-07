#ifndef GEOMETRY_H
#define GEOMETRY_H

// TODO: compute vertex+normal buffers (instead of display list; GL decoupling)

// vec3 vector types
#include "Vector.h"

/// Icosahedron subdivided to approximate a sphere
/// Based on code from the OpenGL "Red Book"
/// \todo build display list in update()
class Icosahedron
{
public:
	void draw( int resolution=3 );

private:
	void drawFace( vec3 v1, vec3 v2, vec3 v3 );
	void subdivide( vec3 v1, vec3 v2, vec3 v3, int level=0 );

	int res;
};

class Sphere
{
public:
	Sphere( int resolution=4, vec3 center=vec3(0,0,0) );
	~Sphere();
	
	void draw();
	void update( int resolution, int *new_n=0 );

private:
	int dl;	// display list

	int radius_n[2];
	float radius( float phi, float theta );
	void drawSphere();

	int res;
	vec3 c;
};

/// Spherical Harmonics
/// Based on Code from Paul Bourke
class SHBourke	
{
public:
	SHBourke( int resolution=64, 
	          int m0=1, int m1=3, int m2=6, int m3=2, 
              int m4=5, int m5=1, int m6=4, int m7=1 );
	~SHBourke();

	void draw();
	void update( int resolution, int *new_m=0 );

	struct XYZ { float x; float y; float z; };
	struct RGB { float r; float g; float b; };

private:
	XYZ Eval( double theta, double phi );
	XYZ CalcNormal( XYZ o, XYZ u, XYZ v );
	void Normalise( XYZ *p );

	void DrawFunction( int resolution );

	int dl;		// display list	
	int m[8];	// parameters
	int res; 
};

#endif
