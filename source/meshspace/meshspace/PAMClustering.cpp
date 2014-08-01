#include "PAMClustering.h"
#include <algorithm>

#define PAMCLUSTERING_DEBUG_OUT
#ifdef PAMCLUSTERING_DEBUG_OUT
  #include <iostream>
  #include <iomanip>
#endif



PAMClustering::PAMClustering( const matrix_type& d, unsigned int k )
  : D(d), N(d.rows()), K(k)
{
	m_labels = ivec( N, 0 );
	m_second = ivec( N, 0 );
	m_clusters = new ivec[K];
	
	// randomly select initial medoids
	ivec indices( N );
	for( unsigned int i=0; i < N; ++i ) indices[i] = i;
	std::random_shuffle( indices.begin(), indices.end() );
	setSeedPoints( indices );
}

PAMClustering::~PAMClustering()
{
	delete [] m_clusters;
}

PAMClustering::PAMClustering( PAMClustering& other )
: D(other.D), N(other.N), K(other.K)
{
	m_medoids = other.m_medoids;
	m_labels  = other.m_labels;
	m_second  = other.m_second;
	
	m_clusters = new ivec[K];
	for( unsigned int k=0; k < K; ++k )
		m_clusters[k] = other.m_clusters[k];
	
	m_selected = other.m_selected;
	m_objective = other.m_objective;
	
	m_sil = other.m_sil;
	m_silTotal = other.m_silTotal;	
}


void PAMClustering::setSeedPoints( ivec indices )
{
	// set initial seeds
	m_medoids = indices;

	// clear previous "selected" points
	m_selected.clear();

	// mark initial medoids as "selected"
	for( unsigned int i=0; i < m_medoids.size(); ++i )
		m_selected.push_back( m_medoids[i] );

	// initially label each point with its nearest medoid
	label();
}


int PAMClustering::cluster( unsigned int maxIterations )
{
	unsigned int iterations = 0;
	bool converged = false;
	while( !converged && (iterations < maxIterations) )
	{
		converged = swap();
		iterations++;
	}
	
	return iterations;
}

double PAMClustering::avgDist( unsigned int i, unsigned int c )
{
	double dist = 0.0;
	ivec::const_iterator it;
	for( it = m_clusters[c].begin(); it != m_clusters[c].end(); ++it )
	{
		dist += D( i, *it );
	}
	
	return dist / (double)m_clusters[c].size();
}

double PAMClustering::silCluster( unsigned int c )
{
	double sil = 0.0;	
	ivec::iterator it;
	for( it = m_clusters[c].begin(); it != m_clusters[c].end(); ++it )
	{
		sil += m_sil[*it];
	}
	
	return sil / (double)m_clusters[c].size();
}

double PAMClustering::calcSilhouette()
{
	m_sil.resize( N );
	m_silTotal = 0.0;
	
	// determine silhouette for each point
	for( unsigned int i=0; i < N; ++i )
	{
		// average distance to points in same cluster
		double a_i = avgDist( i, m_labels[i] );
		
		// min_c average distance to points in other cluster c
		double b_i = std::numeric_limits<double>::max();
		for( unsigned int c=0; c < K; ++c )
		{
			if( c == m_labels[i] ) continue;
			
			double b_ic = avgDist( i, m_labels[c] );
			
			if( b_ic < b_i ) b_i = b_ic;
		}
		
		m_sil[i] = (b_i - a_i) / std::max(a_i,b_i);
		
		// overall silhouette width
		m_silTotal += m_sil[i];
	}

	m_silTotal /= (double)N;	
	return m_silTotal;
}

bool PAMClustering::isSelected( unsigned int i )
{
	return //std::find(m_medoids.begin(),m_medoids.end(),i) != m_medoids.end();
	       std::find(m_selected.begin(),m_selected.end(),i) != m_selected.end();
}

void PAMClustering::label()
{
	for( unsigned int j=0; j < K; ++j ) m_clusters[j].clear();

	double obj = 0.0;
	
	// label each point with its nearest medoid
	// and save second nearest medoid also
	for( unsigned int i=0; i < N; ++i )
	{
		double min, min2;
		unsigned int medoid, medoid2;
		
		min = min2 = std::numeric_limits<double>::max();
		medoid = medoid2 = K+1;
		
		for( unsigned int j=0; j < K; ++j )
		{
			double d = D(i,m_medoids[j]);
			if( d < min )
			{
				min2 = min;
				medoid2 = medoid;
				
				min = d;
				medoid = j;
			}
			else if( d < min2 )
			{
				min2 = d;
				medoid2 = j;
			}
		}
		
		m_labels[i] = medoid;
		m_second[i] = medoid2;
		m_clusters[medoid].push_back( i );
		
		obj += min;
	}
	
	m_objective = obj;
}

#ifdef PAMCLUSTERING_BRUTE_FORCE
double PAMClustering::label( ivec& labels )
{
	double obj = 0.0;
	
	labels.resize( N );
	
	// label each point with its nearest medoid
	// and save second nearest medoid also
	for( unsigned int i=0; i < N; ++i )
	{
		double min;
		unsigned int medoid;
		
		min = std::numeric_limits<double>::max();
		medoid = K+1;
		
		for( unsigned int j=0; j < K; ++j )
		{
			double d = D(i,m_medoids[j]);
			if( d < min )
			{
				min = d;
				medoid = j;
			}
		}
		
		labels[i] = medoid;
		
		obj += min;
	}
	
	return obj;
}
#endif

bool PAMClustering::swap()
{
	double min = std::numeric_limits<double>::max();
	unsigned int i_min, h_min;
	
#ifdef PAMCLUSTERING_BRUTE_FORCE
	ivec tmp_labels;
#endif
		
	// determine swap operation with minimum total cost
	ivec::iterator iti;
	for( iti = m_medoids.begin(); iti != m_medoids.end(); ++iti )
	{
		unsigned int i = *iti;
		
		for( unsigned int h=0; h < N; ++h )
		{
			if( h==i ) 
				continue;
			
			// h shouldn't be selected
			if( isSelected(h) ) 
				continue;
			
			// compute total cost of swap(i,h)
			double total = 0.0;
		#ifdef PAMCLUSTERING_BRUTE_FORCE			
			//--- VIA BRUTE FORCE ---
			
			// temporarily replace i with h
			m_medoids[ m_labels[i] ] = h;
			
			total = label(tmp_labels) - m_objective;
			
			m_medoids[ m_labels[i] ] = i;
			
		#else //--- VIA COST UPDATE ---
			//  
			// HERE IS SOMEWHERE AN ERROR LEADING TO TOTALLY WRONG RESULTS!
			//
			for( unsigned int j=0; j < N; ++j )
			{
				if( isSelected(j) )
					continue;
				
				if( m_labels[j] == m_labels[i] )
				{
					// j in same cluster as i
					
					// second nearest medoid to j
					unsigned int j2 = m_second[j];
					
					// swapping i and h would..
					if( D(j,h) >= D(j,j2) )
						// ..assign j to cluster j2
						total += D(j,j2) - D(j,i);
					else
						// ..assign j to cluster h
						total += D(j,h) - D(j,i);
				}
				else
				{
					// j not in same cluster as i but j2
					unsigned int j2 = m_labels[j];
					
					if( D(j,j2) < D(j,h) )	
						// j stays in its cluster j2
						total += 0;
					else
						// j moves from cluster j2 into cluster h
						total += D(j,h) - D(j,j2);
				}
			}		
		#endif //-----
			
			if( total < min )
			{
				min = total;
				i_min = i;
				h_min = h;
			}
		}		
	}
	
	if( min < 0 )
	{		
		// replace medoid i with h
		m_medoids[ m_labels[i_min] ] = h_min;
		
		// mark h as "selected"
		m_selected.push_back( h_min );
		
		// relabel
		label();

	  #ifdef PAMCLUSTERING_DEBUG_OUT
		std::cerr << "PAM   min=" << std::fixed << std::setprecision(4) << std::setw(9) << min 
		          //~ << " i=" << std::setw(6) << i_min 
				  //~ << " h=" << std::setw(6) << h_min 
				     << "   obj=" << std::fixed << std::setprecision(4) << std::setw(9) << m_objective 
				  << std::endl;
	  #endif
 
		// not converged, more iterations are needed
		return false;
	}

	// final configuration
	return true;
}
