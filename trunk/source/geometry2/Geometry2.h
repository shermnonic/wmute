#ifndef GEOMETRY2_H
#define GEOMETRY2_H

// First approach was to use vectors of vec3 and Face type, but the corresponding
// float/int data is not tightly packed in memory because class/struct overhead.
// (Leave commented out!)
//#define GEOMETRY2_NO_BUFFER_SUPPORT

#include "Vector.h"  // vec3
#include <vector>

//==============================================================================
//	SimpleGeometry
//==============================================================================

/// Indexed face set in linear tightly packed vertex/index buffers
/// Restricted to triangle facets and vertex normals.
/// Internally getters map to more convenient custom vector/face type.
class SimpleGeometry
{
public:
	/// Return number of vertices/normals
	int num_vertices() const;
	/// Return number of faces
	int num_faces()    const;

	///@{ Get pointer to tightly packed buffer
	float* get_vertex_ptr();
	float* get_normal_ptr();
	int*   get_index_ptr();
	///@}

	void clear();

protected:
	/// Reserve memory for vertices
	void reserve_vertices( int n );
	/// Reserve memory for face indices
	virtual void reserve_faces( int n );
	
	/// Internal triangle face type
	/// Assume identical indices for vertices/normals
	struct Face 
	{ 
		Face();
		Face( int i[3] );
		Face( int i, int j, int k );
		int vi[3];
	};

public:
	vec3 get_vertex( int i ) const;
	vec3 get_normal( int i ) const;
	Face get_face  ( int j ) const;
	
	/// Insert face
	virtual int  add_face( Face f );

	/// Insert vertex with normal, returns vertex index
	/// (Yes, you *have* to supply a normal :-)
	int add_vertex_and_normal( vec3 v, vec3 n );
	
private:
#ifndef GEOMETRY2_NO_BUFFER_SUPPORT
	std::vector<float> m_vdata;
	std::vector<float> m_ndata;	 // vertex normals
	std::vector<int>   m_fdata;
#else
	std::vector<vec3> m_vertices;
	std::vector<vec3> m_normals;
	std::vector<Face> m_faces;
#endif
};

//==============================================================================
//	Icosahedron
//==============================================================================

/// Icosahedron with subdivision, nice regular sphere approximation
class Icosahedron : public SimpleGeometry
{
public:
	Icosahedron():
		m_levels(4),
		m_platonicConstantX(.525731112119133606),
		m_platonicConstantZ(.850650808352039932)
		{}

	void setPlatonicConstants( double X, double Z )
		{
			m_platonicConstantX = X;
			m_platonicConstantZ = Z;
		}
	double getPlatonicConstantsX() const { return m_platonicConstantX; }
	double getPlatonicConstantsZ() const { return m_platonicConstantZ; }

	void setLevels( int levels ) 
		{ 
			m_levels = levels; 
			if( m_levels <= 0 ) m_levels=1;
		};
	int getLevels() const { return m_levels; }

	/// Create an Icosahedron model where each face is subdivided level times
	/// In the limit the subdivision surface is a sphere.
	void create( int level=-1 );

protected:
	/// Recursive face subdivision routine
	/// For levels=0 the face is inserted into the model
	void add_face_subdivision( Face f, int levels );

private:
	int    m_levels;
	double m_platonicConstantX,
		   m_platonicConstantZ;
};

//==============================================================================
//	Penrose tiling
//==============================================================================

/// Generate a Penrose tiling pattern
/// Implementation uses a subdivision approach as described on
///   http://preshing.com/20110831/penrose-tiling-explained/
class Penrose : public SimpleGeometry
{
public:
	enum FaceTypes { Blue, Red };
	
	Penrose();

	void create( int levels=-1 );

	void setDefaultGenerator();
	void setGenerator( const SimpleGeometry& geom );

	void setPlatonicConstants( double X, double Z ) {}
	double getPlatonicConstantsX() const { return 0; }
	double getPlatonicConstantsZ() const { return 0; }

	void setLevels( int levels ) 
		{ 
			m_levels = levels; 
			if( m_levels <= 0 ) m_levels=1;
		};
	int getLevels() const { return m_levels; }

protected:
	///@{ Implement custom face type attribute
	virtual void reserve_faces( int n );
	virtual int add_face( SimpleGeometry::Face f );
	///@}
	// Internal function called for each added face setting also type attribute
	int add_face( SimpleGeometry::Face f, int type );

	void add_face_subdivision( SimpleGeometry::Face f, int type, int levels );
	
private:	
	int m_levels;	
	std::vector<int> m_faceType;
	SimpleGeometry m_generator;
};

#endif

