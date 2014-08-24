#include "MeshLaplacian.h"
#include <cmath>
#include <vector>

//-----------------------------------------------------------------------------
/// Compute cotangent of non-degenerate triangle abc at vertex b w/o trigonometry.
/// E.g. see Meyer et al. 2002, "Generalized Barycentric Coordinates on Irregular
/// Polygons", http://www.geometry.caltech.edu/pubs/MHBD02.pdf
double cotangent( const MeshLaplacian::Mesh::Point& a, const MeshLaplacian::Mesh::Point& b, const MeshLaplacian::Mesh::Point& c )
{
	MeshLaplacian::Mesh::Point 
	  ba = a - b,
	  bc = c - b;

	// Divide scalar by cross product
	return (bc | ba) / (bc % ba).length();
}

//-----------------------------------------------------------------------------
void MeshLaplacian::compute( const Mesh& mesh )
{
	solveDense( mesh );
	//solveSparse( mesh ); // Not supported yet!
}

//-----------------------------------------------------------------------------
void MeshLaplacian::solveSparse( const Mesh& mesh )
{
	// 1.Setup cotan Laplacian
	//    L = D^(-1)W
	// with 
	//    D = diag(s_1,...,s_n) / 3   where s_i > 0 area of one ring at vertex i	
	//    w_ij = (cot(\alpha_ij) + cot(\beta_ij)) / 2 
	//    w_ii = -sum_{k\neq i}w_ik
	
	int n = (int)mesh.n_vertices();	

	SparseMatrix W(n,n);
	Vector D(n);
	
	TripletArray weights;
	buildSystem( mesh, weights, D );
	
	W.setFromTriplets( weights.begin(), weights.end() );

	// 2.Solve eigenvalue problem
	//

#if 1
	// Compute a symmetric Laplacian and solve ordinary problem LE = ES

	// Use a sparse SPD solver (for an ordinary eigenvalue problem)
	Eigen::ConjugateGradient<SparseMatrix, Eigen::Upper> cg;

	// Compute L = D^(-1).W by multiplying row i of W with d_i
	for( TripletArray::const_iterator it=weights.begin(); it!=weights.end(); ++it )
		W.coeffRef( it->row(), it->col() ) *= D( it->row() );

	// Symmetrize L via (L'+L)/2, as suggested by [Lévy 2006]
	//SparseMatrix L = .5 * (W.transpose() + W);

	// TODO: Eigen does currently not support sparse eigenvalue problems
	// (https://forum.kde.org/viewtopic.php?f=74&t=84238&sid=3f4ff0c78fee1714266744b3567c3e89&start=15)
	// therfore we have to integrate some other libary, e.g.:
	// - Armadillo http://arma.sourceforge.net/, wraps LAPACK,
	//   provides eigs_sym http://arma.sourceforge.net/docs.html#eigs_sym.
	//   Windows version currently requires Visual Studio >= VS2012.
	// - SuiteSparse for which Eigen3 already provides wrappers
	//   http://eigen.tuxfamily.org/dox-devel/group__TopicSparseSystems.html
	//   http://www.cise.ufl.edu/research/sparse/SuiteSparse/
	//   or more specific only the QR package
	//   http://www.cise.ufl.edu/research/sparse/SPQR/
	//   see also https://github.com/PetterS/SuiteSparse for VS support.
	// - FEAST http://www.ecs.umass.edu/~polizzi/feast
#endif


	throw "MeshLaplacian::solveSparse() not implemented yet!\n";
}

//-----------------------------------------------------------------------------
void MeshLaplacian::solveDense( const Mesh& mesh )
{		
	// 1.Setup cotan Laplacian
	//    L = D^(-1)W
	// with 
	//    D = diag(s_1,...,s_n) / 3   where s_i > 0 area of one ring at vertex i	
	//    w_ij = (cot(\alpha_ij) + cot(\beta_ij)) / 2 
	//    w_ii = -sum_{k\neq i}w_ik
	// 	
	size_t n = mesh.n_vertices();
	
	TripletArray weights;
	Vector D;
	buildSystem( mesh, weights, D );
	
	// Build dense matrix from triplets
	Matrix W = Matrix::Zero(n,n);
	for( TripletArray::iterator it=weights.begin(); it!=weights.end(); ++it )
		W( it->row(), it->col() ) = it->value();
	
	// 2.Solve generalized eigenvalue problem
	//    WE = DES  with eigenvectors E and ~values in diagonal matrix S
	//
	Eigen::GeneralizedSelfAdjointEigenSolver<Matrix> solver;
	solver.compute( W, D.asDiagonal(), Eigen::ComputeEigenvectors | Eigen::Ax_lBx );
	
	// Store decomposition
	m_eigenvectors = solver.eigenvectors();
	m_eigenvalues  = solver.eigenvalues();
}

//-----------------------------------------------------------------------------
void MeshLaplacian::buildSystem( const Mesh& mesh, TripletArray& W, Vector& D )
{
	// Setup cotan Laplacian
	//    L = D^(-1)W
	// with 
	//    D = diag(s_1,...,s_n) / 3   where s_i > 0 area of one ring at vertex i	
	//    w_ij = (cot(\alpha_ij) + cot(\beta_ij)) / 2 
	//    w_ii = -sum_{k\neq i}w_ik
	
	// Reference implementation using dense matrices	
	size_t n = mesh.n_vertices();
	W.reserve( 6*n ); // Est. number of non-zero entries in W
	D = Vector::Zero( n );
	
	// Temporary vector to accumulate weights per row (i!=j)
	Vector w_ii = Vector::Zero( n );

	// Compute w_ij
	// Iterate over all edges 
	Mesh::EdgeIter e_it, e_end( mesh.edges_end() );
	for( e_it = mesh.edges_begin(); e_it!=e_end; ++e_it ) 
	{	
#if 1
		// Iterate over half edges
		for( int k=0; k < 2; k++ )
		{
			// On the boundary only one HE is valid and omitting the 
			// corresponding cot() term is the correct way to handle von
			// Neumann boundary condition. E.g. see 
			//   Zhang et al. "Spectral Mesh Processing", CGF, 2010.
			Mesh::HalfedgeHandle he = mesh.halfedge_handle( *e_it, k );
			if( he.is_valid() )
			{
				// Assume triangle mesh
		
				// Get vertex handles (whose index will be used as well for the matrix!)
				Mesh::VertexHandle
				  hi = mesh.from_vertex_handle( he ),
				  hj = mesh.to_vertex_handle( he ),
				  ha = mesh.to_vertex_handle( mesh.next_halfedge_handle( he ) );
		
				// Cotangent weight
				double cot = cotangent( 
					mesh.point(hj), mesh.point(ha), mesh.point(hi) );

				// Record into weight matrix (duplicate index entries will
				// be summed resulting in the end in 
				//     w_ij = (cot(alpha) + cot(beta)) / 2.
				double w_ij = .5 * cot;
				W.push_back( Triplet( hi.idx(), hj.idx(), w_ij ) );

				// Compute row sum for diagonal entries
				w_ii( hi.idx() ) += w_ij;
			}
		}
#else
		// FIXME: Treat boundary edges correctly!
		Mesh::HalfedgeHandle 
		  he_a = mesh.halfedge_handle( *e_it, 0 ),
		  he_b = mesh.opposite_halfedge_handle( he_a ); // same as: mesh.halfedge_handle( *h_it, 1 );

		// Assume triangle mesh
		
		// Get vertex handles (whose index will be used as well for the matrix!)
		Mesh::VertexHandle
		  hi = mesh.from_vertex_handle( he_a ),
		  hj = mesh.to_vertex_handle( he_a ),
		  ha = mesh.to_vertex_handle( mesh.next_halfedge_handle( he_a ) ),
		  hb = mesh.to_vertex_handle( mesh.next_halfedge_handle( he_b ) );
		
		Mesh::Point 
		  vi = mesh.point( hi ),
		  vj = mesh.point( hj ),
		  va = mesh.point( ha ),
		  vb = mesh.point( hb );
		
		// Cotangent weights
		double 
		  cota = cotangent( vj, va, vi ),
		  cotb = cotangent( vi, vb, vj );
		
		// Record into weight matrix
		double w_ij = 0.5*( cota + cotb );
		W.push_back( Triplet( hi.idx(), hj.idx(), w_ij ) );
		W.push_back( Triplet( hj.idx(), hi.idx(), w_ij ) ); // Symmetry
		w_ii( hi.idx() ) += w_ij;
		
		/* Dense equivalent:
		   W( vi.idx(), vj.idx() ) = 0.5*( cota + cotb );
		   W( vj.idx(), vi.idx() ) = W( vi.idx(), vj.idx() ); // Symmetry
		*/
#endif
	}
	
	// Compute w_ii, i.e. negative row sum so far (very inefficient in dense case!)
	for( int i=0; i < W.size(); i++ )
		W.push_back( Triplet( i,i, w_ii(i) ) );
	
	/* Dense equivalent
	for( int row=0; row < W.rows(); row++ )
	{
		for( int col=0; col < row; col++ )
			W(row,row) += W(row,col)
		for( int col=row+1; col < W.cols(); col++ )
			W(row,row) += W(row,col)
	}
	*/
	
	// Compute 1-ring areas
	// Iterate over all vertices
	Mesh::ConstVertexIter v_it, v_end( mesh.vertices_end() );
	for( v_it = mesh.vertices_begin(); v_it != v_end; ++ v_it )
	{
		// Coordinate of current vertex
		Mesh::Point v0 = mesh.point( *v_it );
		
		// Store 1-ring legs
		std::vector< Mesh::Point > legs;

		// Note that the const_cast wouldn't be neccessary if OpenMesh would
		// provide a suitable overload for vv_...() below!
		Mesh& tempUnconstMesh = const_cast<Mesh&>( mesh );

		// Iterate over 1-ring
		Mesh::ConstVertexVertexIter vv_it, vv_end( tempUnconstMesh.vv_end( *v_it ) );
		for( vv_it=tempUnconstMesh.vv_begin(*v_it); vv_it!=vv_end; ++vv_it )
			legs.push_back( mesh.point( *vv_it ) - v0 );
		
		// Sum up triangle areas
		double area = 0.0;
		size_t m = legs.size()-1;
		for( int i=0; i < legs.size(); i++ )
		{
			// Cross product is denoted as '%'
			area += (legs[i] % legs[i==m?0:(i+1)]).length();			
		}
		
		// Record into D matrix
		D( v_it->idx() ) = area / 3.0;
	}
}
