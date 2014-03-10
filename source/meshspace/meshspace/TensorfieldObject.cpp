#include "TensorfieldObject.h"
#include <ShapeCovariance.h>
#include <cmath>

using ShapeCovariance::computeSampleCovariance;

//-----------------------------------------------------------------------------
// 	Superquadric helper functions
//-----------------------------------------------------------------------------

typedef Eigen::Vector3d vec3;

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

vec3 qref( double theta, double phi, double alpha, double beta )
{
	theta -= M_PI;
	phi   -= .5*M_PI;

	return vec3( spow(cos(phi),beta) * spow(cos(theta),alpha),
		         spow(cos(phi),beta) * spow(sin(theta),alpha),
				 spow(sin(phi),beta) );
}

#if 0
vec3 superquadric_tensor( double cp, double cl, double gamma, double theta, double phi )
{
	return qref( theta, phi, pow(1.-cp,gamma), pow(1.-cl,gamma) );
}
vec3 superquadric_tensor_normal( double cp, double cl, double gamma, double theta, double phi )
{
	return qref( theta, phi, 2.-pow(1.-cp,gamma), 2.-pow(1.-cl,gamma) );	
}
#else
/// Superquadric tensor function parameterized over planarity (cp) and linearity (cl)
vec3 superquadric_tensor( double cp, double cl, double gamma, double theta, double phi )
{
	if( cl >= cp )
		return qx( theta, phi, pow(1.-cp,gamma), pow(1.-cl,gamma) );	
	// cl < cp
	return qz( theta, phi, pow(1.-cl,gamma), pow(1.-cp,gamma) );
}

vec3 superquadric_tensor_normal( double cp, double cl, double gamma, double theta, double phi )
{
	if( cl >= cp )
		return qx( theta, phi, 2.-pow(1.-cp,gamma), 2.-pow(1.-cl,gamma) );	
	// cl < cp
	return qz( theta, phi, 2.-pow(1.-cl,gamma), 2.-pow(1.-cp,gamma) );
}
#endif


//-----------------------------------------------------------------------------
// 	scene::TensorfieldObject implementation
//-----------------------------------------------------------------------------

namespace scene {

TensorfieldObject::TensorfieldObject()
	: m_dirtyFlag     ( CompleteChange ),
	  m_glyphRes      ( 8  ),  // 16 = high quality
	  m_glyphSharpness( 3. ),  // 3. is Kindlman default
	  m_glyphScale    ( .1 )
{}

void TensorfieldObject::setGlyphPositions( meshtools::Mesh* mesh )
{
	// NOT IMPLEMENTED YET!
	assert( false );
}

void TensorfieldObject::setGlyphPositions( Eigen::Matrix3Xd pos )
{
	m_pos = pos;
	m_dirtyFlag |= GeometryChange;
}

void TensorfieldObject::createTestScene()
{
	int n = 21;

	m_R     .resize( 9, n );
	m_Lambda.resize( 3, n );

	Eigen::Matrix3Xd pos( 3, n );

	// Sample Westin triangle
	for( int i=0, count=0; i < 6; ++i  )
	{
		for( int j=0; j <= i; ++j, ++count )
		{
			double l0 = 1.,
			       l1 = 1. - (j / 5.),
			       l2 = 1. - (i / 5.);

			// Sorted eigenvalues
			Eigen::Vector3d lambda;
			if( l1 >= l2 )
				lambda = Eigen::Vector3d( l0, l1, l2 );
			else
				lambda = Eigen::Vector3d( l0, l2, l1 );

			m_Lambda.col(count) = lambda;

			pos.col(count) = Eigen::Vector3d( l1+.5*(i/5.), l2, 0 );
		}
	}
	setGlyphPositions( pos );
	setGlyphScale( 0.07 );

	// Constant rotation
	Eigen::Matrix3d R;
	R = Eigen::AngleAxisd( (double)M_PI/4., Eigen::Vector3d(-1.,0.,0.) );
	//Eigen::Matrix3d I = Eigen::Matrix3d::Identity();
	for( int i=0; i < n; ++i )
	{		
		for( int j=0; j < 9; j++ )
			m_R(j,i) = R.data()[j];
	}

	// Create tensor glyphs
	m_dirtyFlag = CompleteChange;
	updateTensorfield();
}

void TensorfieldObject::deriveTensorsFromPCAModel( const PCAModel& pca )
{
	Eigen::MatrixXd S; 
	computeSampleCovariance( pca.X, S );
	setGlyphPositions( reshape(pca.mu) );
	deriveTensorsFromCovariance( S );
}

void TensorfieldObject::deriveTensorsFromCovariance( const Eigen::MatrixXd& S )
{
	using ShapeCovariance::devectorizeCovariance;
	
	int n = (int)S.cols();
	
	m_R     .resize( 9, n );
	m_Lambda.resize( 3, n );

	// Compute spectrum
	for( int i=0; i < n; ++i )
	{
		// SVD
		Eigen::Matrix3d Sigma;
		devectorizeCovariance( S.col(i), Sigma );
		Eigen::JacobiSVD< Eigen::Matrix3d > svd( Sigma, Eigen::ComputeFullU );

		// Store rotation
	  #if 0
		m_R.col(i) = Eigen::Map<Eigen::VectorXd>( svd.matrixU().data(), 9 );
	  #else
		for( int j=0; j < 9; j++ )
			m_R(j,i) = svd.matrixU().data()[j];
	  #endif
		// Turn reflection into rotation
		if( m_R.determinant() < 0. )
			m_R.col(2) *= -1.;

		// Store scaling
		m_Lambda.col(i) = svd.singularValues().cwiseSqrt();
	}
	
	// Create tensor glyphs
	m_dirtyFlag = CompleteChange;
	updateTensorfield();
}

void TensorfieldObject::updateTensorfield()
{	
	// Generate geometry
	// We use a single VBO to fit into the MeshBuffer framework. A slight 
	// drawback is that color has to be encoded as per-vertex attribute.
	// For now we are relying on MeshObject::m_scalarAttribBuffer for this which
	// is accessible via MeshObject::scalars().
	
	if( m_dirtyFlag & ResolutionChange )
	{
		// Allocate buffers
		ibuf().resize( numGlyphs() * numGlyphFaces() * 3 );
		vbuf().resize( numGlyphs() * numGlyphVertices() * 3 );
		nbuf().resize( numGlyphs() * numGlyphVertices() * 3 );
		scalars().resize( numGlyphs() * numGlyphVertices() ); // see updateColor()
	}
	
	// Add geometry and scalar attribute for coloring
	int n = numGlyphs();
	for( int i=0; i < n; ++i )
	{
		if( m_dirtyFlag & ResolutionChange )
			updateFaces( i );

		if( m_dirtyFlag & ResolutionChange || m_dirtyFlag & GeometryChange )
			updateVertices( i );

		if( m_dirtyFlag & ResolutionChange || m_dirtyFlag & ColorChange )
			updateColor( i );
	}
	
	// Update MeshBuffer
	if( m_dirtyFlag & ResolutionChange )
	{
		// (Re)allocate buffers
		meshBuffer().initSingleFrameFromRawBuffers();

		// We have to provide a valid mesh as well!
		MeshObject::setMesh( meshBuffer().createMesh(), true /* keep buffers! */ );
	}
	else
	{
		// Force update of GPU buffers
		meshBuffer().setFrameUpdateRequired();
		// FIXME: We could also do selective buffer update based on flags.
	}

	m_dirtyFlag = NoChange;
}

Eigen::Vector3d TensorfieldObject::get_vertex( int glyphId, int vhandle )
{
	int ofs = glyphId * numGlyphVertices() * 3 + vhandle * 3;

	Eigen::Vector3d v;
	v(0) = vbuf()[ofs  ];
	v(1) = vbuf()[ofs+1];
	v(2) = vbuf()[ofs+2];

	return v;
}

void TensorfieldObject::add_vertex( int glyphId, int vhandle, const Eigen::Vector3d& v )
{
	int ofs = glyphId * numGlyphVertices() * 3 + vhandle * 3;
	
	vbuf()[ofs  ] = (float)v(0);
	vbuf()[ofs+1] = (float)v(1);
	vbuf()[ofs+2] = (float)v(2);
}

void TensorfieldObject::add_normal( int glyphId, int vhandle, const Eigen::Vector3d& n )
{
	int ofs = glyphId * numGlyphVertices() * 3 + vhandle * 3;
	
	nbuf()[ofs  ] = (float)n(0);
	nbuf()[ofs+1] = (float)n(1);
	nbuf()[ofs+2] = (float)n(2);
}

void TensorfieldObject::add_face( int glyphId, int fhandle, int v0, int v1, int v2 )
{
	int ofs = glyphId * numGlyphFaces() * 3 + fhandle * 3;
	int vofs = glyphId * numGlyphVertices();
	
	ibuf()[ofs  ] = (unsigned)(vofs + v0);
	ibuf()[ofs+1] = (unsigned)(vofs + v1);
	ibuf()[ofs+2] = (unsigned)(vofs + v2);
}

void TensorfieldObject::get_res( int& n, int&m )
{
	n = m_glyphRes,
	m = m_glyphRes+1;	
}

void TensorfieldObject::updateFaces( int glyphId )
{	
	// Sample resolution in phi and theta
	int n,m; get_res(n,m);
	
	// Establish triangle connectivity
	int fh=0;  // Face handle
	for( int i=0; i < n; i++ )
		for( int j=0; j < m-1; j++ )
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
			add_face( glyphId, fh, v0,v1,v3 ); fh++;
			add_face( glyphId, fh, v1,v2,v3 ); fh++;
		}		
}

void TensorfieldObject::updateVertices( int glyphId )
{
	// Tensor glyph parameters
	double gamma = m_glyphSharpness;
	
	Eigen::Vector3d lambda = m_Lambda.col( glyphId );

	// Clamp scaling
	lambda(0) = std::max( lambda(0), 0.01 );
	lambda(1) = std::max( lambda(1), 0.01 );
	lambda(2) = std::max( lambda(2), 0.01 );

	// Westin'97 barycentric coordinates of eigenvalues
	double evsum = lambda(0)+lambda(1)+lambda(2),
	       cl = (lambda(0) - lambda(1)) / evsum,
	       cp = 2.*(lambda(1)-lambda(2)) / evsum;

	// Eigenvectors
	Eigen::Matrix3d R;
	for( int i=0; i < 9; i++ )
		R.data()[i] = m_R(i,glyphId);

	// Sample resolution in phi and theta
	int n,m; get_res(n,m);

	// Step sizes in phi and theta
	double theta_step = (2.*M_PI)/(double)m_glyphRes,
	       phi_step = M_PI/(double)(m_glyphRes-1);	
	
	// Sample vertices
	int vh=0;  // Vertex handle
	for( int i=0; i < n; i++ )
	{
		double theta = (double)i*theta_step;		
		for( int j=0; j < m; j++ )
		{
			double phi = (double)j*phi_step;
			
			// Superquadric geometry
			Eigen::Vector3d 
				v = superquadric_tensor( cp, cl, gamma, theta, phi ),
				n = superquadric_tensor_normal( cp, cl, gamma, theta, phi );
			
			// Workaround to consistently outward orient normals
			if( (v+n).norm() < (v-n).norm() )
				v = -v;
			// Rotate and scale according to spectrum
			v = m_glyphScale * (R * lambda.asDiagonal() * v);
			n = R * lambda.cwiseInverse().asDiagonal() * n;
			n.normalize();

			// Center at corresponding vertex in mean shape
			if( m_pos.cols() == m_R.cols() ) // sanity check
				v = v + m_pos.col(glyphId);
			
			// Store transformed geometry
			add_vertex( glyphId, vh, v );
			add_normal( glyphId, vh, n );
			vh++;
		}
	}
}

int imod( int a, int b )
{
	return (a >= 0) ? a%b : (b-a)%b;
}

void TensorfieldObject::updateColor( int glyphId )
{
	Eigen::Vector3d lambda = m_Lambda.col( glyphId );
	
	// Mean diffusivity
	double tr = lambda(0) + lambda(1) + lambda(2);
	double mu = tr / 3.;
	
	// Fractional anisotropy
	double FA = sqrt( 3.*((lambda(0)-mu)*(lambda(0)-mu) + (lambda(1)-mu)*(lambda(1)-mu) + (lambda(2)-mu)*(lambda(2)-mu))
	            / (2.*tr*tr) );
	
	// Assign FA as scalar attribute to all vertices of given glyph
	int ofs = glyphId*numGlyphVertices(); // Index of first vertex in scalar buffer 
	for( int i=0; i < numGlyphVertices(); ++i )
	{
		scalars()[ ofs + i ] = (float)FA;
	}
	
	// FA is normalized in [0,1]
	setScalarShiftScale( 0, 1 );
}

} // namespace scene
