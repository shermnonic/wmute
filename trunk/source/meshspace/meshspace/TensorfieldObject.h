#ifndef SCENE_TENSORFIELDOBJECT_H
#define SCENE_TENSORFIELDOBJECT_H

#include <meshtools.h>
#include <ShapePCA.h>  // for PCAModel
#include "MeshObject.h"

#include <string>

namespace scene {
	
//-----------------------------------------------------------------------------
// 	TensorfieldObject
//-----------------------------------------------------------------------------
/// Superquadric tensor field visualization.
class TensorfieldObject : public MeshObject
{
	enum DirtyFlags {
		NoChange         = 0,
		ResolutionChange = 1,
		GeometryChange   = 2,
		ColorChange      = 4,
		CompleteChange   = 7
	};

public:
	enum TensorMode {
		AnatomicCovariance,
		InterPointCovariance,
		InterPointCovarianceUnweighted
	};

	enum ColorMode {
		ColorByFractionalAnisotropy,
		ColorByCluster
	};

	TensorfieldObject();

	// Internally calls deriveTensorsFromCovariance() and setGlyphPositions()
	void deriveTensorsFromPCAModel( const PCAModel& pca, int mode=AnatomicCovariance, double gamma=0.0, double scale=1.0 );

	void createTestScene();

	///@{ Glyph geometry
	int numGlyphs()        const { return (int)m_R.cols(); }
	int numGlyphVertices() const { return m_glyphRes*(m_glyphRes+1); }
	int numGlyphFaces()    const { return 2*m_glyphRes*m_glyphRes; }
	///@}

	///@{ Glyph properties. Changes will take effect on next call to updateTensorfield().
	void setGlyphScale( double scale ) { if( m_glyphScale != scale ) m_dirtyFlag |= GeometryChange; m_glyphScale = scale; }
	double getGlyphScale() const { return m_glyphScale; }

	void setGlyphSharpness( double gamma ) { if( m_glyphSharpness != gamma ) m_dirtyFlag |= GeometryChange; m_glyphSharpness = gamma; }
	double getGlyphSharpness() const { return m_glyphSharpness; }

	int getGlyphResolution() const { return m_glyphRes; }
	void setGlyphResolution( int res ) { if( m_glyphRes != res ) m_dirtyFlag |= ResolutionChange; m_glyphRes = res; }

	bool getGlyphSqrtEV() const { return m_glyphSqrtEV; }
	void setGlyphSqrtEV( bool b ) { if( m_glyphSqrtEV != b ) m_dirtyFlag |= GeometryChange; m_glyphSqrtEV = b; }

	void setColorMode( int mode ) { if( mode != m_colorMode ) m_dirtyFlag |= ColorChange; m_colorMode = mode; }
	int getColorMode() const { return m_colorMode; }
	///@}

	/// Performs required updates on tensor field geometry and coloring.
	void updateTensorfield();

	///@{ Tensorfield IO
	void saveTensorfield( std::string filename );
	bool loadTensorfield( std::string filename );
	void exportTensorfieldAsNrrd( std::string path, std::string basename, MeshBuffer* mb=NULL );
	///@}

	void setClusters( const std::vector<unsigned int>& indices, unsigned numClusters=-1 );
	const Eigen::MatrixXd& getTensorField() const { return m_tensorField; }
	const Eigen::Matrix3Xd& getPositions() const { return m_pos; }

protected:
	// Called by deriveTensorsFromPCAModel()
	void deriveTensorsFromCovariance( const Eigen::MatrixXd& S );	
	
	// Call before deriveTensorsFromCovariance
	// Internally called in deriveTensorsFromPCAModel
	void setGlyphPositions( meshtools::Mesh* mesh );
	void setGlyphPositions( Eigen::Matrix3Xd pos );

	/// Filter tensor field (and corresponding 3D sample points) and remove
	/// tensors with Frobenius norm smaller than the given threshold.
	/// Returns the number of filtered out tensors.
	unsigned filterTensorField( Eigen::MatrixXd& S, Eigen::Matrix3Xd& pts, double threshold=0.0001 );

protected:
	void updateFaces   ( int glyphId );
	void updateVertices( int glyphId );
	void updateColor   ( int glyphId );

	void add_vertex( int glyphId, int vhandle, const Eigen::Vector3d& v );
	void add_normal( int glyphId, int vhandle, const Eigen::Vector3d& n );
	void add_face( int glyphId, int fhandle, int v0, int v1, int v2 );

	Eigen::Vector3d get_vertex( int glyphId, int vhandle );

	void get_res( int& n, int&m );

	///@{ Raw buffer access, pass through from MeshBuffer
	std::vector<unsigned>& ibuf() { return meshBuffer().ibuffer(); }
	std::vector<float>& vbuf() { return meshBuffer().vbuffer(); }
	std::vector<float>& nbuf() { return meshBuffer().nbuffer(); }
	///@}

private:
	int m_dirtyFlag;           ///< See \a DirtyFlags
	int m_glyphRes;            ///< Resolution of glyph, i.e. sampling in phi/theta
	int m_colorMode;           ///< Color mode, see \a ColorMode
	double m_glyphSharpness;   ///< Sharpness of superquadric tensor glyph (gamma)
	double m_glyphScale;       ///< Glyph scale factor
	bool   m_glyphSqrtEV;      ///< Scale glyphs by sqrt of eigenvalues
	Eigen::MatrixXd m_R;       ///< Eigenvectors (3x3 V vectorized in 9x1 column)
	Eigen::Matrix3Xd m_Lambda; ///< Eigenvalues  (3x1 vectors in columns)
	Eigen::Matrix3Xd m_pos;    ///< Glyph centers

	Eigen::MatrixXd m_tensorField; ///< Input tensorfield

	std::vector<unsigned int> m_clusterIndices;
	unsigned m_numClusters;
};

} // namespace scene 

#endif // SCENE_TENSORFIELDOBJECT_H
