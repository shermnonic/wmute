#include "ShapeCovariance.h"

namespace ShapeCovariance {

using Eigen::MatrixXd;
using Eigen::Matrix3d;
using Eigen::VectorXd;

void vectorizeCovariance( const Matrix3d& Sigma, VectorXd& v )
{
	v.resize( 6 );
	// Compactly store symmetric covariance matrix as 6D column vector
	v(0) = Sigma(0,0);  v(1) = Sigma(0,1);  v(2) = Sigma(0,2);
	                    v(3) = Sigma(1,1);  v(4) = Sigma(1,2);
											v(5) = Sigma(2,2);
}

void devectorizeCovariance( const VectorXd v, Matrix3d& Sigma )
{
	Sigma(0,0) = v(0);  Sigma(0,1) = v(1);  Sigma(0,2) = v(2);
	Sigma(1,0) = v(1);  Sigma(1,1) = v(3);  Sigma(1,2) = v(4);
	Sigma(2,0) = v(2);  Sigma(2,1) = v(4);  Sigma(2,2) = v(5);
}

void computeSampleCovariance( const MatrixXd& X, MatrixXd& S  )
{
	int p = (int)X.rows() / 3;
	
	S.resize( 6, p );
	
	// #pragma omp parallel for
	for( int i=0; i < p; ++i )
	{
		Matrix3d Sigma;
		sampleCovariance( X.block( 3*i, 0, 3, X.cols() ), Sigma );
		VectorXd v;
		vectorizeCovariance( Sigma, v );
		S.col(i) = v;
	}	
}

void computeSampleCovariance( const MatrixXd& X, std::vector<Matrix3d> S )
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

void computeInterPointZp( const MatrixXd& Bp, double gamma, MatrixXd& Zp )
{
	// Precompute part of interaction tensor depending solely on p
	MatrixXd 
		A = Bp.transpose()*Bp + gamma*MatrixXd::Identity( Bp.cols(), Bp.cols() );
	
	Zp = A.inverse() * Bp.transpose();
}

void computeInterPointZ( const MatrixXd& B, double gamma, MatrixXd& Z )
{
	int n = (int)B.rows() / 3;  // Number of 3D vectors	

	// Buffer for all Zp matrices, concatenated horizontally
	Z.resize( B.cols(), 3*n );
	
	// Pre-compute Zp matrices	
	for( int p=0; p < n; ++p )
	{
		// Compute part of interaction tensor depending solely on p
		MatrixXd Zp;
		computeInterPointZp( B.block( 3*p, 0, 3, B.cols() ), gamma, Zp );

		Z.block( 0, 3*p, B.cols(), 3 ) = Zp;
	}
}

void computeInterPointCovariance( const MatrixXd& B, double gamma, MatrixXd& G )
{
	int n = (int)B.rows() / 3;  // Number of 3D vectors	
	
	// Vectorized covariance matrices in columns
	G.resize( 6, n );
	
	// Compute overview tensor
	#pragma omp parallel for
	for( int p=0; p < n; ++p )
	{	
		// Precompute part of interaction tensor depending solely on p
		MatrixXd Zp;
		computeInterPointZp( B.block( 3*p, 0, 3, B.cols() ), gamma, Zp );
		
		// Sum over quadratic forms
		Matrix3d Gp = Matrix3d::Zero();
		for( int q=0; q < n; ++q )
		{
			MatrixXd 
				Bq = B.block( 3*q, 0, 3, B.cols() ),
				Zpq = Bq * Zp;
			
			Gp = Gp + Zpq.transpose() * Zpq;
		}		
		Gp /= (double)n;
		
		// Store result tensor in output matrix G
		VectorXd v;
		vectorizeCovariance( Gp, v );
		G.col(p) = v;
	}
}

}; // namespace ShapeCovariance
