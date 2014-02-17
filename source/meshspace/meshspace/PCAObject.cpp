#include "PCAObject.h"
#include <PCA.h>
#include <Eigen/Dense>

//-----------------------------------------------------------------------------
// 	PCA utilities
//-----------------------------------------------------------------------------

/// De-vectorize 3n x 1 to 3 x n
Eigen::Matrix3Xd reshape( const Eigen::VectorXd& v )
{
	Eigen::Matrix3Xd mat;
	mat.resize( 3, v.size()/3 );
	for( int j=0; j < v.size()/3; ++j ) 
	{
		// FIXME: Use some reshape/map/stride Eigen functionality here!
		mat(0,j) = v(3*j);
		mat(1,j) = v(3*j+1);
		mat(2,j) = v(3*j+2);
	}
	return mat;
}

/// Compute PCA of MeshBuffer vertex data
/// \param[in]  samples  Input MeshBuffer
/// \param[out] pcmb     Output MeshBuffer with PC modes as frames
/// \param[out] model    \a PCAModel with eigenvectors, ~values and mean
/// \param[out] mshape   Mean shape as \a meshtools::Mesh
void computePCA( /*const*/ MeshBuffer& samples, MeshBuffer& pcmb, PCAModel& model, meshtools::Mesh& mshape )
{
	// Map vertex buffer to data matrix
	Eigen::Map<Eigen::MatrixXf> Xf( &(samples.vbuffer()[0]), samples.numVertices()*3, samples.numFrames() );
	
	// Copy float to double matrix (since we internally mostly use double matrices)
	Eigen::MatrixXd X = Xf.cast<double>();
	
	// Compute PCA
	computePCA( X, model.PC, model.ev, model.mu );
	
	// Create output meshbuffer with PC meshes
	pcmb.clear();	
	meshtools::Mesh* mesh = samples.createMesh(); // reference connectivity
	for( int i=0; i < model.PC.cols(); ++i )
	{	
		meshtools::replaceVerticesFromMatrix( *mesh, reshape( model.PC.col(i) ) );
		pcmb.addFrame( mesh ); // addFrame() implicitly recomputes normals
	}
	
	// Mean shape mesh
	meshtools::replaceVerticesFromMatrix( *mesh, reshape(model.mu) );
	mshape = *mesh; // copy mesh
	
	// Free temporary memory
	delete mesh;
}

//-----------------------------------------------------------------------------
// 	scene::PCAObject
//-----------------------------------------------------------------------------

namespace scene {
	
void PCAObject::derivePCAModelFrom( const MeshObject& mo )
{	
	computePCA( const_cast<MeshObject&>(mo).meshBuffer(), meshBuffer(), m_pca, m_mshape );
}

} // namespace scene 
