// Max Hermann, Mar 2014
#ifndef CROSSVALIDATE_H
#define CROSSVALIDATE_H

#include <Eigen/Dense>
#include <vector>

/// Cross validate gamma parameter for model-based deformation & inter-point covariance analysis
/// @param[in]  X      Shape dataset, i.e. displacement vector fields (vectorized in columns)
/// @param[in]  gamma  Sampling of parameter space
/// @param[out] error  Reconstruction error for each sampling value gamma
void crossvalidate( const Eigen::MatrixXd& X, const std::vector<double>& gamma, 
				    std::vector<double>& error, std::vector<double>& baseline );

#endif // CROSSVALIDATE_H
