#ifndef COVARIANCECLUSTERING_H
#define COVARIANCECLUSTERING_H

#include "ShapeCovariance.h"
#include "ClusterSeeds.h"
#include <Eigen/Dense>

class CovarianceClustering
{
public:
	/// Parameters for k-Medoids clustering
	struct ClusterParms
	{
		unsigned k;                ///< Number of clusters
		double   weightTensorDist; ///< Weight of tensor metric
		double   weightPointDist;  ///< Weight of point metric
		unsigned maxIter;          ///< Max. number of iterations for k-medoids
		unsigned repetitions; ///< Number of clustering repetitions (result will be best of all runs)
		int      seedingStrategy;  ///< Seeding strategy (initialization of k-medoids, one of \a ClusterSeeds::Strategy)
		
		/// C'tor sets default parameters
		ClusterParms()
		: k(10),
		  weightTensorDist(1.0),
		  weightPointDist(1.0),
		  maxIter(10000),
		  repetitions(10),
		  seedingStrategy( ClusterSeeds<Eigen::MatrixXd>::SeedFarthestPoints )
		{}
	};
	
	void compute( const Eigen::MatrixXd& S, const Eigen::Matrix3Xd& pts, ClusterParms parms );

	const std::vector<unsigned int>& getLabels() const { return m_labels; }
	const std::vector<unsigned int>& getMedoids() const { return m_medoids; }
	
protected:
	void computeDistanceMatrix( const Eigen::MatrixXd& S, const Eigen::Matrix3Xd& pts, const ClusterParms& parms, Eigen::MatrixXd& D );

private:
	Eigen::MatrixXd   m_tensorData;
	Eigen::Matrix3Xd  m_pointData;
	ClusterParms      m_parms;
	std::vector<unsigned int> m_labels;
	std::vector<unsigned int> m_medoids;
};

#endif // COVARIANCECLUSTERING_H
