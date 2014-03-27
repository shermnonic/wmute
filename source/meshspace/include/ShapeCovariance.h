// Max Hermann, Feb 2014
#ifndef SHAPECOVARIANCE_H
#define SHAPECOVARIANCE_H

#include <Eigen/Dense>
#include <vector>

/** @addtogroup meshtools
  * @{ */
  
/// Collection of functions for anatomic covariance analysis on 3D points
/// @author Max Hermann (hermann@cs.uni-bonn.de)
namespace ShapeCovariance {

//=============================================================================
//  Template only
//=============================================================================

/// Sample covariance tensor of a set of 3D displacement vectors.
/// @param [in]  D      Input matrix with 3D vectors in columns	
/// @param [out] Sigma  Sample 3x3 covariance tensor
/**
	Kindlmann G, Weinstein D, Lee A, Toga A, Thompson P.
	"Visualization of anatomic covariance tensor fields."
	Conf Proc IEEE Eng Med Biol Soc. 2004;3:1842-5.
*/	
template <typename Derived1, typename Derived2>
void sampleCovariance( const Eigen::MatrixBase<Derived1>& D, Eigen::MatrixBase<Derived2>& Sigma )
{
	int n = (int)D.cols();	
	Sigma = (D * D.transpose()) / ((double)n - 1.);
}

//=============================================================================
//  Specializations
//=============================================================================

//-----------------------------------------------------------------------------
// Specialized sample covariance functions
//----------------------------------------------------------------------------- 

///@{ Write a symmetric 3x3 matrix as 6D column vector and vice versa
void vectorizeCovariance( const Eigen::Matrix3d& Sigma, Eigen::VectorXd& v );
void devectorizeCovariance( const Eigen::VectorXd v, Eigen::Matrix3d& Sigma );
///@}

/// Compute sample covariance tensors for zero-mean data matrix
/// @param [in]  X  Data matrix with vectorized 3D displacements in columns.
/// @param [out] S  Covariance matrices encoded as 6D column vectors.
void computeSampleCovariance( const Eigen::MatrixXd& X, Eigen::MatrixXd& S  );

/// Provided for convenience
void computeSampleCovariance( const Eigen::MatrixXd& X, std::vector<Eigen::Matrix3d> S );
///@}

//-----------------------------------------------------------------------------
// Inter point covariance functions
//-----------------------------------------------------------------------------

/// Compute inter point covariance tensor (aka overview tensor).
/// @param[in]  B      Shape basis, i.e. eigenvectors scaled by eigenvalues
/// @param[in]  gamma  Tikhonov regularization parameter
/// @param[out] G      Inter point covariance tensor (vectorized in columns, 6xn)
void computeInterPointCovariance( const Eigen::MatrixXd& B, double gamma, Eigen::MatrixXd& G );

/// Precompute part of interaction tensor depending solely on p.
/// @param[in]  B      Shape basis, i.e. eigenvectors scaled by eigenvalues
/// @param[in]  gamma  Tikhonov regularization parameter
/// @param[out] Z      Part of interaction tensor depending solely on p (concatenated horizontally, mx3n)
void computeInterPointZ( const Eigen::MatrixXd& B, double gamma, Eigen::MatrixXd& Z );

/// Compute part of interaction tensor depending solely on p.
/// @param[in]  Bp     Selected rows of shape basis corresponding to point p (3xn)
/// @param[in]  gamma  Tikhonov regularization parameter
/// @param[out] Zp     Part of interaction tensor depending solely on p (3x3)
void computeInterPointZp( const Eigen::MatrixXd& Bp, double gamma, Eigen::MatrixXd& Zp );

}; // namespace ShapeCovariance

/** @} */ // end group

#endif // SHAPECOVARIANCE_H
