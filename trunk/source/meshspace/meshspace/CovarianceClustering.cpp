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
	PAMClustering kmedoids( D, m_parms.k );
	ClusterSeeds<MatrixXd> seedGen( D, n, m_parms.k, m_parms.seedingStrategy );
	PAMClustering::ivec seedPoints;

	double bestObjective = std::numeric_limits<double>::max();
	std::vector<double> objectiveGraph;
	for( unsigned i=0; i < m_parms.repetitions; i++ )
	{
		// seed points
		seedGen.seed();
		seedGen.getSeeds( seedPoints );

		// k-medoids
		kmedoids.setSeedPoints( seedPoints );
		int iters = kmedoids.cluster( m_parms.maxIter );
		double objective = kmedoids.getError();

		std::cout << "Round " << i+1 << "/" << m_parms.repetitions << ": " 
			<< objective << ((objective < bestObjective) ? "(best so far)" : "")
			<< std::endl;

		if( objective < bestObjective )
		{
			bestObjective = objective;

			// Store best result so far
			m_labels  = kmedoids.labels();
			m_medoids = kmedoids.medoids();			
		}

		objectiveGraph.push_back( objective );
	}

	std::cout << "Graph of objective function values:" << std::endl;
	for( unsigned i=0; i < objectiveGraph.size(); i++ )
		std::cout << objectiveGraph.at(i) << std::endl;
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
