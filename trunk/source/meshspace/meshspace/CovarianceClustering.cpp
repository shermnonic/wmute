#include "CovarianceClustering.h"
//#include "KDistanceMedoids.h"
#include "PAMClustering.h"
#include <cassert>
#include <iostream>

using Eigen::MatrixXd;
using Eigen::Matrix3Xd;

void CovarianceClustering::compute( const MatrixXd& S, const Matrix3Xd& pts, ClusterParms parms )
{
	using std::cout;
	using std::endl;
	using ShapeCovariance::computeBBoxDiagonal;
	using ShapeCovariance::computeCovariancesNormAvg;
	
	// Sanity
	assert( S.rows() == 6 );
	assert( S.cols() == pts.cols() );

	m_parms = parms;
	
	// Normalize data
	cout << "Normalizing data for clustering..." << endl;
	double alpha = computeCovariancesNormAvg( S );
	double beta  = computeBBoxDiagonal( pts );	
	m_tensorData = S / alpha;
	m_pointData  = pts / beta;
	
	// Compute distance matrix
	cout << "Computing distance matrix for clustering..." << endl;
	unsigned n = (unsigned)S.cols();
	MatrixXd D( n, n );
	computeDistanceMatrix( m_tensorData, m_pointData, m_parms, D );
	
	// Clustering
	cout << "Starting clustering..." << endl;
	m_parms = parms;
	//KDistanceMedoids<Eigen::MatrixXd> kmedoids( D, m_parms.k );
	PAMClustering kmedoids( D, m_parms.k );
	int ret = kmedoids.cluster( m_parms.maxIter );	
	
	// Copy result
	m_labels  = kmedoids.labels();
	m_medoids = kmedoids.medoids();
}

void CovarianceClustering::computeDistanceMatrix( const Eigen::MatrixXd& S, const Eigen::Matrix3Xd& pts, const ClusterParms& parms, Eigen::MatrixXd& D )
{
	using ShapeCovariance::covarDistEuclidean;	
	unsigned n = (unsigned)S.cols();
	for( unsigned i=0; i < n; i++ )
	{
		for( unsigned j=0; j < i; j++ )		
		{
			// Tensor distance
			double dT = covarDistEuclidean( m_tensorData.col(i), m_tensorData.col(j) );
			// Point distance
			double dp = (pts.col(i) - pts.col(j)).norm();
			
			// Combined distance
			D(i,j) = sqrt( parms.weightTensorDist*dT*dT + parms.weightPointDist*dp*dp );
			D(j,i) = D(i,j); // Force symmetry
		}
		D(i,i) = 0.0;
	}
}
