#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#include <cmath> // for sqrt()

/// Geometry utilities for \a RenderArea
namespace Geometry {
	
/// Polygon with texture coordinates
template <typename T, int DIM, int TC>
class TPolygon
{
public:
	void clear() { m_verts.clear(); m_texcoords.clear(); }

	void resize( int n ) { m_verts.resize( DIM * n ); m_texcoords.resize( TC * n ); }	
	int  nverts() const { return (int)m_verts.size() / DIM; }

		  T* vert( int i )       { return (i<0 || i>=m_verts.size()) ? 0 : &m_verts[DIM*i]; }
	const T* vert( int i ) const { return &m_verts[DIM*i]; }

		  T* texcoord( int i )       { return (i<0 || i>=m_texcoords.size()) ? 0 : &m_texcoords[TC*i]; }
	const T* texcoord( int i ) const { return &m_texcoords[TC*i]; }

	std::vector<T>& verts() { return m_verts; }
	std::vector<T>& texcoords() { return m_texcoords; }

private:
	std::vector<T> m_verts;
	std::vector<T> m_texcoords;
};

/// Dot product
template<int D, typename T> T dotprod( const T* u, const T* v )
{
	T res=0.f;
	for( int i=0; i < D; i++ )
		res += u[i]*v[i];
	return res;
}

/// Length of vector between u and v
template<int D, typename T> T length( const T* u, const T* v )
{
	T res=0.f;
	for( int i=0; i < D; i++ )
		res += (u[i]-v[i])*(u[i]-v[i]);
	return sqrt(res);
}

/// Projective mapping for a 2D quad via homogeneous coordinates
/// See also http://www.reedbeta.com/blog/2012/05/26/quadrilateral-interpolation-part-1/
template<typename T>
class THomogeneousQuad2D
{
public:
	void compute( const T* v0, const T* v1, const T* v2, const T* v3 )
	{
		// Normal of diagonal (v0,v2)
		float n[2];
		n[0] = -(v2[1] - v0[1]);
		n[1] =  v2[0] - v0[0];
		// Normalize
		float nl = sqrt(dotprod<2>(n,n));
		n[0] /= nl;
		n[1] /= nl;

		// Direction of second diagonal (v1,v3)
		float d[2];
		d[0] = v3[0] - v1[0];
		d[1] = v3[1] - v1[1];

		// Plug parametric form of 2nd into normal form of 1st diagonal
		float t = 
			(dotprod<2>( v0, n ) - dotprod<2>( v1, n ))
			/ dotprod<2>( n, d ); // FIXME: Check for colinearity!

		// Get intersection point (evaluate parametric 1st diagonal at t)
		m_p[0] = v1[0] + t * d[0];
		m_p[1] = v1[1] + t * d[1];

		// Homogeneous weights
		T s0 = length<2>( v0, m_p ),
		  s1 = length<2>( v1, m_p ),
		  s2 = length<2>( v2, m_p ),
		  s3 = length<2>( v3, m_p );
		m_q[0] = (s0+s2)/s2;
		m_q[1] = (s1+s3)/s3;
		m_q[2] = (s0+s2)/s0;
		m_q[3] = (s1+s3)/s1;
	}

	const T* getIntersection() const { return m_p; }
	const T* getHomogeneousWeights() const { return m_q; }

private:
	T m_p[2]; ///< Point of intersection between quad diagonals
	T m_q[4]; ///< Homogeneous weights for each vertex
};

}; // namespace Geometry

#endif // GEOMETRY_H
