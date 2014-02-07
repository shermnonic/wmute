// Max Hermann, Jan 2014
#ifndef INTERSECT_H
#define INTERSECT_H

/// Simple ray triangle intersection code (with *no* dependencies).
/// Code should be self-explanatory.
/// @author Max Hermann (hermann@cs.uni-bonn.de)
namespace Intersect
{
	struct Vec3
	{
		Vec3(): x(0.),y(0.),z(0.) {}
		Vec3( double x_, double y_, double z_ ): x(x_), y(y_), z(z_) {}
		double x,y,z;

		Vec3& operator += ( const Vec3& other ) { x += other.x;  y += other.y;  z += other.z; return *this; }
		Vec3& operator -= ( const Vec3& other ) { x -= other.x;  y -= other.y;  z -= other.z; return *this; }
		Vec3& operator *= ( double scalar ) { x *= scalar;  y *= scalar;  z *= scalar; return *this; }

		Vec3 operator + ( const Vec3& other ) const { return Vec3(*this) += other; }
		Vec3 operator - ( const Vec3& other ) const { return Vec3(*this) -= other; }
		Vec3 operator * ( double scalar ) const { return Vec3(*this) *= scalar; }

		Vec3 cross( const Vec3& other )
		{
			return Vec3( y*other.z - z*other.y,
				         z*other.x - x*other.z,
						 x*other.y - y*other.x );
		}

		double scalar_prod( const Vec3& other )
		{
			return x*other.x + y*other.y + z*other.z;
		}
	};

	struct Ray 
	{ 
		Ray() {}
		Ray( Vec3 origin_, Vec3 dir_ ): origin(origin_), dir(dir_) {}
		Vec3 origin; 
		Vec3 dir; 
	};

	/// Barycentric coordinates (1-u-v,u,v)
	struct UV
	{
		double u, v;
		
		bool is_inside() const 
		{
			return (1.-u-v)>=0. && u>=0. && v>=0. &&
				   (1.-u-v)<=1. && u<=1. && v<=1.;				   
		}
	};

	struct Triangle 
	{
		Vec3 v0, v1, v2;
		Vec3 barycentric( UV bc )
		{
			return v0*(1.-bc.u-bc.v) + v1*bc.u + v2*bc.v;
		}
	};

	struct RayTriangleIntersection
	{
		UV bc;
		double t;
	};
	
	/// Ray triangle intersection as described in
	/// "Fast, Minimum Storage Ray/Triangle Intersection" by Möller and Trumbore
	/// http://www.cs.virginia.edu/~gfx/Courses/2003/ImageSynthesis/papers/Acceleration/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
	RayTriangleIntersection intersect( const Ray& r, const Triangle& t )
	{
		Vec3 T  = r.origin - t.v0,
			 E1 = t.v1 - t.v0,
			 E2 = t.v2 - t.v0,
			 D  = r.dir,
		     P  = D.cross(E2),
			 Q  = T.cross(E1);

		double scale = 1./P.scalar_prod(E1);

		RayTriangleIntersection rti;		
		rti.t    = scale*(Q.scalar_prod(E2));
		rti.bc.u = scale*(P.scalar_prod(T));
		rti.bc.v = scale*(Q.scalar_prod(D));
		return rti;
	}
};

#endif // INTERSECT_H
