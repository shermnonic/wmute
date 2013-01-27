#include "RGBDFrameRenderer.h"
#include "RGBDFilters.h"
#include <GL/glew.h>

RGBDFrameRenderer::RGBDFrameRenderer()
{
	m_vpos.update( 640, 480, 3 );
	m_vcol.update( 640, 480, 4 );
	m_vind_triangles.reserve( 640*480*3*2 );
}

void RGBDFrameRenderer::render( RGBDFrame& frame, RGBDLocalFilter* filter, int mode )
{
#if 0
	frame.renderImmediate( filter );
#else
	// TODO: Applying filter is not required on every render() call but only
	//       when the frame content changes. The update call should therefore
	//       not be executed here.
	updateVertexBuffer( frame, filter );

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );
	glColorPointer ( 4, GL_FLOAT, 0, m_vcol.ptr(0,0) );
	glVertexPointer( 3, GL_FLOAT, 0, m_vpos.ptr(0,0) );

	if( mode == RenderSurface )
	{
		glDrawElements( GL_TRIANGLES, (int)m_vind_triangles.size(), 
			GL_UNSIGNED_INT, &m_vind_triangles[0] );
	}
	else
	// mode == RenderPoints
	{
		glPointSize( 2.f );
		glDrawArrays( GL_POINTS, 0, m_vpos.width*m_vpos.height );
	}

	glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_VERTEX_ARRAY );
#endif
}

void RGBDFrameRenderer::updateIndexsets( int width, int height )
{
	static int last_width = -1;
	static int last_height = -1;

	// Changed resolution requires update of index sets
	if( last_width == width && last_height == height &&
		m_vind_triangles.size() == width*height*2*3 )
		return;

	last_width  = width;
	last_height = height;

	// Indexed set of single triangles (can even be faster than triangle strip)
	m_vind_triangles.resize( width*height*2*3 );
	int i=0;
	for( int y=0; y < height-1; y++ )
		for( int x=0; x < width-1; x++ )
		{
			m_vind_triangles[i++] = y*width + x;
			m_vind_triangles[i++] = (y+1)*width + x;
			m_vind_triangles[i++] = (y+1)*width + x+1;
			
			m_vind_triangles[i++] = y*width + x;
			m_vind_triangles[i++] = (y+1)*width + x+1;
			m_vind_triangles[i++] = y*width + x+1;
		}
}

void RGBDFrameRenderer::updateVertexBuffer( RGBDFrame& frame, RGBDLocalFilter* filter )
{
	RGBDFrame::Buffer& depth = frame.getDepthBuffer();
	RGBDFrame::Buffer& color = frame.getColorBuffer();

	updateIndexsets( depth.width, depth.height );

	m_vpos.update( depth.width, depth.height, 3 );
	m_vcol.update( depth.width, depth.height, 4 );

	int ofs = 0;
	for( int y=0; y < depth.height; y++ )
		for( int x=0; x < depth.width; x++, ofs++ )
		{
			float dx = x / (float)(depth.width-1),
				  dy = y / (float)(depth.height-1),
				  dz = depth.data[ y*depth.width + x ];

			// map color
			float r = *(color.ptr(x,y)),
				  g = *(color.ptr(x,y)+1),
				  b = *(color.ptr(x,y)+2),
				  a = 1.0;
			if( filter ) 
				filter->processColor( r, g, b, a );

			m_vcol.data[4*ofs+0] = r;
			m_vcol.data[4*ofs+1] = g;
			m_vcol.data[4*ofs+2] = b;
			m_vcol.data[4*ofs+3] = a;

			// map position
			float px = 2*dx-1,
				  py = 2*dy-1,
				  pz = dz;
			bool discard = false;
			if( filter ) 
				filter->processPosition( px, py, pz, discard );

			m_vpos.data[3*ofs+0] = px;
			m_vpos.data[3*ofs+1] = py;
			m_vpos.data[3*ofs+2] = pz;

			if( discard )
			{
				// WORKAROUND: Map discarded vertices to special color.
				// Better solution could be to use an additional index array.
				m_vcol.data[4*ofs+0] = 0;
				m_vcol.data[4*ofs+1] = 0;
				m_vcol.data[4*ofs+2] = 0;
			}
		}
}
