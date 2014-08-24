#ifndef CLUSTERSEEDS_H
#define CLUSTERSEEDS_H

#include <vector>

/**
 *	\class ClusterSeeds
 *
 *	Seed point strategies medoid cluster algorithms based on a distance matrix.
 *  std::rand() is used as RNG where a seed can explicitly be specified via
 *	setRandomSeed() to achieve reproducible results on the same machine.
 *	Different seeding strategies are provided, see \a Strategy.
 *	
 *	\author Max Hermann
 * 	\date	April 28, 2014
 */
template <typename MATRIX> 
class ClusterSeeds
{
public:
	/// Supported seeding strategies
	enum Strategy { 
		SeedRandomly,       ///< Uniform random sampling
		SeedFarthestPoints, ///< Farthest point sampling (wrt distance matrix) 
		SeedMiddlePoints    ///< Heuristical initialization after Park et al. 2009
	};
	
	// c'tor
	ClusterSeeds( const MATRIX& D, int n, int k, int strategy=SeedRandomly )
	: m_D(D),
	  m_strategy(strategy),
	  m_numPoints(n), m_numSeeds(k), 
	  m_seeds       (k,-1), // initialize seeds with an invalid index value
	  m_initialPoint(-1),   // index=-1 indicates random choice of initial point
	  m_randomSeed  (-1)    // seed=-1 indicates random seed value
	{
	}
	
	/// Return seed points, performs seeding if not done previously
	template<class IVEC>
	void getSeeds( IVEC& seeds )
	{
		if( !seeded() ) seed();
		seeds.resize(m_seeds.size());
		for( int i=0; i < m_seeds.size(); i++ ) seeds[i] = m_seeds[i];
	}
	
	/// Compute seed points based on strategy and set parameters
	void seed()
	{
		switch( m_strategy )
		{
		case SeedFarthestPoints: seed_farthest_points( m_D );
		case SeedMiddlePoints  : seed_middle_points  ( m_D );
		default:
		case SeedRandomly: seed_random(); break;
		}
	}

	/// Returns true if seed points are available
	bool seeded() const { return !m_seeds.empty(); }

	///@{ Seeding parameters
	void setInitialPoint( int index ) { m_initialPoint = index; }
	void setRandomSeed( int random_seed ) { m_randomSeed=random_seed; }
	int getInitialPoint() const { return m_initialPoint; }
	int getRandomSeed() const { return m_randomSeed; }
	///@}
	
protected:
	
	//@{ Seeding strategies
	void seed_random();		
	void seed_farthest_points( const MATRIX& D );	
	void seed_middle_points( const MATRIX& D );	
	///@}

	/// Returns a uniformly chosen point in 0,..,numPoints-1
	int random_point();

private:
	const MATRIX& m_D; ///< Symmetric all-pairs distance matrix
	int m_strategy;
	int m_numPoints;
	int m_numSeeds;
	int m_initialPoint;
	int m_randomSeed;
	std::vector<int> m_seeds; // internally -1 signals an invalid index
};


//==============================================================================
//  Template implementations
//==============================================================================

#include <Eigen/Dense>
#include <algorithm> // std::random_shuffle(), std::sort()
#include <ctime>     // std::time
#include <cstdlib>   // std::rand, std::srand
#include <cassert>

template <typename MATRIX>
int ClusterSeeds<MATRIX>::random_point()
{
	return std::rand() % m_numPoints;
}

template <typename MATRIX>
void ClusterSeeds<MATRIX>::seed_random()
{
	// Allow reproducible results by specifying explicit random seed
	// (guarantees same random number sequence at least on same machine).
	if( m_randomSeed < 0 )
	{
		m_randomSeed = (int)std::time(0);
		std::srand ( unsigned ( m_randomSeed ) );
	}
	
	// Randomly select seed points
	// Use a permutation of indices to avoid duplicates
	std::vector<int> indices( m_numPoints );
	for( int i=0; i < m_numPoints; ++i ) indices[i] = i;
	std::random_shuffle( indices.begin(), indices.end() );
	m_seeds = indices;
}

template <typename MATRIX>
void ClusterSeeds<MATRIX>::seed_farthest_points( const MATRIX& D )
{
	m_seeds.clear();
	if( m_initialPoint < 0 )
	{
		// Choose initial sample point randomly		
		m_initialPoint = random_point();
	}
	
	// Initialize closest point distance vector and set first seed point
	Eigen::VectorXd d( m_numPoints );
	d = D.row( m_initialPoint ); // FIXME: Type conversion could be required!
	m_seeds.push_back( m_initialPoint );
	
	// Farthest point sampling
	for( int i=0; i < m_numSeeds; i++ )
	{
		// Choose farthest point
		int j;
		d.maxCoeff( &j );
		
		// Update closest point distances
		d = d.cwiseMin( D.row( j ) );
		
		m_seeds.push_back( j );
	}
}

template <typename MATRIX>
void ClusterSeeds<MATRIX>::seed_middle_points( const MATRIX& D )
{
	// Store a normalized distance per point together with the point index
	std::vector< std::pair<double,int> > v( m_numPoints );

	// Custom sort predicate to sort according to distance
	// Note: Instead of a custom predicate we could also use C++11 bind().
	struct sort_pred {
		bool operator() ( const std::pair<double,int> &l, 
		                  const std::pair<double,int> &r )
		{
			return l.first < r.first;
		}
	};
	
	// Compute normalized distances
	for( int j=0; j < m_numPoints; j++ )
	{
		v[j].first  = 0.0;
		v[j].second = j;
		
		for( int i=0; i < m_numPoints; i++ )
			v[j].first += m_D(i,j) / m_D.row(i).sum();
	}
	
	// Sort ascending
	std::sort( v.begin(), v.end(), sort_pred() );
	
	// Select k first points with smallest values
	m_seeds.clear();
	for( int i=0; i < m_numSeeds; i++ )
		m_seeds.push_back( v[i].second );
}

#endif // CLUSTERSEEDS_H
