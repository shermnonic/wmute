#include "meshtools.h"
#include <cmath>
#include <string>

namespace meshtools {

bool loadMesh( Mesh& mesh, const char* filename )
{	
	std::string file_in( filename );
	try
	{
		if( !OpenMesh::IO::read_mesh( mesh, file_in ) )
		{
			std::cerr << "Error: Couldn't load mesh from '" << file_in << "'!" 
				<< std::endl;
			return false;
		}
	}
	catch( std::exception& x )
	{
		std::cerr << "An exception occured while trying to read a mesh from '"
		          << file_in << "'." << std::endl
		          << "Exception: " << x.what() << std::endl;
		return false;
	}
	return true;
}

bool saveMesh( const Mesh& mesh, const char* filename )
{
	std::string file_out( filename );
	try
	{
		if( !OpenMesh::IO::write_mesh( mesh, file_out ) )
		{
			std::cerr << "Error: Couldn't save mesh to '" << file_out << "'!" 
				<< std::endl;
			return false;
		}
	}
	catch( std::exception& x )
	{
		std::cerr << "An exception occured while trying to write a mesh to '"
		          << file_out << "'." << std::endl
		          << "Exception: " << x.what() << std::endl;
		return false;
	}
	return true;
}
	
void printMeshInfo( const Mesh& mesh, std::ostream& os )
{
	std::cout << "# Vertices: " << mesh.n_vertices() << std::endl;
	std::cout << "# Edges   : " << mesh.n_edges   () << std::endl;
	std::cout << "# Faces   : " << mesh.n_faces   () << std::endl;
}

void updateMeshVertexNormals( Mesh* m )
{
	// Sometimes normals are not stored and have to be computed explicitly
	m->request_vertex_normals();		
	m->request_face_normals(); // Face normals required to update vertex normals
	m->update_normals();       // Compute vertex normals
	m->release_face_normals(); // Dispose the face normals, as we don't need them anymore
}

double computeMeshVolume( Mesh* m )
{
	// Assumes a strict triangle mesh with consistent orientation.
	
	double vol = 0.;
	
	Mesh::FaceIter f_it = m->faces_begin();
	for( ; f_it != m->faces_end(); ++f_it )
	{
		// Collect vertex coordinates of a triangle
		struct Vertex { double x,y,z; };
		Vertex v[3];
		
		Mesh::FaceVertexIter fv_it = m->fv_iter( f_it );
		for( int i=0; fv_it && i < 3; ++fv_it, i++ )
		{
			const Mesh::Point& p = m->point( fv_it );
			v[i].x = p[0];
			v[i].y = p[1];
			v[i].z = p[2];			
		}
		
		// Signed volume of tetrahedron formed with origin
		vol += (1./6.)*(
		  -v[2].x*v[1].y*v[0].z + v[1].x*v[2].y*v[0].z + v[2].x*v[0].y*v[1].z
		  -v[0].x*v[2].y*v[1].z - v[1].x*v[0].y*v[2].z + v[0].x*v[1].y*v[2].z );
	}
	
	return fabs(vol);
}
	
} // namespace meshspace

