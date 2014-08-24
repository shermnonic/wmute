#ifndef KDISTANCEMEDOIDS_H
#define KDISTANCEMEDOIDS_H

#include <vector>
#include <algorithm> // std::random_shuffle
#include <iostream>

/**
	Some old code I found in my master thesis repository, 
	seems the implementation is plain wrong?!

	DO NOT USE THIS CODE!

	Max Hermann
*/
template<class matrix_type>
class KDistanceMedoids
{
public:	
	typedef std::vector<unsigned int> ivec;

	KDistanceMedoids( const matrix_type& d, unsigned int k )
	  : D(d), N((unsigned)d.rows()), K(k)
	{
		m_labels  = ivec( N, 0 );	
		m_clusters = new ivec[K];

		// randomly select initial medoids
		ivec indices( N );
		for( unsigned int i=0; i < N; ++i ) indices[i] = i;
		std::random_shuffle( indices.begin(), indices.end() );	
		m_medoids = ivec( indices.begin(), indices.begin()+K );	
		
		// label each point with its nearest medoid
		label();
	}

	~KDistanceMedoids()
	{
		delete [] m_clusters;
	}	
	
	/// \return >0 number of iterations needed until convergence 
	///         <0 number of unchanged medoids in last update
	int cluster( unsigned int maxIterations=10000 )
	{
		using std::cout;
		using std::endl;
		int verbose=1;

		unsigned int unchanged = 0;
		unsigned int iterations = 0;
		while( (unchanged < K) && (iterations < maxIterations) )
		{
			if( verbose )
				cout << "K-Medoids iteration " << iterations+1 << "\r";
			unchanged = update();
			iterations++;
		}
		if( verbose )
			cout << endl;		

		if( unchanged!=K )
		{
			if( verbose )
				cout << "K-Medoids did not converge, " << unchanged << " medoids unchanged in last iteration" << endl;
			return -(int)unchanged;
		}
		
		if( verbose )
			cout << "K-Medoids converged after " << iterations << " iterations" << endl;
		return iterations;
	}
	

	/// \return sum of inter cluster distances
	double sumInterClusterDistances() const
	{
		// sum of inter cluster distances
		double sum = 0.0;
		ivec::const_iterator it,jt;
		for( it = m_medoids.begin(); it != m_medoids.end(); ++it )
			for( jt = m_medoids.begin(); jt != m_medoids.end(); ++jt )
			{
				sum += D( *it, *jt );
			}
		
		return sum;
	}	

	/// \return const reference to vector of labels
	const ivec& labels()  const { return m_labels; };
	/// \return const reference to vector of medoids
	const ivec& medoids() const { return m_medoids; }

protected:
	/// Label each point with its nearest medoid.
	void label()
	{
		for( unsigned int j=0; j < K; ++j ) m_clusters[j].clear();

		// label each point with its nearest medoid
		for( unsigned int i=0; i < N; ++i )
		{
			double          min = std::numeric_limits<double>::max();
			unsigned int medoid = K+1;
			
			for( unsigned int j=0; j < K; ++j )
			{
				double d = D(i,m_medoids[j]);
				if( d < min )
				{
					min = d;
					medoid = j;
				}
			}
			
			m_labels[i] = medoid;
			m_clusters[medoid].push_back( i );		
		}
	}
	
	/// Find new medoid for each cluster.
	/// \return number of unchanged medoids
	unsigned int update()
	{
		// number of unchanged medoids after update step
		unsigned int unchanged = 0;
		
		// find new medoid for each cluster
		for( unsigned int j=0; j < K; ++j )		
		{
			double min = std::numeric_limits<double>::max();
			unsigned int medoid = K+1;
		
			// find point which minimizes the sum of distances to all other points inside cluster
			ivec::iterator it;
			for( it = m_clusters[j].begin(); it != m_clusters[j].end(); ++it )
			{
				unsigned int x = *it;
				double sum = 0.0;
				
				// calculate sum of distances
				ivec::iterator it2;
				for( it2 = m_clusters[j].begin(); it2 != m_clusters[j].end(); ++it2 )
				{
					unsigned int y = *it;				
					sum += D(x,y);
				}
				
				// the point which minimizes this sum becomes the new medoid
				if( sum < min )
				{
					min = sum;
					medoid = x;
				}
			}
			
			if( m_medoids[j] == medoid ) unchanged++;
			m_medoids[j] = medoid;
		}
		
		// label each point with its nearest medoid
		label();
		
		return unchanged;
	}
	
private:
	const matrix_type& D;
	unsigned int N;
	unsigned int K;

	ivec  m_medoids;
	ivec  m_labels;
	ivec* m_clusters;
};

#endif // KDISTANCEMEDOIDS_H
