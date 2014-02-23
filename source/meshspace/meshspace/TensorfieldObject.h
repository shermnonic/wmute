#ifndef SCENE_TENSORFIELDOBJECT_H
#define SCENE_TENSORFIELDOBJECT_H

#include <meshtools.h>
#include <ShapePCA.h>  // for PCAModel
#include "MeshObject.h"

namespace scene {
	
//-----------------------------------------------------------------------------
// 	TensorfieldObject
//-----------------------------------------------------------------------------
class TensorfieldObject : public MeshObject
{
public:
	TensorfieldObject();

	void deriveTensorsFromPCAModel( const PCAModel& pca );

	int numGlyphs()        const { return (int)m_R.cols(); }
	int numGlyphVertices() const { return m_glyphRes*(m_glyphRes+1); }
	int numGlyphFaces()    const { return 2*numGlyphVertices(); }

protected:
	void deriveTensorsFromCovariance( const Eigen::MatrixXd& S );	
	
	// Call before deriveTensorsFromCovariance
	// Internally called in deriveTensorsFromPCAModel
	void setGlyphPositions( meshtools::Mesh* mesh );
	void setGlyphPositions( Eigen::Matrix3Xd pos );

protected:
	void createGeometry( int res=16 );
	void updateFaces   ( int glyphId );
	void updateVertices( int glyphId );
	void updateColor   ( int glyphId );

	void add_vertex_and_normal( int glyphId, int vhandle, 
	                const Eigen::Vector3d& v, const Eigen::Vector3d& n );
	void add_face( int glyphId, int fhandle, int v0, int v1, int v2 );

	///@{ Raw buffer access, pass through from MeshBuffer
	std::vector<unsigned>& ibuf() { return meshBuffer().ibuffer(); }
	std::vector<float>& vbuf() { return meshBuffer().vbuffer(); }
	std::vector<float>& nbuf() { return meshBuffer().nbuffer(); }
	///@}

private:
	int m_glyphRes;
	double m_glyphSharpness;   ///< Sharpness of superquadric tensor glyph (gamma)
	Eigen::MatrixXd m_R;       ///< Eigenvectors (3x3 V vectorized in 9x1 column)
	Eigen::Matrix3Xd m_Lambda; ///< Eigenvalues  (3x1 vectors in columns)
	Eigen::Matrix3Xd m_pos;    ///< Glyph centers
};

} // namespace scene 

#endif // SCENE_TENSORFIELDOBJECT_H
