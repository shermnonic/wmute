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

/// Superquadric tensor function parameterized over planarity (cp) and linearity (cl)
vec3 superquadric_tensor( double cp, double cl, double gamma, double theta, double phi )
{
	if( cl >= cp )
		return qx( theta, phi, pow(1.-cp,gamma), pow(1.-cl,gamma) );	
	// cl < cp
	return qz( theta, phi, pow(1.-cl,gamma), pow(1.-cp,gamma) );
}

//-----------------------------------------------------------------------------
// 	scene::TensorfieldObject implementation
//-----------------------------------------------------------------------------

namespace scene {

void TensorfieldObject::deriveTensorsFromPCAModel( const PCAModel& pca )
{
	Eigen::MatrixXd S; 
	computeSampleCovariance( pca.X, S );
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

		// Store rotation and scaling
	  #if 0
		m_R.col(i) = Eigen::Map<Eigen::VectorXd>( svd.matrixU().data(), 9 );
	  #else
		for( int j=0; j < 9; j++ )
			m_R(j,i) = svd.matrixU().data()[j];
	  #endif
		m_Lambda.col(i) = svd.singularValues().cwiseSqrt();
	}
	
	// Create tensor glyphs
	createGeometry( 16 );
}

void TensorfieldObject::createGeometry( int res )
{
	// Glyph resolution can only be changed here in createGeometry()
	m_glyphRes = res;
	
	// Generate geometry
	// We use a single VBO to fit into the MeshBuffer framework. A slight 
	// drawback is that color has to be encoded as per-vertex attribute.
	// For now we are relying on MeshObject::m_scalarAttribBuffer for this which
	// is accessible via MeshObject::scalars().
	
	// Allocate buffers
	ibuf().resize( numGlyphs() * numGlyphFaces() * 3 );
	vbuf().resize( numGlyphs() * numGlyphVertices() * 3 );
	nbuf().resize( numGlyphs() * numGlyphVertices() * 3 );
	scalars().resize( numGlyphs() * numGlyphVertices() );
	
	// Add geometry and scalar attribute for coloring
	int n = numGlyphs();
	for( int i=0; i < n; ++i )
	{
		updateFaces( i );
		updateVertices( i );
		//updateColor( i );
	}
	
	// Update MeshBuffer
	meshBuffer().initSingleFrameFromRawBuffers();

	// We have to provide a valid mesh as well!
	MeshObject::setMesh( meshBuffer().createMesh(), true /* keep buffers! */ );
}

void TensorfieldObject::add_vertex_and_normal( int glyphId, int vhandle, 
				const Eigen::Vector3d& v, const Eigen::Vector3d& n )
{
	int ofs = glyphId * numGlyphVertices() * 3 + vhandle * 3;
	
	vbuf()[ofs  ] = v(0);
	vbuf()[ofs+1] = v(1);
	vbuf()[ofs+2] = v(2);
	
	nbuf()[ofs  ] = n(0);
	nbuf()[ofs+1] = n(1);
	nbuf()[ofs+2] = n(2);
}

void TensorfieldObject::add_face( int glyphId, int fhandle, int v0, int v1, int v2 )
{
	int ofs = glyphId * numGlyphFaces() * 3 + fhandle * 3;
	int vofs = glyphId * numGlyphVertices() * 3;
	
	ibuf()[ofs  ] = vofs + v0;
	ibuf()[ofs+1] = vofs + v1;
	ibuf()[ofs+2] = vofs + v2;
}

void TensorfieldObject::updateFaces( int glyphId )
{	
	// Sample resolution in phi and theta
	int n = m_glyphRes,
	    m = m_glyphRes+1;
	
	// Establish triangle connectivity
	int fh=0;  // Face handle
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
			add_face( glyphId, fh, v0,v1,v3 ); fh++;
			add_face( glyphId, fh, v1,v2,v3 ); fh++;
		}		
}

void TensorfieldObject::updateVertices( int glyphId )
{
	// Tensor glyph parameters
	double gamma = m_glyphSharpness;
	
	// Westin'97 barycentric coordinates of eigenvalues
	Eigen::Vector3d lambda = m_Lambda.col( glyphId );
	double evsum = lambda(0)+lambda(1)+lambda(2),
	       cl = lambda(0) - lambda(1) / evsum,
	       cp = 2.*(lambda(1)-lambda(2)) / evsum;

	// Eigenvectors
	Eigen::Matrix3d R;
	for( int i=0; i < 9; i++ )
		R.data()[i] = m_R(i,glyphId);

	// Sample resolution in phi and theta
	int n = m_glyphRes,
	    m = m_glyphRes+1;
	
	// Step sizes in phi and theta
	double theta_step = (2.*M_PI)/(double)m_glyphRes,
	       phi_step = M_PI/(double)m_glyphRes;	
	
	// Sample vertices
	int vh=0;  // Vertex handle
	for( int i=0; i < n; i++ )
	{
		double theta = (double)i*theta_step;		
		for( int j=0; j < m; j++ )
		{
			double phi = (double)j*phi_step;
			
			// Superquadric geometry
			Eigen::Vector3d v = superquadric_tensor( cp, cl, gamma, theta, phi );
			
			// Rotate and scale according to spectrum
			v = R * lambda.asDiagonal() * v;
			
			// Store transformed geometry
			add_vertex_and_normal( glyphId, vh, v, v/v.norm() );
			vh++;
		}
	}
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
		scalars()[ ofs + i ] = FA;
	}
	
	// FA is normalized in [0,1]
	setScalarShiftScale( 0, 1 );
}

} // namespace scene
