#include "meshtools.h"
#include <cmath>
#include <string>
#include <iomanip>

namespace meshtools {

//-----------------------------------------------------------------------------
//  Mesh functions
//-----------------------------------------------------------------------------

OpenMesh::IO::Options meshIOdefaultOptions( 
	OpenMesh::IO::Options::VertexColor | 
	OpenMesh::IO::Options::VertexNormal
	);

bool loadMesh( Mesh& mesh, const char* filename )
{	
	OpenMesh::IO::Options opts( meshIOdefaultOptions );

	mesh.request_vertex_colors();
	mesh.request_vertex_normals();

	std::string file_in( filename );
	try
	{
		if( !OpenMesh::IO::read_mesh( mesh, file_in, meshIOdefaultOptions ) )
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

	if( opts.check( OpenMesh::IO::Options::FaceNormal ) )
		std::cout << "meshtools::loadMesh() : Found face normals" << std::endl;
	if( opts.check( OpenMesh::IO::Options::VertexNormal ) )
		std::cout << "meshtools::loadMesh() : Found vertex normals" << std::endl;
	if( opts.check( OpenMesh::IO::Options::VertexColor ) )
		std::cout << "meshtools::loadMesh() : Found vertex color" << std::endl;

	return true;
}

bool saveMesh( const Mesh& mesh, const char* filename )
{
	std::string file_out( filename );
	try
	{
		if( !OpenMesh::IO::write_mesh( mesh, file_out, meshIOdefaultOptions ) )
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
	std::cout << "Mesh has vertex normals : " << 
		(mesh.has_vertex_normals() ? "yes" : "no") << std::endl;
}

void updateMeshVertexNormals( Mesh* m )
{
	// Sometimes normals are not stored and have to be computed explicitly
	if( !m->has_vertex_normals() )
	{
		m->request_vertex_normals();
		m->request_face_normals(); // Face normals required to update vertex normals
		m->update_normals();       // Compute vertex normals
		m->release_face_normals(); // Dispose the face normals, as we don't need them anymore
	}
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


#ifndef MESHTOOLS_WITHOUT_EIGEN_SUPPORT

//-----------------------------------------------------------------------------
//  Matrix functions
//-----------------------------------------------------------------------------

void convertMeshToMatrix( const Mesh& mesh, Eigen::Matrix3Xd& mat )
{
	mat.resize( 3, mesh.n_vertices() );

	Mesh::ConstVertexIter v_it = mesh.vertices_begin();
	unsigned col=0;
	for( ; v_it != mesh.vertices_end(); ++v_it, col++ )
	{
		const Mesh::Point& p = mesh.point( v_it );
		mat(0,col) = p[0];
		mat(1,col) = p[1];
		mat(2,col) = p[2];
	}

	if( col != mesh.n_vertices() )
		std::cerr << "Warning: Mismatch in number of vertices!" << std::endl;
}

void convertMeshNormalsToMatrix( const Mesh& mesh, Eigen::Matrix3Xd& mat )
{
	if( !mesh.has_vertex_normals() )
	{
		std::cerr << "Mesh requires vertex normals!" << std::endl;
		return;
	}

	mat.resize( 3, mesh.n_vertices() );

	Mesh::ConstVertexIter v_it = mesh.vertices_begin();
	unsigned col=0;
	for( ; v_it != mesh.vertices_end(); ++v_it, col++ )
	{	
		const Mesh::Normal& n = mesh.normal( v_it );
		mat(0,col) = n[0];
		mat(1,col) = n[1];
		mat(2,col) = n[2];
	}

	if( col != mesh.n_vertices() )
		std::cerr << "Warning: Mismatch in number of vertices!" << std::endl;
}

void replaceVerticesFromMatrix( Mesh& mesh, const Eigen::Matrix3Xd& mat )
{
	if( mesh.n_vertices() != mat.cols() )
	{
		std::cerr <<"Error: Mismatching number of vertices and matrix columns!"	<< std::endl
			<< "Mesh has " << mesh.n_vertices() << " vertices but matrix has " << mat.cols() << " columns" << std::endl;
		return;
	}

	Mesh::VertexIter v_it = mesh.vertices_begin();
	unsigned col=0;
	for( ; v_it != mesh.vertices_end(); ++v_it, col++ )
	{
		mesh.point(v_it)[0] = mat(0,col);
		mesh.point(v_it)[1] = mat(1,col);
		mesh.point(v_it)[2] = mat(2,col);
	}
}
	
void writeMatrix( const Eigen::Matrix3Xd& mat, std::ostream& os )
{
	os.precision( 7 );
	os << std::scientific;
	for( int i=0; i < mat.cols(); i++ )
	{
		os << std::setw(15) << mat(0,i) << " "
		   << std::setw(15) << mat(1,i) << " "
		   << std::setw(15) << mat(2,i) << std::endl;
	}
}

void readMatrix( Eigen::Matrix3Xd& mat, std::istream& is )
{
	// Read all lines into buffer
	std::stringstream buf;
	std::string s;
	int lines=0;
	while( std::getline( is, s ) )
	{
		buf << s;
		lines++;
	}

	// Parse matrix entries
	mat.resize( 3, lines );
	int col=0;
	while( buf.good() && col<lines )
	{
		double x,y,z;
		buf >> x;
		buf >> y;
		buf >> z;
		mat(0,col) = x;
		mat(1,col) = y;
		mat(2,col) = z;
		col++;
	}

	if( col != lines )
		std::cerr << "readMatrix() mismatch in matrix format!" << std::endl;
}

#endif // MESHTOOLS_WITHOUT_EIGEN_SUPPORT


} // namespace meshspace
