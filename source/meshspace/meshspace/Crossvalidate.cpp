#include "Crossvalidate.h"
#include <ShapeCovariance.h>
#include <ShapePCA.h>
#include <PCA.h>
#include <iostream>

#include "MatrixUtilities.h"
using MatrixUtilities::removeColumn;

using ShapeCovariance::computeInterPointZ;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using Eigen::Vector3d;

//-----------------------------------------------------------------------------
//  error function
//----------------------------------------------------------------------------- 
double crossvalidate_error( const VectorXd& x0, const VectorXd& x )
{
	// Average L2 error between displacement vectors
	double err=0.0;
	unsigned n = (unsigned)x0.rows() / 3;
	for( unsigned p=0; p < n; p++ )
	{
		err += (x0.segment(3*p,3) - x.segment(3*p,3)).norm();
	}
	return (err / (double)n);

	// Global L2 error (does not make so much sense for vector fields)
	//return (x0 - x).norm(); // L2 error
}

//-----------------------------------------------------------------------------
//  crossvalidate() - internal function
//----------------------------------------------------------------------------- 
double crossvalidate( const MatrixXd& X, const VectorXd& x0, const PCAModel& pca, double gamma )
{
	int n = (int)X.rows() / 3;  // Number of 3D vectors	
	int m = (int)pca.PC.cols(); // Number of basis vectors	
	
	// Basis
	MatrixXd B = pca.PC * pca.ev.asDiagonal();

	// Compute Zp operators (depending on gamma!)
	MatrixXd Z;
	computeInterPointZ( B, gamma, Z );
	
	VectorXd x = x0 - pca.mu;
	
	VectorXd err( n );
	
	//#pragma omp parallel for
	for( int p=0; p < n; ++p )
	{		
		// Locally optimal reconstruction coefficients
		// 	c_opt = Zp * (x0 - mu)
	  #if 0
		MatrixXd Zp = Z.block( 0, 3*p, Z.rows(), 3 );
		VectorXd xp = x.segment(3*p,3);
		VectorXd c_opt = Zp * xp;
	  #else
		VectorXd c_opt = Z.block( 0, 3*p, Z.rows(), 3 ) * x.segment(3*p,3);
	  #endif
		
		// Global reconstruction error
		// 	err = || x0 - B*c_opt + mu ||
		VectorXd x_recon = B * c_opt + pca.mu;
		err(p) = crossvalidate_error( x0, x_recon );
	}
	
	// Average error
	return err.sum() / (double)n;
}

//-----------------------------------------------------------------------------
//  crossvalidate()
//----------------------------------------------------------------------------- 
void crossvalidate( const MatrixXd& X, const std::vector<double>& gamma, std::vector<double>& error, std::vector<double>& baseline )
{
	using namespace std;

	unsigned n = (unsigned)X.rows() / 3;  // Number of 3D vectors	
	unsigned m = (unsigned)X.cols();      // Number of shapes

	unsigned int step = 5;

	baseline.clear();
	
	// Record error per gamma sample
	VectorXd err = VectorXd::Zero( gamma.size() );
	unsigned count = m / step;
	for( unsigned i=0; i < m; i+=step )	
	{
		cout << "Cross-validation " << i+1 << " / " << m << endl;

		// Leave out i-th column of X
		VectorXd xi = X.col(i);
		MatrixXd Xi = X;
		removeColumn( Xi, i );
		
		// Compute shape model on reduced matrix
		PCAModel pca;
		computePCA( Xi, pca.PC, pca.ev, pca.mu );

		// Baseline comparison between left out shape and mean
		baseline.push_back( crossvalidate_error( xi, pca.mu ) );
		cout << "  baseline=" << baseline.back() << endl;
		
		// Sample gamma parameter space
		for( unsigned g=0; g < gamma.size(); g++ )
		{
			// cout << "  Sampling gamma " << g+1 << " / " << gamma.size() << endl;

			// Accumulate error
			double V0 = crossvalidate( Xi, xi, pca, gamma[g] );
			cout << "    gamma=" << gamma[g] << ", error=" << V0 << endl;
			err(g) += V0 / (double)count;
		}
		cout << endl;
	}
	
	// Copy result
	error.resize( gamma.size() );
	for( unsigned i=0; i < gamma.size(); i++ )
		error[i] = err(i);
}
