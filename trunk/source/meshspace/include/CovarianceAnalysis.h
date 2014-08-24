// Max Hermann, July 2014
#ifndef COVARIANCEANALYSIS_H
#define COVARIANCEANALYSIS_H
#include <Eigen/Dense>

/// General covariance analysis for arbitrary dimensions based on distance 
/// functions between covariance matrices.
///
/// For specific functions operating on covariances of 3D point data see
/// \a ShapeCovariance.
///
/// @author Max Hermann (hermann@cs.uni-bonn.de)
namespace CovarianceAnalysis {

/// Riemannian distance between covariance matrices
/// This metric is described in detail in:
/// - Mitteroecker & Bookstein, "The ontogenetic trajectory of the phenotypic
///   covariance matrix, with examples from craniofacial shape in rats and humans"
///   Evolution 63-3: 727-737, 2009
/// The Riemannian metric is only valid for non rank-deficient matrices!
double covarDistRiemannian( const Eigen::MatrixXd& A, const Eigen::MatrixXd& B );

}; // namespace CovarianceAnalysis

#endif // COVARIANCEANALYSIS_H
