#ifndef MNC_PRIMITIVES
#define MNC_PRIMITIVES

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include "vector3.h"

void draw_cube( float x, float y, float z, 
				float scalex, float scaley, float scalez, bool color=false );
				
void draw_aabb( vector3 min, vector3 max );

#endif
