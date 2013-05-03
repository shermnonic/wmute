#ifndef MARCHINGCUBES_H
#define MARCHINGCUBES_H

#include "vector3.h"

class MarchingCubes
{
public:
	MarchingCubes()
	{
		datasize = 16;
		isovalue = 48.f;
		scale = 1.f/16.f;
		compute_normals = true;
	};

	MarchingCubes( float i_isovalue, float i_scale, int i_datasize )
	{
		datasize = i_datasize;
		isovalue = i_isovalue;
		scale = i_scale;		
		compute_normals = true;
	};

	/// Override this by the function to be polygonized
	virtual float sample( float x, float y, float z );
	
	void draw_mcubes()
	{
		for( int x=0; x < datasize; x++ )
		for( int y=0; y < datasize; y++ )
		for( int z=0; z < datasize; z++ )
			marchcube( (x-datasize/2)*scale, (y-datasize/2)*scale, (z-datasize/2)*scale );
	};
	
	void  set_isovalue( float iso ) { isovalue = iso; };
	float get_isovalue() { return isovalue; };
	
	void  set_compute_normals( bool b ) { compute_normals = b; }

protected:
	void  marchcube( float x, float y, float z );
	float get_offset( float val1, float val2, float desired );

private:
	float isovalue;
	float scale;
	int   datasize;
	bool  compute_normals;
};

#endif
