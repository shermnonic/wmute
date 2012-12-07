#include "Geometry.h"
#include <iostream>
#include <GL/glew.h>   // replaces gl.h

// standard math
#include <cmath>
#ifndef PI
#define PI (double)3.1415926
#endif
#ifndef TWOPI
#define TWOPI (double)2*3.1415926
#endif
#ifndef M_PI
#define M_PI(float) 3.1415926f
#endif

//------------------------------------------------------------------------------
// 	Icosahedron
//------------------------------------------------------------------------------

void Icosahedron::drawFace( vec3 v1, vec3 v2, vec3 v3 )
{
	vec3 n1,n2,n3;
	n1=v1; n1.normalize();
	n2=v2; n2.normalize();
	n3=v3; n3.normalize();

	glBegin( GL_TRIANGLES ); //_STRIP );
	glNormal3fv( n1() );
	glVertex3fv( v1() );
	glNormal3fv( n2() );
	glVertex3fv( v2() );
	glNormal3fv( n3() );
	glVertex3fv( v3() );
    glEnd(); 
}

void Icosahedron::subdivide( vec3 v1, vec3 v2, vec3 v3, int level )
{
	vec3 v12=v1,v13=v1,v23=v2;
	v12 += v2;
	v13 += v3;
	v23 += v3;
	v12.normalize();
	v13.normalize();
	v23.normalize();
	if( level<= 0 )
	{
		drawFace( v1, v12, v13 );
		drawFace( v12, v2, v23 );
		drawFace( v13,v23, v3 );
		drawFace( v12,v23, v13 );
	}
	else
	{
		subdivide( v1, v12, v13, level-1 );
		subdivide( v12, v2, v23, level-1 );
		subdivide( v13,v23, v3,  level-1 );
		subdivide( v12,v23, v13, level-1 );
	}
}

void Icosahedron::draw( int resolution )
{
	static GLfloat X = .525731112119133606;
	static GLfloat Z = .850650808352039932;	

	static vec3 vdata[12] = {
	   vec3(-X, 0.0, Z), vec3(X, 0.0, Z), vec3(-X, 0.0, -Z), vec3(X, 0.0, -Z),    
	   vec3(0.0, Z, X), vec3(0.0, Z, -X), vec3(0.0, -Z, X), vec3(0.0, -Z, -X),    
	   vec3(Z, X, 0.0), vec3(-Z, X, 0.0), vec3(Z, -X, 0.0), vec3(-Z, -X, 0.0) 
	};

	static GLint tindices[20][3] = { 
	   {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},    
	   {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},    
	   {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6}, 
	   {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };

	   
	for( int i = 0; i < 20; i++ ) 
	{
		subdivide( vdata[tindices[i][0]], 
			      vdata[tindices[i][1]],
				  vdata[tindices[i][2]],
				  resolution ); //3 );
		//drawFace( vdata[tindices[i][0]], 
		//	      vdata[tindices[i][1]],
		//		  vdata[tindices[i][2]]  );
	}
}

//------------------------------------------------------------------------------
// 	Sphere
//------------------------------------------------------------------------------

Sphere::Sphere( int resolution, vec3 center )
: dl(0),c(center)
{	
	radius_n[0] = radius_n[1] = 0; // ** magic tuning parameter (e.g. try 6) **
}

Sphere::~Sphere()
{
	glDeleteLists( dl, 1 );
}

void Sphere::draw()
{	
	if( !dl )
		update( res );
	glCallList( dl );
}

void Sphere::update( int resolution, int *new_n )
{
	res = resolution;
	if( res<4 ) res=4;
	if( res>1024 ) res=1024;

	if( new_n )
	{
		radius_n[0] = new_n[0];
		radius_n[1] = new_n[1];
	}

	if( !dl ) dl = glGenLists(1);
	glNewList( dl, GL_COMPILE );
	drawSphere();
	glEndList();
}

void Sphere::drawSphere()
{
	std::cout << "sphere res=" << res << std::endl;
	float dtheta=PI/(float)res;
	float dphi=2*PI/(float)res;
	for( float theta=0; theta < PI; theta+=dtheta )
	{
		glBegin( GL_TRIANGLE_STRIP );
		for( float phi=0; phi < 2*PI+0.1; phi+=dphi )	// TODO: eliminate +0.1
		{
			vec3 vn = vec3( sin(theta)*cos(phi), 
				    sin(theta)*sin(phi), 
					cos(theta) );
			vec3 v = vn * radius(phi,theta) + c;

			vec3 un( sin(theta+dtheta)*cos(phi), 
				    sin(theta+dtheta)*sin(phi), 
					cos(theta+dtheta) );
			vec3 u = un * radius(phi,theta+dtheta) + c;

			glNormal3fv( vn() );
			glVertex3fv( v() );
			glNormal3fv( un() );
			glVertex3fv( u() );			
		}
		glEnd();
	}
}

float Sphere::radius( float phi, float theta )
{
	return 1.f + 0.3*sin(radius_n[0]*phi)*cos(radius_n[1]*theta);
}

//------------------------------------------------------------------------------
// 	SHBourke
//------------------------------------------------------------------------------

SHBourke::SHBourke( int resolution, 
  int m0, int m1, int m2, int m3, int m4, int m5, int m6, int m7 )
: dl(0),res(64)
{
	m[0]=m0; m[1]=m1; m[2]=m2; m[3]=m3; m[4]=m4; m[5]=m5; m[6]=m6; m[7]=m7;
	
	update( resolution );
}

SHBourke::~SHBourke()
{
	glDeleteLists( dl, 1 );
}

void SHBourke::update( int resolution, int *new_m )
{
	res = resolution;
	
	if( new_m )
		for( int i=0; i < 8; i++ )
			m[i] = new_m[i];

	if( !dl) dl = glGenLists(1);
	glNewList( dl, GL_COMPILE );
	DrawFunction( resolution );
	glEndList();
}

void SHBourke::draw()
{
	if( !dl )
		update( res );
	glCallList( dl );
}

SHBourke::XYZ SHBourke::Eval( double theta, double phi )
{
   double r = 0;
   XYZ p;

   r += pow(sin(m[0]*phi),(double)m[1]);
   r += pow(cos(m[2]*phi),(double)m[3]);
   r += pow(sin(m[4]*theta),(double)m[5]);
   r += pow(cos(m[6]*theta),(double)m[7]);

   p.x = r * sin(phi) * cos(theta);
   p.y = r * cos(phi);
   p.z = r * sin(phi) * sin(theta);

   return p;
}

void SHBourke::Normalise( XYZ *p )
{
   double length;

   length = sqrt(p->x * p->x + p->y * p->y + p->z * p->z);
   if (length != 0) {
      p->x /= length;
      p->y /= length;
      p->z /= length;
   } else {
      p->x = 0;
      p->y = 0;
      p->z = 0;
   }}

//SHBourke::XYZ SHBourke::CalcNormal( XYZ o, XYZ u, XYZ v )
SHBourke::XYZ SHBourke::CalcNormal( XYZ p, XYZ p1, XYZ p2 )
{
	//vec3 a( u.x-o.x, u.y-o.y, u.z-o.z );
	//vec3 b( v.x-o.x, v.y-o.y, v.z-o.z );
	//
	//a.normalize();
	//b.normalize();
	//vec3 axb = a % b;	// cross-product
	//axb.normalize();

	//XYZ ret;
	//ret.x=axb[0]; ret.y=axb[1]; ret.y=axb[2];

	//return ret;

   XYZ n,pa,pb;

   pa.x = p1.x - p.x;
   pa.y = p1.y - p.y;
   pa.z = p1.z - p.z;
   pb.x = p2.x - p.x;
   pb.y = p2.y - p.y;
   pb.z = p2.z - p.z;

   Normalise(&pa);
   Normalise(&pb);
  
   n.x = pa.y * pb.z - pa.z * pb.y;
   n.y = pa.z * pb.x - pa.x * pb.z;
   n.z = pa.x * pb.y - pa.y * pb.x;
   Normalise(&n);

   return(n);
}

void SHBourke::DrawFunction( int resolution )
{
   double du = TWOPI / (double)resolution; /* Theta */
   double dv = PI / (double)resolution;    /* Phi   */

   XYZ n[4], q[4];
   RGB c[4];
       c[0].r = c[0].g = c[0].b 
	 = c[1].r = c[1].g = c[1].b 
	 = c[2].r = c[2].g = c[2].b
	 = c[3].r = c[3].g = c[3].b = 1.0f;
   double u,v;

   glBegin(GL_QUADS);
   for (int i=0;i<resolution;i++) {
      u = i * du;
      for (int j=0;j<resolution;j++) {
         v = j * dv;
         q[0] = Eval(u,v);
         n[0] = CalcNormal(q[0],
                           Eval(u+du/10,v),
                           Eval(u,v+dv/10));
//         c[0] = GetColour(u,0.0,TWOPI,colourmap);
         glNormal3f(n[0].x,n[0].y,n[0].z);
         glColor3f(c[0].r,c[0].g,c[0].b);
         glVertex3f(q[0].x,q[0].y,q[0].z);

         q[1] = Eval(u+du,v);
         n[1] = CalcNormal(q[1],
                           Eval(u+du+du/10,v),
                           Eval(u+du,v+dv/10));
//         c[1] = GetColour(u+du,0.0,TWOPI,colourmap);
         glNormal3f(n[1].x,n[1].y,n[1].z);
         glColor3f(c[1].r,c[1].g,c[1].b);
         glVertex3f(q[1].x,q[1].y,q[1].z);

         q[2] = Eval(u+du,v+dv);
         n[2] = CalcNormal(q[2],
                           Eval(u+du+du/10,v+dv),
                           Eval(u+du,v+dv+dv/10));
//         c[2] = GetColour(u+du,0.0,TWOPI,colourmap);
         glNormal3f(n[2].x,n[2].y,n[2].z);
         glColor3f(c[2].r,c[2].g,c[2].b);
         glVertex3f(q[2].x,q[2].y,q[2].z);

         q[3] = Eval(u,v+dv);
         n[3] = CalcNormal(q[3],
                           Eval(u+du/10,v+dv),
                           Eval(u,v+dv+dv/10));
  //       c[3] = GetColour(u,0.0,TWOPI,colourmap);
         glNormal3f(n[3].x,n[3].y,n[3].z);
         glColor3f(c[3].r,c[3].g,c[3].b);
         glVertex3f(q[3].x,q[3].y,q[3].z);
      }
   }
   glEnd(); 
}
