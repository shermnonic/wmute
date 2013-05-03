#include "primitives.h"

/******************************************************************************/
void draw_aabb( vector3 min, vector3 max )
{
	glBegin( GL_QUADS );

	// left face
	glVertex3f( min[0], min[1], min[2] );
	glVertex3f( min[0], min[1], max[2] );
	glVertex3f( min[0], max[1], max[2] );
	glVertex3f( min[0], max[1], min[2] );

	// right face
	glVertex3f( max[0], min[1], min[2] );
	glVertex3f( max[0], min[1], max[2] );
	glVertex3f( max[0], max[1], max[2] );
	glVertex3f( max[0], max[1], min[2] );

	// front face
	glVertex3f( min[0], min[1], min[2] );
	glVertex3f( max[0], min[1], min[2] );
	glVertex3f( max[0], max[1], min[2] );
	glVertex3f( max[0], min[1], min[2] );

	// back face
	glVertex3f( min[0], min[1], max[2] );
	glVertex3f( max[0], min[1], max[2] );
	glVertex3f( max[0], max[1], max[2] );
	glVertex3f( max[0], min[1], max[2] );

	// top face
	glVertex3f( min[0], max[1], min[2] );
	glVertex3f( min[0], max[1], max[2] );
	glVertex3f( max[0], max[1], max[2] );
	glVertex3f( max[0], max[1], min[2] );

	// bottom face
	glVertex3f( min[0], min[1], min[2] );
	glVertex3f( min[0], min[1], max[2] );
	glVertex3f( max[0], min[1], max[2] );
	glVertex3f( max[0], min[1], min[2] );

	glEnd();
}

/******************************************************************************/
void draw_cube( float x, float y, float z, 
				float scalex, float scaley, float scalez, bool color )
{
	glPushMatrix();
	glTranslatef( x,y,z );

	glBegin( GL_TRIANGLE_STRIP );
		if( color ) glColor3f( 1.0f, 0.0f, 0.0f );
		glNormal3f( 0, 0, -1 );
		glVertex3f( -1.0f*scalex, -1.0f*scaley, -1.0f*scalez );
		glVertex3f(  1.0f*scalex, -1.0f*scaley, -1.0f*scalez );
		glVertex3f( -1.0f*scalex,  1.0f*scaley, -1.0f*scalez );
		glVertex3f(  1.0f*scalex,  1.0f*scaley, -1.0f*scalez );
	glEnd();

	glBegin( GL_TRIANGLE_STRIP );
		if( color ) glColor3f( 0.0f, 1.0f, 0.0f );
		glNormal3f( -1, 0, 0 );
		glVertex3f( -1.0f*scalex, -1.0f*scaley,  1.0f*scalez );
		glVertex3f( -1.0f*scalex, -1.0f*scaley, -1.0f*scalez );
		glVertex3f( -1.0f*scalex,  1.0f*scaley,  1.0f*scalez );
		glVertex3f( -1.0f*scalex,  1.0f*scaley, -1.0f*scalez );
	glEnd();

	glBegin( GL_TRIANGLE_STRIP );
		if( color ) glColor3f( 0.0f, 1.0f, 0.0f );
		glNormal3f( 1, 0, 0 );
		glVertex3f( 1.0f*scalex, -1.0f*scaley,  1.0f*scalez );
		glVertex3f( 1.0f*scalex, -1.0f*scaley, -1.0f*scalez );
		glVertex3f( 1.0f*scalex,  1.0f*scaley,  1.0f*scalez );
		glVertex3f( 1.0f*scalex,  1.0f*scaley, -1.0f*scalez );
	glEnd();

	glBegin( GL_TRIANGLE_STRIP );
		if( color ) glColor3f( 0.0f, 0.0f, 1.0f );
		glNormal3f( 0, 1, 0 );
		glVertex3f( -1.0f*scalex, 1.0f*scaley,  1.0f*scalez );
		glVertex3f(  1.0f*scalex, 1.0f*scaley,  1.0f*scalez );
		glVertex3f( -1.0f*scalex, 1.0f*scaley, -1.0f*scalez );
		glVertex3f(  1.0f*scalex, 1.0f*scaley, -1.0f*scalez );
	glEnd();

	glBegin( GL_TRIANGLE_STRIP );
		if( color ) glColor3f( 0.0f, 0.0f, 1.0f );
		glNormal3f( 0, -1, 0 );
		glVertex3f( -1.0f*scalex, -1.0f*scaley,  1.0f*scalez );
		glVertex3f(  1.0f*scalex, -1.0f*scaley,  1.0f*scalez );
		glVertex3f( -1.0f*scalex, -1.0f*scaley, -1.0f*scalez );
		glVertex3f(  1.0f*scalex, -1.0f*scaley, -1.0f*scalez );
	glEnd();

	glBegin( GL_TRIANGLE_STRIP );
		if( color ) glColor3f( 1.0f, 0.0f, 0.0f );
		glNormal3f( 0,0,1.f );
		glVertex3f( -1.0f*scalex, -1.0f*scaley, 1.0f*scalez );
		glVertex3f(  1.0f*scalex, -1.0f*scaley, 1.0f*scalez );
		glVertex3f( -1.0f*scalex,  1.0f*scaley, 1.0f*scalez );
		glVertex3f(  1.0f*scalex,  1.0f*scaley, 1.0f*scalez );
	glEnd();

	glPopMatrix();
}

