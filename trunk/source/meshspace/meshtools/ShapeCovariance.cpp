#include "ShapeCovariance.h"

namespace ShapeCovariance {

void vectorizeCovariance( const Eigen::Matrix3d& Sigma, Eigen::VectorXd& v )
{
	v.resize( 6 );
	// Compactly store symmetric covariance matrix as 6D column vector
	v(0) = Sigma(0,0);  v(1) = Sigma(0,1);  v(2) = Sigma(0,2);
	                    v(3) = Sigma(1,1);  v(4) = Sigma(1,2);
											v(5) = Sigma(2,2);
}

void devectorizeCovariance( const Eigen::VectorXd v, Eigen::Matrix3d& Sigma )
{
	Sigma(0,0) = v(0);  Sigma(0,1) = v(1);  Sigma(0,2) = v(2);
	Sigma(1,0) = v(1);  Sigma(1,1) = v(3);  Sigma(1,2) = v(4);
	Sigma(2,0) = v(2);  Sigma(2,1) = v(4);  Sigma(2,2) = v(5);
}

void computeSampleCovariance( const Eigen::MatrixXd& X, Eigen::MatrixXd& S  )
{
	int p = (int)X.rows() / 3;
	
	S.resize( 6, p );
	
	// #pragma omp parallel for
	for( int i=0; i < p; ++i )
	{
		Eigen::Matrix3d Sigma;
		sampleCovariance( X.block( 3*i, 0, 3, X.cols() ), Sigma );
		Eigen::VectorXd v;
		vectorizeCovariance( Sigma, v );
		S.col(i) = v;
	}	
}

void computeSampleCovariance( const Eigen::MatrixXd& X, std::vector<Eigen::Matrix3d> S )
{
	using ShapeCovariance::sampleCovariance;	
	
	int p = (int)X.rows() / 3;
	
	S.resize( p );	
	
	#pragma omp parallel for
	for( int i=0; i < p; ++i )
	{
		sampleCovariance( X.block( 3*p, 0, 3, X.cols() ), S[i] );
	}	
}

}; // namespace ShapeCovariance
