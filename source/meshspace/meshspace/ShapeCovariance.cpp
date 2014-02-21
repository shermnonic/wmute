#include "ShapeCovariance.h"

namespace ShapeCovariance {

void vectorizeCovariance( const Eigen::Matrix33d& Sigma, Eigen::VectorXd v )
{
	v.resize( 6 );
	// Compactly store symmetric covariance matrix as 6D column vector
	v(0) = Sigma(0,0);  v(1) = Sigma(0,1);  v(2) = Sigma(0,2);
	                    v(3) = Sigma(1,1);  v(4) = Sigma(1,2);
											v(5) = Sigma(2,2);
}

void devectorizeCovariance( const Eigen::VectorXd v, Eigen::Matrix33d& Sigma )
{
	Sigma(0,0) = v(0);  Sigma(0,1) = v(1);  Sigma(0,2) = v(2);
	                    Sigma(1,1) = v(3);  Sigma(1,2) = v(4);
											Sigma(2,2) = v(5);	
}

void computeSampleCovariance( const Eigen::MatrixXd& X, Eigen::MatrixXd& S  )
{
	int p = X.rows() / 3;
	
	S.resize( 6, p );
	
	#pragma omp parallel for
	for( int i=0; i < p; ++i )
	{
		Matrix33d Sigma;
		sampleCovariance( X.block( 3*p, 0, 3, X.cols() ), Sigma );
		vectorizeCovariance( Sigma, S.col(i) );
	}	
}

void computeSampleCovariance( const Eigen::MatrixXd& X, std::vector<Eigen::Matrix33d> S )
{
	using ShapeCovariance::sampleCovariance;	
	
	int p = X.rows() / 3;
	
	S.resize( p );	
	
	#pragma omp parallel for
	for( int i=0; i < p; ++i )
	{
		sampleCovariance( X.block( 3*p, 0, 3, X.cols() ), S[i] );
	}	
}

}; // namespace ShapeCovariance
