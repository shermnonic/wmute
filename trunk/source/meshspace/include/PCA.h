#ifndef PCA_H
#define PCA_H

#include <Eigen/Dense>

/// Principal Component Analysis (PCA) of an arbitrary data matrix
/// @param [in]  X_ Input dataset with one sample per column
/// @param [out] PC Principal components (= eigenvectors of sample covariance)
/// @param [out] ev Eigenvalues vector (= standard deviations)
/// @param [out] mu Sample mean column vector
template <typename Derived1, typename Derived2, typename Derived3, typename Derived4>
void computePCA( const Eigen::MatrixBase<Derived1>& X_, Eigen::MatrixBase<Derived2>& PC, 
	Eigen::MatrixBase<Derived3>& ev, Eigen::MatrixBase<Derived4>& mu )
{
	typedef Eigen::MatrixXd TempMatrix;
	TempMatrix X( X_ );
	
	// Number of samples (= columns)
	int n = (int)X.cols();
	
	// Center data
	mu = X.rowwise().sum() / n;
	#pragma omp parallel for
	for( int i=0; i < X.cols(); ++i )
		X.col(i) -= mu;
	
	// Scatter matrix
	TempMatrix S;
	if( X.rows() >= X.cols() )
	{
		// Smaller scatter matrix X'X
		S = X.transpose() * X;				
	}
	else
	{
		// Original scatter matrix XX'
		S = X * X.transpose();
	}
	
	// Diagonalization via SVD
	Eigen::JacobiSVD< TempMatrix > svd( S, Eigen::ComputeFullU );
	
	// XX' and X'X share eigenvalues
	ev = (svd.singularValues() / (n-1.)).cwiseSqrt();
	
	if( X.rows() >= X.cols() )
	{
		// Reconstruct eigenvectors of XX'		
		PC = X * svd.matrixU() * svd.singularValues().cwiseSqrt().asDiagonal().inverse();
	}
	else
	{
		PC = svd.matrixU();
	}
}

#endif // PCA_H
