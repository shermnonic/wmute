#ifndef MESHTOOLS_H
#define MESHTOOLS_H

#include <iostream>
#include <OpenMesh/Core/IO/MeshIO.hh> // must be included before any mesh type
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <Eigen/Dense>

/** @addtogroup meshtools_grp meshtools library
  * @{ */

namespace meshtools {

///@name Mesh definition
///@{ 

/// Definition of mesh attributes
struct MeshTraits : public OpenMesh::DefaultTraits
{
	// Status is required to allow deletion of geometry elements
	// (see http://openmesh.org/Documentation/OpenMesh-2.0-Documentation/tutorial_07b.html)
	VertexAttributes(OpenMesh::Attributes::Status);
	FaceAttributes(OpenMesh::Attributes::Status);
	EdgeAttributes(OpenMesh::Attributes::Status);
};

/// Mesh datatype, \sa \a MeshTraits
typedef OpenMesh::TriMesh_ArrayKernelT<MeshTraits> Mesh;

///@}


///@name Mesh functions
///@{ 

/// Read a mesh from disk via OpenMesh which supports formats like .stl, .obj, .ply, .off
bool loadMesh( Mesh& mesh, const char* filename );
/// Write a mesh disk disk via OpenMesh which supports formats like .stl, .obj, .ply, .off
bool saveMesh( const Mesh& mesh, const char* filename );
/// Print some useful information about a given mesh
void printMeshInfo( const Mesh& m, std::ostream& os=std::cout );

/// (Re)compute per vertex normals
void updateMeshVertexNormals( Mesh* m );

/// Compute the volume of a watertight 3D triangle mesh
double computeMeshVolume( Mesh* m );

///@}


///@name Matrix functions
///@{ 

/// Write all vertices from a mesh into columns of a 3xd matrix
/// \sa replaceVerticesFromMatrix
/// \sa meshICP
void convertMeshToMatrix( const Mesh& mesh, Eigen::Matrix3Xd& mat );

/// Write all vertex normals from a mesh into columns of a 3xd matrix
/// \sa convertMeshToMatrix
/// \sa meshICP
void convertMeshNormalsToMatrix( const Mesh& mesh, Eigen::Matrix3Xd& mat );

/// Replace vertices in a mesh with those given in columns of a 3xd matrix
/// \sa convertMeshToMatrix
/// \sa meshICP
void replaceVerticesFromMatrix( Mesh& mesh, const Eigen::Matrix3Xd& mat );

/// Write matrix to a text stream.
void writeMatrix( const Eigen::Matrix3Xd& mat, std::ostream& os );

/// Read matrix from a text stream.
void readMatrix( Eigen::Matrix3Xd& mat, std::istream& is );


} // namespace meshtools

/** @} */ // end group meshtools

#endif // MESHTOOLS_H
