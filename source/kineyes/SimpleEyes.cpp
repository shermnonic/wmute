#include "SimpleEyes.h"
#include <GL/glut.h>
#include <GL/GLU.h> // gluSphere()
#include <cmath>
#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

int makeSphereDisplayList( float radius=1.0 )
{
	GLUquadricObj* sphere = gluNewQuadric();
	gluQuadricDrawStyle( sphere, GLU_FILL ); //GLU_LINE );
	gluQuadricTexture( sphere, FALSE );
	gluQuadricNormals( sphere, GLU_SMOOTH );

	int dl = glGenLists(1);
	glNewList( dl, GL_COMPILE );
	gluSphere( sphere, radius, 16,16 );
	glEndList();
	
	gluDeleteQuadric( sphere );
	return dl;
}

void SimpleEye::init()
{
	m_eyeBall    = makeSphereDisplayList( m_eyeRadius );
	m_eyePupille = makeSphereDisplayList( m_eyeRadius/2.3f );
	m_initialized = true;
}

void SimpleEye::destroy()
{
	glDeleteLists( m_eyeBall, 1 );
	glDeleteLists( m_eyePupille, 1 );
}

void SimpleEye::draw()
{
	if( !m_initialized )
		init();

	// Compute eyeball rotation
	float dx = m_poix - m_posx,
		  dy = m_poiy - m_posy,
		  dz = m_poiz - m_posz;
	float d = sqrt(dx*dx+dy*dy+dz*dz);
	float theta_z = (fabs(dz)>0.001) ? acos( dz / d ) : 0.f;
	float theta_y = (fabs(dy)>0.001) ? acos( dy / d ) : 0.f;

	float sign_x = (dx > 0.f) ? 1.f : -1.f,
	      sign_y = (dy > 0.f) ? 1.f : -1.f;
	
	// Immediate mode rendering
	glPushMatrix();
	
	glTranslatef( m_posx, m_posy, m_posz );
	glRotatef( 180.f*sign_x*theta_z/M_PI, 0,1,0 );
	//glRotatef( 180.f*sign_y*theta_y/M_PI, 1,0,0 );
	glColor3f( 1,1,1 );
	glCallList( m_eyeBall );

	glTranslatef( 0,0, m_eyeRadius );
	glColor3f( 0,0,1 );
	glCallList( m_eyePupille );
	
	glPopMatrix();
}
