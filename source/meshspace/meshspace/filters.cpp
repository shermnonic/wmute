#include "filters.h"
#include <ICP.h> // for nanoflann::KDTreeAdaptor

namespace filters {
	
void closestPointDistance( const Mesh& source_mesh, const Mesh& target_mesh, std::vector<float>& dist )
{
	Eigen::Matrix3Xd X, Y;
	convertMeshToMatrix( source_mesh, X );
	convertMeshToMatrix( target_mesh, Y );
	
	// Build kd-tree
	nanoflann::KDTreeAdaptor<Eigen::Matrix3Xd, 3, nanoflann::metric_L2_Simple> kdtree( Y );
	
	// Find closest points	
	dist.resize( X.cols() );
	#pragma omp parallel for
	for(int i=0; i<X.cols(); ++i) 
	{
		int j = kdtree.closest(X.col(i).data());		
		dist[i] = (X.col(i) - Y.col(j)).norm();		
	}
}

};
