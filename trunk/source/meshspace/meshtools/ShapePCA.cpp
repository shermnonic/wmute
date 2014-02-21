#include "ShapePCA.h"
#include <PCA.h>

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

void computePCA( /*const*/ MeshBuffer& samples, MeshBuffer& pcmb, PCAModel& model, meshtools::Mesh& mshape )
{
	// Map vertex buffer to data matrix
	Eigen::Map<Eigen::MatrixXf> Xf( &(samples.vbuffer()[0]), samples.numVertices()*3, samples.numFrames() );
	
	// Copy float to double matrix (since we internally mostly use double matrices)
	Eigen::MatrixXd X = Xf.cast<double>();
	
	// Compute PCA
	computePCA( X, model.PC, model.ev, model.mu );

	// Also store zero-mean data matrix for further analyis
	model.X = X;
	centerMatrix( model.X );

	// Create output meshbuffer
	pcmb.clear();	
	meshtools::Mesh* mesh = samples.createMesh(); // reference connectivity
	
	// Mean shape mesh
	meshtools::replaceVerticesFromMatrix( *mesh, reshape(model.mu) );
	mshape = *mesh; // copy mesh

	pcmb.addFrame( mesh );


	/* Add principal modes as single frames (OBSOLETE)
	for( int i=0; i < model.PC.cols(); ++i )
	{	
		meshtools::replaceVerticesFromMatrix( *mesh, reshape( model.PC.col(i) ) );
		pcmb.addFrame( mesh ); // addFrame() implicitly recomputes normals
	}*/	
	
	// Free temporary memory
	delete mesh;
}
