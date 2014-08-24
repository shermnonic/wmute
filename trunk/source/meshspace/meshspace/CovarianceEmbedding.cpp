#include "CovarianceEmbedding.h"
#include "CovarianceAnalysis.h"
#include "PCA.h" // centerMatrix()
#include <iostream>
#include <fstream>
#include <cmath> // std::floor()
using std::cout;
using std::cerr;
using std::endl;
using CovarianceAnalysis::covarDistRiemannian;

CovarianceEmbedding::Labels CovarianceEmbedding
::genLabels( int numFrames, int numGroups, int groupSize )
{
	Labels labels;

	for( int g=0; g < numGroups; g++ )
	{
		double center = numFrames * (g+1) / (double)(numGroups+1);
		int i0 = (int)floor(center-groupSize/2),
			i1 = (int)floor(center+groupSize/2);
		if( i0 < 0 || i1 > numFrames ) // Skip first/last segment if too small
			continue;

		for( int i=i0; i < i1; i++ )
			labels.insert(std::pair<int,int>( g, std::min( i, numFrames-1 ) ));
	}

#if 1 // DEBUG OUT
	for( Labels::iterator it=labels.begin(); it!=labels.end(); ++it )
		cout << "Group " << it->first << " index " << it->second << endl;
#endif
	return labels;
}

CovarianceEmbedding::Labels CovarianceEmbedding
::genLabels( int numFrames, int numGroups, bool slidingGroups )
{
	Labels labels;

	if( slidingGroups )
	{
		// Overlapping segmentation
		// Time domain: |-----|
		// Segmentation:|--|
		//               |--|
		//                |--|
		//                 |--|

		// Length of a segment (=group size)
		int size = numFrames - numGroups + 1;

		int group=0;
		for( int i0=0; i0 < (numFrames - size + 1); i0++ )
		{
			for( int i=i0; i < i0+size; i++ )
				labels.insert(std::pair<int,int>( group, std::min( i, numFrames-1 ) ));
			group++;
		}
	}
	else
	{
		// Non-overlapping segmentation of frames into groups
		// Time domain:  |-----|
		// Segmentation: |-|-|-|

		// All groups have to have the same size
		int size = (int)floor( numFrames / (double)numGroups );
		int group=0;
		for( int i0=0; (i0+size) <= numFrames; i0+=size )
		{
			for( int i=i0; i < i0+size; i++ )
				labels.insert(std::pair<int,int>( group, std::min( i, numFrames-1 ) ));
			group++;
		}
	}

#if 1 // DEBUG OUT
	for( Labels::iterator it=labels.begin(); it!=labels.end(); ++it )
		cout << "Group " << it->first << " index " << it->second << endl;
#endif
	return labels;
}

void CovarianceEmbedding
::setup( MeshBuffer& samples, Labels labels )
{
	// Map vertex buffer to data matrix
	Eigen::Map<Eigen::MatrixXf> Xf( &(samples.vbuffer()[0]), samples.numVertices()*3, samples.numFrames() );
	
	// Copy float to double matrix (since we internally mostly use double matrices)
	Eigen::MatrixXd X = Xf.cast<double>();

	// Center matrix (once in advance; alternatively one could center each group matrix!)
	centerMatrix( X );

	// Compute set of covariance matrices, one for each group label
	MatrixArray covarSet;

	// Iterate over keys in the map
	// (http://stackoverflow.com/questions/9371236/is-there-an-iterator-across-unique-keys-in-a-stdmultimap)
	int count=0;
	for( Labels::iterator it=labels.begin(); it!=labels.end(); it=labels.upper_bound(it->first) )
	{
		// Collect all column indices for current label
		std::vector<int> cols;
		std::pair<Labels::iterator, Labels::iterator> ret;
		ret = labels.equal_range( it->first );
		for( Labels::iterator it=ret.first; it != ret.second; ++it )
		{
			cols.push_back( it->second );
		}

		if( !cols.empty() ) // Sanity
		{
			count++;
			cout << "Computing scatter matrix " << count << endl;			

			// Assemble temporary matrix from labelled columns
			Eigen::MatrixXd Xtmp( X.rows(), cols.size() );
			for( int j=0; j < cols.size(); j++ )
			{
				Xtmp.col(j) = X.col( cols[j] );
			}

			// Center individual group matrices!
			//centerMatrix( Xtmp );

			// Compute scatter matrix
			// (NOTE: We do not compute the full covariance matrix because
			//        it would be too large!)
			Eigen::MatrixXd Stmp = (Xtmp.transpose() * Xtmp) * (1./(double)cols.size());

			if( m_verbosity > 1 )
				cout << "Xtmp is " << Xtmp.rows() << "x" << Xtmp.cols() << ",  "
				     << "Stmp is " << Stmp.rows() << "x" << Stmp.cols() << endl;

			if( m_verbosity > 2 )
				cout << "Scatter matrix " << count << "=" << endl << Stmp << endl;
			
			covarSet.push_back( Stmp );
		}
	}

	m_covarSet = covarSet;
}

void CovarianceEmbedding
::compute()
{
	if( m_covarSet.empty() )
		return;

	int n = (int)m_covarSet.size();

	// Compute pair-wise distance matrix
	cout << "Computing pair-wise distance matrix..." << endl;
	Eigen::MatrixXd D = Eigen::MatrixXd::Identity( n, n );
	for( int i=0; i < n; i++ )
		for( int j=0; j < i; j++ )
		{
			// TODO: Try other metrics!
			D(i,j) = covarDistRiemannian( m_covarSet[i], m_covarSet[j] );
			D(j,i) = D(i,j);
		}

	if( m_verbosity > 2 )
		cout << "Distance matrix =" << endl << D << endl;

	// Compute metric embedding
	cout << "Computing metric embedding..." << endl;
	m_mds.setDistanceMatrix( D );
	m_mds.computeEmbedding( 0 );

	// DEBUG OUT
	std::ofstream f("embedding.txt");
	if( f.is_open() )
		f << m_mds.getCoordinates();
	f.close();
}
