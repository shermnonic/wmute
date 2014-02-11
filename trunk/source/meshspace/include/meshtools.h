#ifndef MESHTOOLS_H
#define MESHTOOLS_H

#include <iostream>
#include <OpenMesh/Core/IO/MeshIO.hh> // must be included before any mesh type
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <Eigen/Dense>

namespace meshtools {

struct MeshTraits : public OpenMesh::DefaultTraits
{
	// Status is required to allow deletion of geometry elements
	// (see http://openmesh.org/Documentation/OpenMesh-2.0-Documentation/tutorial_07b.html)
	VertexAttributes(OpenMesh::Attributes::Status);
	FaceAttributes(OpenMesh::Attributes::Status);
	EdgeAttributes(OpenMesh::Attributes::Status);
};

typedef OpenMesh::TriMesh_ArrayKernelT<MeshTraits> Mesh;

bool loadMesh( Mesh& mesh, const char* filename );	
bool saveMesh( const Mesh& mesh, const char* filename );
void printMeshInfo( const Mesh& m, std::ostream& os=std::cout );
void updateMeshVertexNormals( Mesh* m );

/// Compute the volume of a watertight 3D triangle mesh
double computeMeshVolume( Mesh* m );

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

} // namespace meshtools

#endif // MESHTOOLS_H
