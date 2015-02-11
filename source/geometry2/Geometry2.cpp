#include "Geometry2.h"
#include <cassert>
#include <iostream>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

//==============================================================================
//	SimpleGeometry
//==============================================================================

SimpleGeometry::Face::Face() 
	{ vi[0]=vi[1]=vi[2]=0; }
SimpleGeometry::Face::Face( int i[3] )
	{ vi[0]=i[0]; vi[1]=i[1]; vi[2]=i[2]; }
SimpleGeometry::Face::Face( int i, int j, int k )
	{ vi[0]=i; vi[1]=j; vi[2]=k; }

/*
void SimpleGeometry::test_pointer_consistency()
{
	using namespace std;
	// test ptr consistency
	float* vp = get_vertices_ptr();
	float err=0.f;
	for( int i=0; i < num_vertices(); ++i )
	{
		vec3 v = get_vertex( i );
		for( int j=0; j < 3; ++j ) 
		{			
			cout << i << ":  " << vp[3*i+j] << "  vs.  " << v[j] << endl;
			err += (vp[3*i+j] - v[j])*(vp[3*i+j] - v[j]);
		}
	}
	cout << "Error = " << err << endl;
	int* tp = get_indices_ptr();
	for( int i=0; i < num_faces(); ++i )
	{
		Face f = get_face(i);
		for( int j=0; j < 3; ++j )
			assert( tp[i*3+j] == f.vi[j] );
	}
}
*/

void SimpleGeometry::clear()
{
	m_vdata.clear();
	m_ndata.clear();
	m_fdata.clear();
}

void SimpleGeometry::reserve_vertices( int n )
{
#ifndef GEOMETRY2_NO_BUFFER_SUPPORT
	m_vdata.reserve( 3*(n-8) );
	m_ndata.reserve( 3*(n-8) );
#else
	m_vertices.reserve( n - 8 );
	m_normals .reserve( n - 8 );
#endif
}

void SimpleGeometry::reserve_faces( int n )
{
#ifndef GEOMETRY2_NO_BUFFER_SUPPORT
	m_fdata.reserve( 3*n );
#else
	m_faces   .reserve( n );
#endif
}

#ifndef GEOMETRY2_NO_BUFFER_SUPPORT
	
int SimpleGeometry::num_vertices() const { return m_vdata.size()/3; }
int SimpleGeometry::num_faces()    const { return m_fdata.size()/3; }

vec3 SimpleGeometry::get_vertex( int i ) const
{
	return vec3( m_vdata[i*3], m_vdata[i*3+1], m_vdata[i*3+2] );
}

vec3 SimpleGeometry::get_normal( int i ) const
{
	return vec3( m_ndata[i*3], m_ndata[i*3+1], m_ndata[i*3+2] );
}

SimpleGeometry::Face SimpleGeometry::get_face( int i ) const
{
	return Face( m_fdata[i*3+0], m_fdata[i*3+1], m_fdata[i*3+2] );
}

int SimpleGeometry::add_face( Face f )
{
	for( int i=0; i < 3; ++i )
		m_fdata.push_back( f.vi[i] );
	assert( m_fdata.size()%3 == 0 );
	return m_fdata.size()/3 - 1;
}

int SimpleGeometry::add_vertex_and_normal( vec3 v, vec3 n )
{	
	for( int i=0; i < 3; ++i ) 
	{
		m_vdata.push_back( v[i] );
		m_ndata.push_back( n[i] );
	}
	assert( m_vdata.size() == m_ndata.size() );
	assert( m_vdata.size()%3 == 0 );
	return m_vdata.size()/3 - 1;
}

float* SimpleGeometry::get_vertex_ptr()  { return &m_vdata[0]; }

float* SimpleGeometry::get_normal_ptr ()  { return &m_ndata[0]; }

int*   SimpleGeometry::get_index_ptr()   { return &m_fdata[0]; }

#else

int SimpleGeometry::num_vertices() const { return m_vertices.size(); }
int SimpleGeometry::num_faces()    const { return m_faces   .size(); }

vec3 SimpleGeometry::get_vertex( int i )
{
	return m_vertices.at(i);
}

SimpleGeometry::Face SimpleGeometry::get_face( int i )
{
	return m_faces.at(i);
}

int SimpleGeometry::add_face( Face f )
{
	m_faces.push_back( f );
	return m_faces.size()-1;
}

int SimpleGeometry::add_vertex_and_normal( vec3 v, vec3 n )
{	
	m_vertices.push_back( v );
	m_normals .push_back( n );
	// enforce identical indices for vertices/normals
	assert( m_vertices.size() == m_normals.size() );
	return m_vertices.size()-1;
}

float* SimpleGeometry::get_vertex_ptr()  { return &m_vertices[0][0]; }

float* SimpleGeometry::get_normal_ptr ()  { return &m_normals[0][0]; }

int*   SimpleGeometry::get_index_ptr()   { return &m_faces[0].vi[0]; }

#endif // GEOMETRY2_NO_BUFFER_SUPPORT


//==============================================================================
//	Icosahedron
//==============================================================================

void Icosahedron::add_face_subdivision( Face f, int levels )
{
	if( levels == 0 )
	{
		// insert face for real
		add_face( f );
		return;
	}

	// get vertices
	vec3 v1 = get_vertex( f.vi[0] ),
		 v2 = get_vertex( f.vi[1] ),
		 v3 = get_vertex( f.vi[2] );
	
	// new vertices
	vec3 v12=v1,v13=v1,v23=v2;
	v12 += v2;
	v13 += v3;
	v23 += v3;
#if 0
	// test subdivision
	v12 /= 2.f;
	v13 /= 2.f;
	v23 /= 2.f;
#else
	// normalize such that the new vertices again lie on the unit sphere surface
	v12.normalize();
	v13.normalize();
	v23.normalize();

	float foo = m_platonicConstantZ;
	v12 *= foo;
	v13 *= foo;
	v23 *= foo;
#endif
	
	// corresponding indices
	int i1  = f.vi[0],
	    i2  = f.vi[1],
	    i3  = f.vi[2],
		i12 = add_vertex_and_normal( v12, v12/v12.magnitude() ),
		i13 = add_vertex_and_normal( v13, v13/v13.magnitude() ),
		i23 = add_vertex_and_normal( v23, v23/v23.magnitude() );
	
	// 4 new faces
	add_face_subdivision( Face( i1,  i12, i13 ), levels-1 );
	add_face_subdivision( Face( i12, i2,  i23 ), levels-1 );
	add_face_subdivision( Face( i13, i23, i3  ), levels-1 );
	add_face_subdivision( Face( i12, i23, i13 ), levels-1 );
}

void Icosahedron::create( int levels )
{
	if( levels < 0 ) 
		levels = m_levels;
	else
		m_levels = levels;

	// initial 20 sided platonic solid
	
	// golden ratio (1+sqrt(5))/2 = 1.6180339887498948482045868343656
	// sqrt(5) = 2,2360679774997896964091736687313
	// 2*pi/5 = 1,2566370614359172953850573533118

	//static double X = 1.287654321; //.525731112119133606;
	//static double Z = 1.723456789; //.850650808352039932;
#if 0
	// Bendels Icosahedron definition
	double X = .525731112119133606 , //m_platonicConstantX,
		   Z = .850650808352039932;  //m_platonicConstantZ,		   
	//Y = 0.3;
	/*static*/ vec3 vdata[12] = {
	   //vec3(-X, -Y, Z), vec3(X, 0.0, Z), vec3(-X, 0.0, -Z), vec3(X, 0.0, -Z),
	   //vec3(0.0, Z, X), vec3(0.0, Z, -X), vec3(0.0, -Z, X), vec3(0.0, -Z, -X),    
	   //vec3(Z, X, -Y), vec3(-Z, X, 0.0), vec3(Z, -X, 0.0), vec3(-Z, -X, 0.0) 
	   vec3(-X, 0.0, Z), vec3(X, 0.0, Z), vec3(-X, 0.0, -Z), vec3(X, 0.0, -Z),
	   vec3(0.0, Z, X), vec3(0.0, Z, -X), vec3(0.0, -Z, X), vec3(0.0, -Z, -X),    
	   vec3(Z, X, 0.0), vec3(-Z, X, 0.0), vec3(Z, -X, 0.0), vec3(-Z, -X, 0.0) 
	};
	static int tindices[20][3] = { 
	   {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},    
	   {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},    
	   {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6}, 
	   {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };
#else
	// Classic Icosahedron definition (based solely on golden ratio constant)
	float tao = m_platonicConstantX; //1.61803399;
	vec3 vdata[12] = { vec3(1,tao,0),vec3(-1,tao,0),vec3(1,-tao,0),vec3(-1,-tao,0),
					   vec3(0,1,tao),vec3(0,-1,tao),vec3(0,1,-tao),vec3(0,-1,-tao),
					   vec3(tao,0,1),vec3(-tao,0,1),vec3(tao,0,-1),vec3(-tao,0,-1) };

	static int tindices[20][3] = { 
	{0,1,4},{1,9,4},{4,9,5},{5,9,3},{2,3,7},{3,2,5},{7,10,2},{0,8,10},{0,4,8},{8,2,10},{8,4,5},{8,5,2},{1,0,6},{11,1,6},{3,9,11},{6,10,7},{3,11,7},{11,6,7},{6,0,10},{9,1,11}
	};
#endif

	// exact memory calulation
	size_t n = (double)20*pow(4.f,levels);
	reserve_vertices( n - 8 );
	reserve_faces   ( n );
	   
	// insert vertices
	for( int i=0; i < 12; ++i )
	{
		// add normalized vertices lying on the surface of the unit sphere
		vec3 v = vdata[i];
		v.normalize();
		add_vertex_and_normal( v, v );
	}
	
	// insert faces (at the end of subdivision process)
	for( int i=0; i < 20; ++i ) 
	{
		add_face_subdivision( Face(tindices[i]), levels );
	}
}

//==============================================================================
//	Penrose tiling
//==============================================================================

Penrose::Penrose()
: m_levels(1)
{
	setDefaultGenerator();
}

void Penrose::reserve_faces( int n )
{
	// Reserve memory for face type attribute
	m_faceType.reserve( n );
	
	// Call super implementation
	SimpleGeometry::reserve_faces( n );
}

int Penrose::add_face( SimpleGeometry::Face f, int type )
{
	// Set face type
	m_faceType.push_back( type );
	// Call super implementation
	return SimpleGeometry::add_face( f );	
}

int Penrose::add_face( SimpleGeometry::Face f )
{
	// Distinguish face type by thresholding angle at apex (= first vertex).
	// Note that we do not check for isoscele nor exact angles 36° / 108°.
	float* vp = get_vertex_ptr();
	vec3 AB = get_vertex( f.vi[1] ) - get_vertex( f.vi[0] ),
	     AC = get_vertex( f.vi[2] ) - get_vertex( f.vi[0] );
	float theta = acos( AB.scalarprod( AC ) / (AB.magnitude() * AC.magnitude()) );
	int type = ( theta < 50.f ) ? Red : Blue;
	
	return add_face( f, type );
}

void Penrose::add_face_subdivision( SimpleGeometry::Face f, int type, int levels )
{
	const double goldenRatio = (1. + sqrt(5.)) / 2.;
	
	if( levels == 0 )
	{
		// insert face for real
		add_face( f, type );
		return;
	}
	
	// get vertices
	vec3 a = get_vertex( f.vi[0] ),
	     b = get_vertex( f.vi[1] ),
	     c = get_vertex( f.vi[2] );
	
	// get normals
	vec3 na = get_normal( f.vi[0] ),
	     nb = get_normal( f.vi[1] ),
	     nc = get_normal( f.vi[2] );
	
	if( type == Red )
	{
		// split into two triangles
		
		// new vertex p
		vec3 p = a + (b - a) / goldenRatio;
		// new normal by interpolating normals at a and b
		vec3 np = na + (nb - na) / goldenRatio;
		np.normalize();
		int pi = add_vertex_and_normal( p, np );		
		// new faces
		Face pca( pi, f.vi[2], f.vi[0] );
		Face cpb( f.vi[2], pi, f.vi[1] );
		
		add_face_subdivision( pca, Blue, levels-1 );
		add_face_subdivision( cpb, Red,  levels-1 );
	}
	else if( type == Blue )
	{
		// split into three triangles
		
		vec3 q = b + (a - b) / goldenRatio,
		     r = b + (c - b) / goldenRatio;
		
		vec3 nq = nb + (na - nb) / goldenRatio,
		     nr = nb + (nc - nb) / goldenRatio;
		nq.normalize();
		nr.normalize();
		
		int qi = add_vertex_and_normal( q, nq ),
		    ri = add_vertex_and_normal( r, nr );
		
		Face rca( ri, f.vi[2], f.vi[0] );
		Face qrb( qi, ri, f.vi[1] );
		Face rqa( ri, qi, f.vi[0] );
		
		add_face_subdivision( rca, Blue, levels-1 );
		add_face_subdivision( qrb, Blue, levels-1 );
		add_face_subdivision( rqa, Red,  levels-1 );
	}
	else
	{
		// This should not happen!
	}	
}

void Penrose::create( int levels )
{
	if( levels<0 )
		levels = m_levels;
	else
		m_levels = levels;

	// Copy generator vertices (FIXME: direct copy would be much faster!)
	for( int i=0; i < m_generator.num_vertices(); i++ )
		add_vertex_and_normal( m_generator.get_vertex(i), m_generator.get_normal(i) );

	// Start with all faces "Red"
	for( int i=0; i < m_generator.num_faces(); i++ )
		add_face_subdivision( m_generator.get_face(i), Red, levels );

#if 0  // HARDCODED WHEEL GENERATOR
	// Origin
	add_vertex_and_normal( vec3( 0,0,0 ), vec3(0,0,1) );
	
	// Wheel around origin
	for( int i=0; i < 10; i++ )
	{
		double phi = (double)i * 2.*M_PI / 10.;
		add_vertex_and_normal(
			vec3( cos(phi), sin(phi), 0 ),
			vec3( 0,0,1 ) );
	}
	
	// Subdivide red triangles
	for( int i=0; i < 10; i++ )
	{
		int j = (i+0) % 10 + 1,
			k = (i+1) % 10 + 1;

		// Mirror every second triangle
		if( i%2 )		
			add_face_subdivision( Face( 0, j, k ), Red, levels );
		else
			add_face_subdivision( Face( 0, k, j ), Red, levels );
	}
#endif
}

void Penrose::setDefaultGenerator()
{
	clear();

	SimpleGeometry geom;

	// Origin
	geom.add_vertex_and_normal( vec3( 0,0,0 ), vec3(0,0,1) );
	
	// Wheel around origin
	for( int i=0; i < 10; i++ )
	{
		double phi = (double)i * 2.*M_PI / 10. + M_PI/2.;
		geom.add_vertex_and_normal(
			vec3( cos(phi), sin(phi), 0 ),
			vec3( 0,0,1 ) );
	}
	
	// Subdivide red triangles
	for( int i=0; i < 10; i++ )
	{
		int j = (i+0) % 10 + 1,
			k = (i+1) % 10 + 1;

		// Mirror every second triangle
		if( i%2 )		
			geom.add_face( Face( 0, j, k ) );
		else
			geom.add_face( Face( 0, k, j ) );
	}

	// Copy
	m_generator = geom;
}

void Penrose::setGenerator( const SimpleGeometry& geom )
{
	// Copy
	m_generator = geom;
}


//==============================================================================
//	Superquadric
//==============================================================================

//-----------------------------------------------------------------------------
//  Superquadric helper functions
//-----------------------------------------------------------------------------

/// Sign function
double sgn( double value )
{
	return (value >= 0.) ? +1. : -1.;
}

/// Signed absolute power
double spow( double base, double exponent )
{
	return sgn(base) * pow(abs(base),exponent);
}

/// Superquadric function around z-axis
vec3 qz( double theta, double phi, double alpha, double beta )
{
	double sphi = spow(sin(phi),beta);
	return vec3( spow(cos(theta),alpha) * sphi,
	             spow(sin(theta),alpha) * sphi,
	             spow(cos(phi),beta) );
}

/// Superquadric function around x-axis
vec3 qx( double theta, double phi, double alpha, double beta )
{
	double sphi = spow(sin(phi),beta);
	return vec3( spow(cos(phi),beta),
	            -spow(sin(theta),alpha) * sphi,
	             spow(cos(theta),alpha) * sphi );
}

/// Superquadric tensor function parameterized over planarity (cp) and linearity (cl)
vec3 superquadric_tensor( double cp, double cl, double gamma, double theta, double phi )
{
	if( cl >= cp )
		return qx( theta, phi, pow(1.-cp,gamma), pow(1.-cl,gamma) );	
	// cl < cp
	return qz( theta, phi, pow(1.-cl,gamma), pow(1.-cp,gamma) );
}

void Superquadric::create( int unused )
{
	clear();

	int res = 32;

	double theta_step = (2.*M_PI)/(double)res;
	double phi_step = M_PI/(double)res;

	int n = 2.*M_PI / theta_step;
	int m = M_PI / phi_step + 1;	
		// n*m == num_vertices()
	
	// Sample vertices
	for( int i=0; i < n; i++ )
	{
		double theta = (double)i*theta_step;		
		for( int j=0; j < m; j++ )
		{
			double phi = (double)j*phi_step;

			vec3 v;
			switch( m_mode )
			{
			default:
			case Quadric:
				// FIXME: Decide when to use qx or qz automatically.
				v = qx( theta, phi, m_alpha, m_beta );
				break;
			case TensorGlyph:
				v = superquadric_tensor( m_cp, m_cl, m_gamma, theta, phi );
				break;
			}

			add_vertex_and_normal( v, v/v.magnitude());			
		}
	}
	
	// Establish faces
	for( int i=0; i < n; i++ )
		for( int j=0; j < m; j++ )
		{
			// v3___v2
			//  |   |
			//  |___|
			// v0   v1
			int v0 = i*m + j,
				v1 = i*m + (j+1)%m,
				v2 = ((i+1)%n)*m + (j+1)%m,
				v3 = ((i+1)%n)*m + j;

			// Triangulate quad face
			add_face( Face(v0,v1,v3) );
			add_face( Face(v1,v2,v3) );
		}
}
