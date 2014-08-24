#ifndef MDSEMBEDDING_H
#define MDSEMBEDDING_H

#include <Eigen/Dense>

/**
	Classical Multidimensional Scaling to find a Euclidean point configuration
	approximating a given dissimilarity (distance) matrix.
	
	A full spectral decomposition is computed, so please avoid feeding too 
	large distance matrices! Be aware that input is not checked for symmetry!
	
	\author Max Hermann (hermann@cs.uni-bonn.de)
	\date May 23, 2014
*/
class MDSEmbedding
{
public:
	void setDistanceMatrix( double* D, size_t n );
	void setDistanceMatrix( const Eigen::MatrixXd& D );
	void computeEmbedding( int verbosity=0 );

	double getCoordinate( int idx, int dim ) const;
	Eigen::MatrixXd& getCoordinates() { return m_C; }

	///@{ Convenience functions for 2D and 3D embeddings.
	double getXCoordinate( int idx ) const { return getCoordinate(idx,0); }
	double getYCoordinate( int idx ) const { return getCoordinate(idx,1); }
	double getZCoordinate( int idx ) const { return getCoordinate(idx,2); }
	///@}

private:
	Eigen::MatrixXd 
		m_D,  ///< Distance matrix
		m_ev, ///< Eigenvalues
		m_V,  ///< Eigenvectors
		m_C;  ///< Embedded coordinates, items in rows, dimensions in columns
};

#endif // MDSEMBEDDING_H
