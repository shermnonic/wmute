#ifndef MESHLAPLACIAN_H
#define MESHLAPLACIAN_H

#include <meshtools.h> // OpenMesh and Eigen/Dense, meshtools::Mesh
#include <Eigen/Sparse> // Eigen::SparseMatrix, Eigen::Triplet

/**
	\class MeshLaplacian
	
	Eigenvalue decomposition of a discrete Laplace-Beltrami Operator on a mesh.	

	For some background see the following survey paper
	- Zhang et al. "Spectral Mesh Processing", Computer Graphics Forum 2010
	  https://www.cs.sfu.ca/~haoz/pubs/zhang_cgf10_spect_survey.pdf
	
	\author Max Hermann (hermann@cs.uni-bonn.de)
*/
class MeshLaplacian
{
public:
	// OpenMesh mesh type
	typedef meshtools::Mesh Mesh;
	// Eign dense matrix types
	typedef Eigen::MatrixXd Matrix;
	typedef Eigen::VectorXd Vector;
private:
	// Types to handle sparse matrices with Eigen
	typedef Eigen::Triplet<double,int>  Triplet;
	typedef std::vector<Triplet>        TripletArray;
	typedef Eigen::SparseMatrix<double> SparseMatrix;

public:
	/// Compute eigenvalue decomposition of a (triangle) mesh Laplacian
	void compute( const Mesh& mesh );

	template<typename STD_ITERATEABLE_ARRAY>
	void getMode( int mode, STD_ITERATEABLE_ARRAY& values ) const;

protected:
	void buildSystem( const Mesh& mesh, TripletArray& W, Vector& D );
	
	void solveSparse( const Mesh& mesh );
	void solveDense ( const Mesh& mesh );

private:
	Matrix m_eigenvectors;
	Matrix m_eigenvalues;
};

//-----------------------------------------------------------------------------
//  Template implementation
//-----------------------------------------------------------------------------
template<typename T>
void MeshLaplacian::getMode( int mode, T& values ) const
{
	values.resize( m_eigenvectors.rows() );
	int row=0;
	for( T::iterator it=values.begin(); it!=values.end(); ++it, row++ )
	{
		*it = m_eigenvectors( row, mode );
	}
}

#endif // MESHLAPLACIAN_H
