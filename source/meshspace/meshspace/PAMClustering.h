#ifndef PAMCLUSTERING
#define PAMCLUSTERING

#include <vector>
#include <Eigen/Dense>

// Determine minimum cost swap by really calculating objective function.
// This is needed as long as there is this nasty bug in the more efficient cost-update code.
#define PAMCLUSTERING_BRUTE_FORCE

/**
 *  \class PAMClustering
 *
 *  Implementation of Partitioning Around Medoids algorithm.
 *  After "Finding Groups in Data: an Introduction to Cluster Analysis" by Kaufman,Rosseeuw 1990.
 *
 *  \todo Switch from brute force swap-cost calculation to more efficient cost-update.
 */
class PAMClustering
{
public:
	typedef Eigen::MatrixXd matrix_type;
	typedef std::vector<unsigned int> ivec;
	typedef std::vector<double> dvec;

	/// Constructor (does *not* perform clustering, call cluster() to do this).
	/// \param d symmetric distance matrix
	/// \param k number of clusters
	/// \remarks Input matrix d is stored as const reference, so make sure it is valid
	///          throughout the existence of your PAMClustering instance.
	PAMClustering( const matrix_type& d, unsigned int k );
	~PAMClustering();
	
	/// Copy constructor (copy assignment not allowed because of const referenced distance matrix)
	PAMClustering( PAMClustering& other );

	/// \param maxIterations is the number of iterations after which to abort
	/// \return number of iterations needed until convergence 
	int cluster( unsigned int maxIterations=10000 );

	/// Calculate per point, per cluster and overall silhouette width.
	/// \return overall silhouette width
	double calcSilhouette();
	/// \return silhouette width of cluster c as determined by last calcSilhouette() call
	double silCluster( unsigned int c );
	/// \return array of silhouette width per point as determined by last calcSilhouette() call
	const dvec& sils() const { return m_sil; }

	/// \return const reference to vector of labels
	const ivec& labels()  const { return m_labels; };
	/// \return const reference to vector of medoids
	const ivec& medoids() const { return m_medoids; }

protected:
	/// Label each point with its nearest medoid.
	void label();
	
	/// Perform PAM swap step.
	/// \return true if final medoids found, false otherwise
	bool swap();

	/// \return true if point i already was used as medoid
	bool isSelected( unsigned int i );

	/// \return average distance from point i to points in cluster c
	double avgDist( unsigned int i, unsigned int c );


#ifdef PAMCLUSTERING_BRUTE_FORCE
	// returns objective value of clustering using m_medoids
	// in contrast to label() here is not acted on member m_labels but given reference
	double PAMClustering::label( ivec& labels );
#endif
	
private:
	// assignment not possible because of const Reference D member
	PAMClustering& operator = ( PAMClustering& other );

	const matrix_type& D;
	unsigned int N;
	unsigned int K;

	ivec  m_medoids;
	ivec  m_labels;
	ivec  m_second;
	ivec* m_clusters;

	ivec  m_selected;    ///> All points already used medoids
	double m_objective;  ///> Sum of all intra-cluster distances
	
	dvec  m_sil;         ///> Silhouette width for each point
	double m_silTotal;   ///> Overall silhouette width
};

#endif
