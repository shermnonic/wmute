// Max Hermann, Feb 2014
#ifndef SHAPECOVARIANCE_H
#define SHAPECOVARIANCE_H

#include <Eigen/Dense>

/// Collection of functions for anatomic covariance analysis on 3D points
/// @author Max Hermann (hermann@cs.uni-bonn.de)
namespace ShapeCovariance {

/// Sample covariance tensor of a set of 3D displacement vectors.
/// @param [in]  D      Input matrix with 3D vectors in columns	
/// @param [out] Sigma  Sample 3x3 covariance tensor
/**
	Kindlmann G, Weinstein D, Lee A, Toga A, Thompson P.
	"Visualization of anatomic covariance tensor fields."
	Conf Proc IEEE Eng Med Biol Soc. 2004;3:1842-5.
*/	
template <typename Derived1, typename Derived2>
void sampleCovariance( const Eigen::MatrixBase<Derived1>& D, Eigen::MatrixBase<Derived1>& Sigma )
{
	int n = D.cols();	
	Sigma = (D * D.transpose()) / ((double)n - 1.);
}
	
}; // namespace ShapeCovariance

#endif // SHAPECOVARIANCE_H
