#include "CovarianceAnalysis.h"
#include <cmath>
#include <limits>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

using Eigen::MatrixXd;
using Eigen::VectorXd;

namespace CovarianceAnalysis {

double covarDistRiemannian( const Eigen::MatrixXd& A, const Eigen::MatrixXd& B )
{
	// The Riemannian distance is computed as follows
	//
	// dR(A,B) = || log(A^-0.5 . B . A^-0.5) || 
	//         = sqrt( log(lambda_1)^2 + ... + log(lambda_n)^2 )
	//
	// where lambda_i are the generalized eigenvalues of B^-1.A

#if 1 // SELF-ADJOINT SOLVER

	// Use solver for symmetric generalized eigenvalue problem Av = lambda.Bv
	// where A must be symmetric and B positive definite.
	Eigen::GeneralizedSelfAdjointEigenSolver<MatrixXd> solver;
	solver.compute( A, B, Eigen::EigenvaluesOnly );

	if( solver.info() == Eigen::NoConvergence )
	{
		cerr << "CovarianceAnalysis::covarDistRiemannian() : "
			"GeneralizedSelfAdjointEigenSolver did not converge!" << endl;
		return std::numeric_limits<double>::infinity();
	}

	VectorXd ev = solver.eigenvalues();
#else // PSEUDO INVERSE

	// Solve explicitly for eigenvalues of B^(-1).A
	MatrixXd Binverse = B.inverse();
	MatrixXd X = Binverse * A;
	X = (X.transpose() + X) * 0.5; // Symmetrize

	Eigen::JacobiSVD<MatrixXd> svd;
	svd.compute( X ); // By default only singular values are computed
	VectorXd ev = svd.singularValues().cwiseSqrt();
#endif

	//cout << "Eigenvalues = " << endl << ev.transpose() << endl;

	double sum = 0.;
	for( int i=0; i < ev.size(); i++ )
	{
		if( ev.coeff(i) > 0. )
		{
			double l = log( ev.coeff(i) );
			sum += l*l;
		}
		else
		{
			cerr << "CovarianceAnalysis::covarDistRiemannian() : "
				"Encountered negative generalized eigenvalue!" << endl;
		}
	}
	return sqrt(sum);
}

}; // namespace CovarianceAnalysis
