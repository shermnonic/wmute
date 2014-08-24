#ifndef COVARIANCEEMBEDDING_H
#define COVARIANCEEMBEDDING_H

#include <Eigen/Dense>
#include <map>
#include <vector>
#include "MDSEmbedding.h"
#include "MeshBuffer.h"

class CovarianceEmbedding
{
public:
	/// Map group label to column of MeshBuffer vertex buffer matrix
	typedef std::multimap<int,int> Labels;
	/// Internal type to store set of (covariance) matrices
	typedef std::vector<Eigen::MatrixXd> MatrixArray;

	/// Generate equally spaced groups of an animation sequence
	static Labels genLabels( int numFrames, int numGroups, bool slidingGroups=false );	
	static Labels genLabels( int numFrames, int numGroups, int size );

	/// C'tor
	CovarianceEmbedding()
		: m_verbosity(0) 
		{}

	void clear()
	{
		m_covarSet.clear();
	}

	/// Setup covariance matrices from labelled mesh sequence.
	void setup( /*const*/ MeshBuffer& samples, Labels labels );

	/// Compute pair-wise distances between covariances set via \a setup().
	void compute();

	const MDSEmbedding& getMDSEmbedding() const { return m_mds; }

private:	
	int m_verbosity;
	MatrixArray  m_covarSet;
	MDSEmbedding m_mds;	
};

#endif // COVARIANCEEMBEDDING_H
