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
		{
			updateVertices( i );
			updateNormals( i );
		}

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
	
	vbuf()[ofs  ] = v(0);
	vbuf()[ofs+1] = v(1);
	vbuf()[ofs+2] = v(2);
}

void TensorfieldObject::add_normal( int glyphId, int vhandle, const Eigen::Vector3d& n )
{
	int ofs = glyphId * numGlyphVertices() * 3 + vhandle * 3;
	
	nbuf()[ofs  ] = n(0);
	nbuf()[ofs+1] = n(1);
	nbuf()[ofs+2] = n(2);
}

void TensorfieldObject::add_face( int glyphId, int fhandle, int v0, int v1, int v2 )
{
	int ofs = glyphId * numGlyphFaces() * 3 + fhandle * 3;
	int vofs = glyphId * numGlyphVertices();
	
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
			
		  #if 1
			// Rotate and scale according to spectrum
			v = m_glyphScale * (R * lambda.asDiagonal() * v);
		  #endif

		  #if 1
			// Center at corresponding vertex in mean shape
			if( m_pos.cols() == m_R.cols() ) // sanity check
				v = v + m_pos.col(glyphId);
		  #endif
			
			// Store transformed geometry
			add_vertex( glyphId, vh, v );
			add_normal( glyphId, vh, v/v.norm() ); // FIXME: Compute real normal
			vh++;
		}
	}
}

int imod( int a, int b )
{
	return (a >= 0) ? a%b : (b-a)%b;
}

void TensorfieldObject::updateNormals( int glyphId )
{
	// Sample resolution in phi and theta
	int n = m_glyphRes,
	    m = m_glyphRes+1;

	// Sample vertices
	int vh=0;  // Vertex handle
	for( int i=0; i < n; i++ )
	{
		for( int j=0; j < m; j++ )
		{
			// Compute discrete normal by averaging face normals of 1-diamond
			// around sample vertex p:
			//     v2
			// n2 / | \ n1
			//   /  |  \
			// v3 - p - v1
			//  \   |  /
			// n3\  | / n0
			//     v0 

			// Compute vertex handles
			int h1 = i*m + imod(j + 1, m),
				h3 = i*m + imod(j - 1, m),
				h2 = imod(i + 1, n)*m + j,
				h0 = imod(i - 1, n)*m + j;

			// Get vertices
			Eigen::Vector3d 
				p  = get_vertex( glyphId, vh ),
				v0 = get_vertex( glyphId, h0 ),
				v1 = get_vertex( glyphId, h1 ),
				v2 = get_vertex( glyphId, h2 ),
				v3 = get_vertex( glyphId, h3 );

			// Estimate normals via cross-product
			Eigen::Vector3d
				n0 = ((v0-p).cross(v1-p)).normalized(),
				n1 = ((v1-p).cross(v2-p)).normalized(),
				n2 = ((v2-p).cross(v3-p)).normalized(),
				n3 = ((v3-p).cross(v0-p)).normalized();

			// Average vertex normal
			Eigen::Vector3d
				navg = (n0 + n1 + n2 + n3) / 4.;

			add_normal( glyphId, vh, navg ); vh++;
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
