#include <iostream>
#include <GL/glew.h>
#include "Geometry.h"

using namespace std;

// Geometry classes
Icosahedron g_ico;
Sphere      g_sphere;
SHBourke    g_sh;
int g_scene=0;

// lighting
GLfloat light0_position[] = {-1.0, 1.0, 1.0, 0.0 };
void setupLight();

//------------------------------------------------------------------------------
// Implementation of glutmain.cpp functions:
//	- bool frame()      which is called every frame
//	- bool init()		which is called once right after GLinit()
//	- void shutdown()	which is called before exiting the application.
//------------------------------------------------------------------------------

extern bool toggle[256]; // keystate as provided by glutmain.cpp

bool frame()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	// toggle scenes
	if( toggle[' '] )	{ g_scene = (g_scene+1) % 3;  toggle[' ']=false; cout << "scene " << g_scene << endl; }
	
	// toggle wireframe
	if( !toggle['W'] ) 
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	else
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	
	// toggle resolution (for icosahedron)
	static int res=3;
	if( toggle['+'] && (res<5) ) { res++;  toggle['+']=false; cout << "res " << res << endl; }
	if( toggle['-'] && (res>0) ) { res--;  toggle['-']=false; cout << "res " << res << endl; }
	
	// render scene
	glColor3f( 1,1,1 );	
	switch( g_scene )
	{
	default:
	case 0:		g_ico   .draw( res );  break;
	case 1:		g_sphere.draw();       break;
	case 2: 	g_sh    .draw();       break;
	}	
	
	return true;
}

bool init()
{
	static int n[2] = { 0, 0 };
	g_sphere.update( 16, n );

	// parameters for SHBourke
	static int m[8] = { 1,3,6,2,5,1,4,1 };; //{ 0,1,0,1,0,1,0,1 };	; //{ 1,3,6,2,5,1,4,1 };
	static int res = 64;
	g_sh.update( res, m );	
	
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
	setupLight();
	
	return true;
}

void shutdown()
{
}


//------------------------------------------------------------------------------

void setupLight()
{
	// gl light 0
	GLfloat light0_ambient[]  = {  .1,  .1,  .1, 1.0 };
	GLfloat light0_diffuse[]  = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat light0_specular[] = { 1.0, 1.0, 1.0, 1.0 };
//	GLfloat light0_position[] = {-0.6, 0.6, 0.6, 0.0 };
	glLightfv( GL_LIGHT0, GL_AMBIENT,  light0_ambient  );
	glLightfv( GL_LIGHT0, GL_DIFFUSE,  light0_diffuse  );
	glLightfv( GL_LIGHT0, GL_SPECULAR, light0_specular );
	glLightfv( GL_LIGHT0, GL_POSITION, light0_position );
	glEnable( GL_LIGHT0 );
	glEnable( GL_LIGHTING );

	// gl states 
	glEnable( GL_COLOR_MATERIAL );
	glColor4f( 1,1,1,1 );
	glShadeModel( GL_SMOOTH );
}