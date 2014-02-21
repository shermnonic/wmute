#ifndef SHAPEPCA_H
#define SHAPEPCA_H

#include "meshtools.h"
#include "MeshBuffer.h"
#include <Eigen/Dense>

/** @addtogroup meshtools
  * @{ */

/// PCA model, result data structure for \a computePCA() functions.
struct PCAModel
{
	Eigen::MatrixXd PC; /// Eigenvectors of sample covariance matrix
	Eigen::VectorXd ev; /// Eigenvalues of sample covariance matrix
	Eigen::VectorXd mu;	/// Sample mean (column vector)

	Eigen::MatrixXd X;  /// Zero mean data matrix (useful for further analysis)
};

/// Compute PCA of MeshBuffer vertex data
/// \param[in]  samples  Input MeshBuffer
/// \param[out] pcmb     Output MeshBuffer with mean shape
/// \param[out] model    \a PCAModel with eigenvectors, ~values and mean
/// \param[out] mshape   Mean shape as \a meshtools::Mesh
void computePCA( /*const*/ MeshBuffer& samples, MeshBuffer& pcmb, PCAModel& model, meshtools::Mesh& mshape );

/// De-vectorize a 3n x 1 vector into a 3 x n matrix
Eigen::Matrix3Xd reshape( const Eigen::VectorXd& v );

/** @} */ // end group

#endif // SHAPEPCA_H
