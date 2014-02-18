#ifndef SCENE_PCAOBJECT_H
#define SCENE_PCAOBJECT_H

#include <meshtools.h>
#include "MeshObject.h"

//-----------------------------------------------------------------------------
// 	PCA utilities
//-----------------------------------------------------------------------------

/// PCA model, result data structure for \a computePCA() functions.
struct PCAModel
{
	Eigen::MatrixXd PC; /// Eigenvectors of sample covariance matrix
	Eigen::VectorXd ev; /// Eigenvalues of sample covariance matrix
	Eigen::VectorXd mu;	/// Sample mean (column vector)
};	

//-----------------------------------------------------------------------------
// 	scene::PCAObject
//-----------------------------------------------------------------------------

namespace scene {

class PCAObject : public MeshObject
{
public:
	PCAObject()
		: MeshObject(),
		  m_curPC(0)
	{}

	/// Compute PCA model for given mesh sequence
	void derivePCAModelFrom( const MeshObject& mo );

	///@{ Reimplemented from MeshObject, show i-th eigenmode plus mean shape
	void setFrame( int i );
	unsigned numFrames() const { return (int)m_pca.PC.cols(); }
	int curFrame() const { return m_curPC; }
	///@}

	/// Synthesize a shape from PCA model with given PC coefficients
	void synthesize( const std::vector<double>& coefficients );

	int numPCs() const { return (int)m_pca.PC.cols(); }

protected:
	///@{ Protect all modify functions of \a MeshObject
	void setMesh( boost::shared_ptr<meshtools::Mesh> mesh ) { MeshObject::setMesh(mesh); }
	void setMesh( meshtools::Mesh* mesh )                   { MeshObject::setMesh(mesh); }
	void setMeshBuffer( const MeshBuffer& mb )              { MeshObject::setMeshBuffer(mb); }
	bool addFrame( meshtools::Mesh* mesh )                  { return MeshObject::addFrame(mesh); }
	MeshBuffer& meshBuffer() { return MeshObject::meshBuffer(); }
	///@}
	
private:
	PCAModel        m_pca;
	int             m_curPC;
	meshtools::Mesh m_mshape;
};

} // namespace scene 

#endif // SCENE_PCAOBJECT_H
