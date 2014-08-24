#include "MDSEmbedding.h"
#include <cassert>
#include <iostream>

using Eigen::MatrixXd;
using Eigen::VectorXd;

double MDSEmbedding::getCoordinate( int idx, int dim ) const
{
	if( idx>=0 && idx < m_C.rows() && dim>=0 && dim < m_C.cols() )
		return m_C(idx,dim);
	return 0.0;
}

void MDSEmbedding::setDistanceMatrix( double* D, size_t n )
{
	m_D = Eigen::Map<MatrixXd>( D, n, n );
}

void MDSEmbedding::setDistanceMatrix( const MatrixXd& D )
{
	m_D = D;
}

void MDSEmbedding::computeEmbedding( int verbosity )
{
	using namespace std;
	bool debug=verbosity > 2;

	if( m_D.cols()==0 || m_D.rows()==0 )
		return;	
	
	assert( m_D.rows() == m_D.cols() );
	size_t n = m_D.rows();
	
	MatrixXd D = m_D;
	if( debug ) cout << "Input matrix=" << endl << D << endl;

	// Double center via Helmert matrix 
	MatrixXd P = MatrixXd::Identity(n,n) - MatrixXd::Ones(n,n)*(1./n);
	D = P * (-.5 * D.cwiseProduct(D)) * P;

	if( debug ) cout << "Double centered matrix=" << endl << D << endl;
	
#if 1
	// Self adjoint solver
	Eigen::SelfAdjointEigenSolver<MatrixXd> es;
	es.compute( D );
	// Sort descending w.r.t. eigenvalues
	VectorXd ev = es.eigenvalues().reverse();
	MatrixXd V  = es.eigenvectors().rowwise().reverse();
#else
	// Diagonalization via SVD
	Eigen::JacobiSVD<MatrixXd> svd( D, Eigen::ComputeFullU );
	VectorXd ev = (svd.singularValues() / (n-1.)).cwiseSqrt();
	MatrixXd V = svd.matrixU();
#endif

	if( debug ) cout << "Eigenvalues=" << endl << ev << endl;
	if( debug ) cout << "Eigenvectors=" << endl << V << endl;

	// Cut-off
	double eps = 1e-12;
	MatrixXd::Index d; 
	for( d=0; d < ev.size() && ev(d)>eps; d++ );
	m_ev = ev.head(d);
	m_V  = V .leftCols(d);

	if( debug ) cout << "Eigenvalues after cut-off=" << endl << m_ev << endl;
	if( debug ) cout << "Eigenvectors after cut-off=" << endl << m_V << endl;
	
	// Compute full embedding (up to a maximum dimensionality)	
	m_C = m_V * (m_ev.cwiseSqrt()).asDiagonal();

	if( debug ) cout << "Full embedding=" << endl << m_C << endl;
}
