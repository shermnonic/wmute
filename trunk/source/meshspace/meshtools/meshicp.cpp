// meshicp - Align two meshes using (sparse) iterative closest point algorithm
// Max Hermann, Jan. 2014 (hermann@cs.uni-bonn.de)
#include <iostream>
#include <ICP.h>        // "Sparse Iterative Closest Point" by Sofien Bouaziz
#include "meshtools.h"  // some custom OpenMesh functions
#include "Intersect.h"

using namespace meshtools;

const char g_usage[] =
"meshicp - Align two meshes using (sparse) iterative closest point algorithm.\n"
"Max Hermann 2014 (hermann@cs.uni-bonn.de)\n"
"\n"
"Usage: meshicp <source> <target> <aligned_source_output> [<projected_target_to_source>]\n";

void callback( int itr, double error )
{
	printf("Iteration %4d: %7.5f\n",itr,error);
}

void makeMask( /*const*/ Eigen::Matrix3Xd& Z, std::vector<bool>& mask )
{
	double threshold = 0.001;
	mask.resize( Z.cols() );
	for( int i=0; i < Z.cols(); i++ )
	{
		mask[i] = Z.col(i).norm() > threshold;
	}
}

void makeMask( /*const*/ Eigen::VectorXd& Z, std::vector<bool>& mask )
{
	double threshold = 0.001;	
	mask.resize( Z.size() );
	for( int i=0; i < Z.size(); i++ )
	{
		mask[i] = Z(i) > threshold;
	}
}

/// Align source against target mesh using sparse iterative closest point
void meshICP( Mesh& source_mesh, const Mesh& target_mesh, SICP::Parameters parm, std::vector<bool>& mask )
{
	Eigen::Matrix3Xd source, target;
	convertMeshToMatrix( source_mesh, source );
	convertMeshToMatrix( target_mesh, target );

	if( target_mesh.has_vertex_normals() )
	{
		// point-to-plane		

		Eigen::VectorXd Z;

		Eigen::Matrix3Xd normals;
		convertMeshNormalsToMatrix( target_mesh, normals );

		std::cout << "Performing sparse ICP (point to plane)..." << std::endl;
		SICP::point_to_plane( source, target, normals, parm, &Z );

		makeMask( Z, mask );
	}
	else
	{
		// point-to-point
		Eigen::Matrix3Xd Z;

		std::cout << "Performing sparse ICP (point to point)..." << std::endl;
		SICP::point_to_point( source, target, parm, &Z );

		makeMask( Z, mask );
	}

	// Overwrite source vertices with ICP aligned ones
	replaceVerticesFromMatrix( source_mesh, source );
}

void makeCorrespondingMeshes( Mesh& source_mesh, Mesh& target_mesh, std::vector<bool> mask )
{
	typedef OpenMesh::ArrayKernel::VertexHandle VertexHandle;

	std::cout << "Creating correspondence meshes..." << std::endl;

	// Compute closest points source to target
	Eigen::Matrix3Xd source, target;
	convertMeshToMatrix( source_mesh, source );
	convertMeshToMatrix( target_mesh, target );

	// For each vertex i in source, idx[i] is the corresponding target vertex.
	std::vector<int> idx;
	SICP::closest_points( source, target, idx );

	assert( mask.size() == idx.size() );

	// Injective index mapping between source and target
	std::map<int,int> s2t; // Source to target indices
	std::map<int,int> t2s; // Target to source indices

	// Filter mask
	for( int i=0; i < idx.size(); i++ )
		if( mask[i] )
		{
			// Consider only first match
			if( s2t.find(i) == s2t.end() )
			{
				s2t[i] = idx[i];
				t2s[idx[i]] = s2t[i];
			}
		}
	std::cout << "Found " << s2t.size() << " correpondences" << std::endl;

	// Remove unmatched points from target
	int deleted = 0;
	Mesh::VertexIter v_it = target_mesh.vertices_begin();
	for( ; v_it != target_mesh.vertices_end(); ++v_it )
	{
		if( t2s.find( v_it.handle().idx() ) != t2s.end() )
		{
			// If no correpondence in source is found, mark vertex for deletion
			target_mesh.delete_vertex( *v_it, true ); // Delete isolated vertices
			deleted++;
		}
	}
	std::cout << "Deleted " << deleted << " vertices in target mesh" << std::endl;

	// Copy modified target mesh
	Mesh tmp_mesh = target_mesh;
	
	// Replace corresponding target with source vertices
	std::map<int,int>::const_iterator it = t2s.begin();
	for( ; it != t2s.end(); ++it )
	{
		tmp_mesh.point( VertexHandle( it->first ) )
			= source_mesh.point( VertexHandle( it->second ) );
	}

	// Delete vertices
	tmp_mesh   .garbage_collection();
	target_mesh.garbage_collection();

	source_mesh = tmp_mesh;
}

void registerMeshes( Mesh& source_mesh, Mesh& target_mesh, Mesh& aligned_source_mesh, SICP::Parameters parm )
{
	std::vector<bool> mask;
	
	meshICP( source_mesh, target_mesh, parm, mask );
	aligned_source_mesh = source_mesh;

	makeCorrespondingMeshes( source_mesh, target_mesh, mask );
}

void alignMeshes( Mesh& source_mesh, const Mesh& target_mesh, SICP::Parameters parm )
{
	std::vector<bool> mask;
	meshICP( source_mesh, target_mesh, parm, mask );
}

bool computeIntersection( const Mesh::Point& p, const Mesh::Normal& n,
	const Mesh::Point& v0, const Mesh::Point& v1, const Mesh::Point& v2,
	Mesh::Point& intersection, double& distance )
{
	using namespace Intersect;
	Ray r( Vec3(p[0],p[1],p[2]), Vec3(n[0],n[1],n[2]) );
	Triangle t;
	t.v0 = Vec3(v0[0],v0[1],v0[2]);
	t.v1 = Vec3(v1[0],v1[1],v1[2]);
	t.v2 = Vec3(v2[0],v2[1],v2[2]);

	RayTriangleIntersection rti = intersect( r, t );	
	Vec3 q = t.barycentric( rti.bc );
	intersection[0] = q.x;
	intersection[1] = q.y;
	intersection[2] = q.z;
	distance = rti.t;
	return rti.bc.is_inside();
}

void projectMesh( Mesh& mesh, const Mesh& surface )
{
	struct Intersection
	{
		Mesh::Point p;
		double dist;

		Intersection() {}
		Intersection( Mesh::Point p_, double dist_ ): p(p_), dist(dist_) {};

		// Sort according to absolute distance
		bool operator < ( const Intersection& other ) const
		{
			return fabs(dist) < fabs(other.dist);
		}
	};

	// Resulting mesh projected onto surface
	Mesh projected_mesh = mesh;

	// Vertex normals are required
	if( !mesh.has_vertex_normals() )
		meshtools::updateMeshVertexNormals( &mesh );

	// For each mesh vertex find intersection with surface in normal direction
	int numProjPts = 0;
	Mesh::VertexIter v_it = mesh.vertices_begin();
	for( int count=0; v_it != mesh.vertices_end(); ++v_it, count++ )
	{
		if( count%100==0 )
			printf("Projecting mesh %3.0f%% \r", 100.*count / (float)mesh.n_vertices());

		// Define ray
		Mesh::Point ray_origin = mesh.point( *v_it );
		Mesh::Normal ray_dir = mesh.normal( *v_it );

		// Result intersections
		std::vector<Intersection> intersections;

		// Brute-force intersection calculation with all surface triangles
		Mesh::FaceIter f_it = surface.faces_begin();
		for( ; f_it != surface.faces_end(); ++f_it )
		{
			// Collect vertex coordinates of a triangle
			static std::vector<Mesh::Point> points(3);
			Mesh::FaceVertexIter fv_it = surface.cfv_iter( f_it );
			for( int i=0; fv_it && i < 3; ++fv_it, i++ )
				points[i] = surface.point( fv_it );

			// Check for intersection
			Intersection x;
			if( computeIntersection( ray_origin, ray_dir, points[0], points[1], points[2], x.p, x.dist ) )
			{
				intersections.push_back( x );
			}
		}

		if( !intersections.empty() )
		{
			// Surface intersections found, set projected point

			// Sort according to distance
			std::sort( intersections.begin(), intersections.end() );

			// Project onto closest intersection point 
			projected_mesh.point( *v_it ) = intersections.front().p;

			numProjPts++;

			// REMARK: A threshold on surface distance could be useful to 
			//         avoid false matches of far away structures.
		}
		else
		{
			// No projection point found, mark vertex for deletion
			projected_mesh.delete_vertex( *v_it, false );
		}
	}
	printf("Projected %d points  \n",numProjPts);

	// Copy to output
	mesh = projected_mesh;
}

int main( int argc, char* argv[] )
{
	// -- Parse command line		
	if( argc != 4 && argc != 5 )
	{
		std::cout << g_usage;
		return 0;
	}
	
	// -- Load meshes
	Mesh source, target;
	if( !loadMesh( source, argv[1] ) ) return -1;
	if( !loadMesh( target, argv[2] ) ) return -1;
	
	std::cout << "Loaded source mesh '" << argv[1] << "'" << std::endl;
	printMeshInfo( source );

	std::cout << "Loaded target mesh '" << argv[2] << "'" << std::endl;
	printMeshInfo( target );

	// -- Update target normals (for point-to-plane ICP variant)
	std::cout << "Updating vertex normals..." << std::endl;
	updateMeshVertexNormals( &target );

	// -- Perform ICP alignment
	SICP::Parameters parm;
	parm.max_icp = 10;
	parm.callback = callback;
	parm.p = 0.4;

	if( argc==4 )
	{
		// Compute only alignment
		alignMeshes( source, target, parm );

		// Save result
		std::cout << "Saving aligned source mesh to '" << argv[3] << "'" << std::endl;
		saveMesh( source, argv[3] );
	}
	else
	{
		alignMeshes( source, target, parm );

		projectMesh( target, source );
		target.garbage_collection();

		// Save results
		std::cout << "Saving aligned source mesh to '" << argv[3] << "'" << std::endl;
		saveMesh( source, argv[3] );

		std::cout << "Saving projected target to '" << argv[4] << "'" << std::endl;
		saveMesh( target, argv[4] );
	}

#if 0 // OBSOLETE REGISTRATION HACK CODE
	{
		// Compute also registered meshes
		Mesh aligned_source;
		registerMeshes( source, target, aligned_source, parm );

		// Save results
		std::cout << "Saving aligned source mesh to '" << argv[3] << "'" << std::endl;
		saveMesh( aligned_source, argv[3] );

		std::cout << "Saving registered source mesh to '" << argv[4] << "'" << std::endl;
		saveMesh( source, argv[4] );

		std::cout << "Saving registered target mesh to '" << argv[5] << "'" << std::endl;
		saveMesh( target, argv[5] );
	}
#endif

	return 0;
}
