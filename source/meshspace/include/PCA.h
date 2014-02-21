// Header-only Principal Component Analysis
// Max Hermann, Feb 2014
#ifndef PCA_H
#define PCA_H

#include <Eigen/Dense>

/// Center a data matrix row- or column-wise, i.e. make row/columns zero mean.
/// @param [in,out] X   Input data matrix will be centered in-place
/// @param [out]    mu  Column or row average vector
/// @param [in]     centerRows  Choose if columns or rows are centered
template <typename Derived1, typename Derived2>
void centerMatrix( Eigen::MatrixBase<Derived1>& X, Eigen::MatrixBase<Derived2>& mu, 
	bool centerRows=true )
{
	// Center data
	if( centerRows )
	{
		int n = (int)X.cols();
		
		mu = X.rowwise().sum() / n;
		#pragma omp parallel for
		for( int i=0; i < X.cols(); ++i )
			X.col(i) -= mu;
	}
	else		
	{
		int n = (int)X.rows();
		
		mu = X.colwise().sum() / n;
		#pragma omp parallel for
		for( int i=0; i < X.rows(); ++i )
			X.row(i) -= mu;
	}
}

/// Center a data matrix, convenience wrapper with fewer arguments.
template <typename Derived1>
void centerMatrix( Eigen::MatrixBase<Derived1>& X, bool centerRows=true )
{
	Eigen::VectorXd mu;
	centerMatrix( X, mu, centerRows );
}

/// Principal Component Analysis (PCA) of an arbitrary data matrix
/// @param [in]  X_ Input dataset with one sample per column
/// @param [out] PC Principal components (= eigenvectors of sample covariance)
/// @param [out] ev Eigenvalues vector (= standard deviations)
/// @param [out] mu Sample mean column vector
/// @author Max Hermann (hermann@cs.uni-bonn.de)
template <typename Derived1, typename Derived2, typename Derived3, typename Derived4>
void computePCA( const Eigen::MatrixBase<Derived1>& X_, Eigen::MatrixBase<Derived2>& PC, 
	Eigen::MatrixBase<Derived3>& ev, Eigen::MatrixBase<Derived4>& mu )
{
	// FIXME: Choose temporary matrix type according to template argument!
	typedef Eigen::MatrixXd TempMatrix;
	TempMatrix X( X_ );
	
	// Number of samples (= columns)
	int n = (int)X.cols();
	
	// Center data
	centerMatrix( X, mu );
		// Same as: 
		// 		mu = X.rowwise().sum() / n;
		// 		#pragma omp parallel for
		// 		for( int i=0; i < X.cols(); ++i )
		//  		X.col(i) -= mu;
	
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
