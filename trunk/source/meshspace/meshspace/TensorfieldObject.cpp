#include "TensorfieldObject.h"
#include <ShapeCovariance.h>
#include <cmath>
#include <fstream>

#include "MatrixUtilities.h"
using MatrixUtilities::removeColumn;

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
	  m_colorMode     ( ColorByFractionalAnisotropy ),
	  m_glyphRes      ( 8  ),  // 16 = high quality
	  m_glyphSharpness( 3. ),  // 3. is Kindlman default
	  m_glyphScale    ( .1 ),
	  m_glyphSqrtEV   ( false )
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

	// Create tensor field data (e.g. for debugging IO code)
	m_tensorField.resize( 6, n );
	for( int i=0; i < n; i++ )
	{
		// Make use of constant rotation R
		// Generate a symmetric matrix with decomposition R*Lambda*R'
		Eigen::MatrixXd T = R * m_Lambda.col(i).asDiagonal() * R.transpose();
		Eigen::VectorXd v;
		ShapeCovariance::vectorizeCovariance( T, v );
		m_tensorField.col(i) = v;
	}
}

void TensorfieldObject::exportTensorfieldAsNrrd( std::string path, std::string basename, MeshBuffer* mb )
{
	using namespace std;

	string filename      = basename + ".nrrd",
		   filename_raw  = basename + ".raw",
	       filename_mesh = basename + "-mesh.lmpd";

	// Write header file (.nrrd)
	ofstream hf( path + "/" + filename );
	if( !hf.is_open() )
	{
		cerr << "TensorfieldObject::exportTensorfieldAsNrrd() : Could not open " 
			 << filename << endl;
		return;	
	}

	hf << "NRRD0002" << endl
	   << "type: double" << endl
	   << "dimension: 3" << endl
	   << "sizes: " << m_tensorField.rows() << " " << m_tensorField.cols() << endl
	   << "endian: little" << endl
	   << "encoding: raw" << endl
	   << "data file: ./" << filename_raw << endl
	   << "DATA_LIMN:=" << filename_mesh << endl
	   << "DATA_NAME_0000:=" << basename << endl
	   << "DATA_TYPE_0000:=tensor" << endl;
	hf.close();

	// Write raw data file with tensors (.raw)
	ofstream rf( path + "/" + filename_raw, ios_base::binary );
	if( !rf.is_open() )
	{
		cerr << "TensorfieldObject::exportTensorfieldAsNrrd() : Could not open " 
			 << filename_raw << endl;
		return;	
	}
	rf.write( (char*)m_tensorField.data(), m_tensorField.cols()*m_tensorField.rows()*sizeof(double) );
	rf.close();

	// Write mesh data
	if( !mb )
		return;
}

void TensorfieldObject::saveTensorfield( std::string filename )
{
	using namespace std;
	ofstream of( filename, ios_base::binary );
	if( !of.is_open() )
	{
		cerr << "TensorfieldObject::saveTensorfield() : Could not open " 
			 << filename << endl;
		return;
	}

	// FOR DEBUGGING
	double* ptr_pos = (double*)m_pos.data();
	double* ptr_S   = (double*)m_tensorField.data();

	const char magic[] = "TENSORFIELD";	
	unsigned nrows = (unsigned)m_tensorField.rows(),
		     ncols = (unsigned)m_tensorField.cols(),
			 npts  = (unsigned)m_pos.cols();

	of.write( magic, sizeof(magic) );
	of.write( (char*)&nrows, sizeof(unsigned) );
	of.write( (char*)&ncols, sizeof(unsigned) );
	of.write( (char*)&npts, sizeof(unsigned) );
	of.write( (char*)m_tensorField.data(), sizeof(double)*nrows*ncols );
	of.write( (char*)m_pos.data(), sizeof(double)*3*npts );
	of.close();
}

bool TensorfieldObject::loadTensorfield( std::string filename )
{
	using namespace std;
	ifstream f( filename, ios_base::binary );
	if( !f.is_open() )
	{
		cerr << "TensorfieldObject::loadTensorfield() : Could not open " 
			 << filename << endl;
		return false;
	}

	// Read header
	char magic[] = "TENSORFIELD";
	unsigned nrows,
		     ncols,
			 npts;

	f.read( magic, sizeof(magic) );
	if( string(magic) != string("TENSORFIELD") )
	{
		cerr << "TensorfieldObject::loadTensorfield() : " << filename 
			 << "is not a valid TENSORFIELD file!" << endl;
		return false;
	}

	f.read( (char*)&nrows, sizeof(unsigned) );
	f.read( (char*)&ncols, sizeof(unsigned) );
	f.read( (char*)&npts,  sizeof(unsigned) );

	// Read tensor matrix
	Eigen::MatrixXd S; S.resize( nrows, ncols );
	f.read( (char*)S.data(), sizeof(double)*nrows*ncols );

	// Read glyph positions (if included)
	Eigen::Matrix3Xd pos; pos.resize(3,npts);
	f.read( (char*)pos.data(), sizeof(double)*3*npts );	
	f.close();

	cout << "Loaded " << ncols << " tensors" << endl;

	// FOR DEBUGGING
	double* ptr_pos = (double*)pos.data();
	double* ptr_S   = (double*)S.data();

	// Filter results
	unsigned filtered = filterTensorField( S, pos );
	cout << "Filtered out " << filtered << " tensors with too small Frobenius norm" << endl;

	// Update visualization
	setGlyphPositions( pos );
	deriveTensorsFromCovariance( S );

	return true;
}

unsigned TensorfieldObject::filterTensorField( Eigen::MatrixXd& S, Eigen::Matrix3Xd& pts, double threshold )
{
	unsigned n = (unsigned)S.cols();
#if 1 // USE TEMPORARIES
	std::vector<unsigned> valid_columns;
	for( unsigned p=0; p < n; p++ )
	{
		double frob = S.col(p).norm();
		if( frob >= threshold )
		{
			valid_columns.push_back( p );
		}
	}
	Eigen::MatrixXd  S_filtered( S.rows(), valid_columns.size() );
	Eigen::Matrix3Xd pts_filtered( 3, valid_columns.size() );
	for( unsigned i=0; i < valid_columns.size(); i++ )
	{
		S_filtered.col(i) = S.col( valid_columns[i] );
		pts_filtered.col(i) = pts.col( valid_columns[i] );
	}
	unsigned filtered = S.cols() - S_filtered.cols();
	S = S_filtered;
	pts = pts_filtered;
	return filtered;
#else // DIRECT REMOVAL (CRASHES?!)
	unsigned filtered = 0;
	for( unsigned p=n-1; p >= 0; p-- )
	{
		double frob = S.col(p).norm();
		if( frob < threshold && S.cols()>1 )
		{
			removeColumn( S,   p );
			removeColumn( pts, p );
			filtered++;
		}
	}
	return filtered;
#endif
}

void TensorfieldObject::deriveTensorsFromPCAModel( const PCAModel& pca,  int mode, double gamma, double scale )
{
	using ShapeCovariance::computeInterPointCovariance;
	using ShapeCovariance::computeSampleCovariance;
	using ShapeCovariance::devectorizeCovariance;
	using ShapeCovariance::vectorizeCovariance;

	// Sanity check
	if( mode == InterPointCovarianceUnweighted ||
		mode == InterPointCovariance )
	{
		if( gamma <= 0.0 )	
			std::cerr << "TensorfieldObject::deriveTensorsFromPCAModel(): "
			  "For inter-point covariance a gamma value > 0.0 has to be "
			  "specified!" << std::endl;
	}

	// For all modes we place glyph at vertex positions of mean shape
	setGlyphPositions( reshape(pca.mu) );

	if( mode == AnatomicCovariance )
	{
		Eigen::MatrixXd S;
		computeSampleCovariance( pca.X, S );
		m_glyphSqrtEV = false;
		deriveTensorsFromCovariance( S );
	}
	else
	if( mode == InterPointCovarianceUnweighted )
	{
		Eigen::MatrixXd G;
		computeInterPointCovariance( pca.PC * pca.ev.asDiagonal(), gamma, G );
		G *= scale;
		m_glyphSqrtEV = false;
		deriveTensorsFromCovariance( G );
	}
	else
	if( mode == InterPointCovariance )
	{
		Eigen::MatrixXd S, G;
		computeSampleCovariance( pca.X, S );
		computeInterPointCovariance( pca.PC * pca.ev.asDiagonal(), gamma, G );
		G *= scale;

		// Weight inter-point with global covariance tensors
		for( unsigned p=0; p < G.cols(); p++ )
		{
			// Extract tensors at point p
			Eigen::Matrix3d Sp, Gp;
			devectorizeCovariance( S.col(p), Sp );
			devectorizeCovariance( G.col(p), Gp );

			// Weight inter-point G with global S tensor
			Eigen::Matrix3d W;
			W = Sp.transpose() * Gp * Sp;

			// Write result to G
			Eigen::VectorXd w;
			vectorizeCovariance( W, w );
			G.col(p) = w;
		}

		m_glyphSqrtEV = true;
		deriveTensorsFromCovariance( G );
	}
}

void TensorfieldObject::deriveTensorsFromCovariance( const Eigen::MatrixXd& S )
{
	using ShapeCovariance::devectorizeCovariance;

	// Store input tensor field
	m_tensorField = S;
	
	// Variables
	int n = (int)S.cols();
	
	m_R     .resize( 9, n );
	m_Lambda.resize( 3, n );

	int countFixedRotations = 0;
	
	// Compute spectrum
	int n_update = n / 200;
	for( int i=0; i < n; ++i )
	{
		// Progress info
		if( n_update>0 && (i%n_update == 0 || i==n-1) )
			std::cout << "Computing tensor spectrum " << (100*i/(n-1)) << "% "
				"(point " << i+1 << " / " << n << ") \r";

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

	  #if 0
		// Turn reflection into rotation
		if( m_R.determinant() < 0. )
		{
			m_R.col(2) *= -1.;
			countFixedRotations++;
		}
	  #endif

		// Store scaling
		m_Lambda.col(i) = svd.singularValues().cwiseSqrt();
	}
	std::cout << std::endl;

	if( countFixedRotations > 0 )
		std::cout << "Fixed " << countFixedRotations << " rotation matrices" << std::endl;
	
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

	// Sqrt
	if( m_glyphSqrtEV )
	{
		lambda(0) = sqrt(lambda(0));
		lambda(1) = sqrt(lambda(1));
		lambda(2) = sqrt(lambda(2));
	}

	// Clamp scaling
	lambda(0) = std::max( lambda(0), 0.020 );
	lambda(1) = std::max( lambda(1), 0.015 );
	lambda(2) = std::max( lambda(2), 0.010 );

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
	if( m_colorMode==ColorByFractionalAnisotropy )
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
		setScalarShiftScale( 0.f, 1.f );
	}
	else
	if( m_colorMode==ColorByCluster )
	{
		// Sanity
		unsigned clusterIdx=0;
		if( m_clusterIndices.size() != m_pos.cols() )
			std::cerr << "TensorfieldObject::updateColor() : "
				"Mismatch between cluster indices and sample points!" << std::endl;
		else
			clusterIdx = m_clusterIndices.at(glyphId);

		int ofs = glyphId*numGlyphVertices(); // Index of first vertex in scalar buffer 
		for( int i=0; i < numGlyphVertices(); ++i )
		{
			scalars()[ ofs + i ] = (float)clusterIdx;
		}

		if( m_numClusters > 0 )
			setScalarShiftScale( 0.f, 1.f/(float)(m_numClusters-1) );
		else
			setScalarShiftScale( 0.f, 1.f );
	}
	else
	{
		std::cerr << "TensorfieldObject::updateColor() : Unknown color mode!" << std::endl;
	}
}

void TensorfieldObject::setClusters( const std::vector<unsigned int>& indices, unsigned numClusters )
{
	m_clusterIndices = indices;
	if( numClusters <= 0 )
	{
		// Find maximum index
		for( unsigned i=0; i < indices.size(); i++ )
			if( indices.at(i) > numClusters ) 
				numClusters = indices.at(i);
	}
	m_numClusters = numClusters;
}

} // namespace scene
