#ifndef SCENE_PCAOBJECT_H
#define SCENE_PCAOBJECT_H

#include <meshtools.h>
#include <ShapePCA.h>
#include "MeshObject.h"

//-----------------------------------------------------------------------------
// 	scene::PCAObject
//-----------------------------------------------------------------------------

namespace scene {

/// PCA shape model as a scene object
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

	/// Return number of principal components
	int numPCs() const { return (int)m_pca.PC.cols(); }

	/// Return current PC coefficients
	Eigen::VectorXd coeffs() const { return m_coeffs; }
	/// Provided for convenience, see \a coeffs()
	void getCoeffs( std::vector<double>& coeffs ) const;

	/// Access raw PCA model (read-only)
	const PCAModel& getPCAModel() const { return m_pca; }

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
	Eigen::VectorXd m_coeffs;
};

} // namespace scene 

#endif // SCENE_PCAOBJECT_H
