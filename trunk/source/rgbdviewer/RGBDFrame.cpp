#include "RGBDFrame.h"
#include "RGBDFilters.h" // for: RGBDLocalFilter
#include <QImage>
#include <QObject>
#include <QDir>
#include <QRgb>
#include <QMessageBox>
#include <fstream>
#include <cassert>
#include <iostream>

#include <GL/glew.h>
void RGBDFrame::render( RGBDLocalFilter* filter, int mode )
{
#if 0
	renderImmediate( filter );
#else
	updateVertexBuffer( filter );

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

void RGBDFrame::renderImmediate( RGBDLocalFilter* filter )
{
	const float depthScale = 1.0f;

	glPointSize( 2.f );
	glBegin( GL_POINTS );

	for( int y=0; y < m_depth.height; y++ )
		for( int x=0; x < m_depth.width; x++ )
		{
			float dx = x / (float)(m_depth.width-1),
				  dy = y / (float)(m_depth.height-1),
				  dz = m_depth.data[ y*m_depth.width + x ] * depthScale;

			// map color
			float r = *(m_color.ptr(x,y)),
				  g = *(m_color.ptr(x,y)+1),
				  b = *(m_color.ptr(x,y)+2),
				  a = 1.0;
			if( filter ) filter->processColor( r, g, b, a );
			glColor4f( r, g, b, a );
			// was:
			//  glColor3fv( m_color.ptr(x,y) ); // Assume identical size
			//  m_color.ptr(floor(dy*m_color.height), floor(dx*m_color.width)) );		

			// map position
			float px = 2*dx-1,
				  py = 2*dy-1,
				  pz = dz;
			bool discard = false;
			if( filter ) filter->processPosition( px, py, pz, discard );
			if( !discard )
				glVertex3f( px, py, pz );
			// was: 
			// glVertex3f( 2*dx-1, 2*dy-1, dz );
		}

	glEnd();
}

void RGBDFrame::updateIndexsets( int width, int height )
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

void RGBDFrame::updateVertexBuffer( RGBDLocalFilter* filter )
{
	updateIndexsets( m_depth.width, m_depth.height );

	m_vpos.update( m_depth.width, m_depth.height, 3 );
	m_vcol.update( m_depth.width, m_depth.height, 4 );

	int ofs = 0;
	for( int y=0; y < m_depth.height; y++ )
		for( int x=0; x < m_depth.width; x++, ofs++ )
		{
			float dx = x / (float)(m_depth.width-1),
				  dy = y / (float)(m_depth.height-1),
				  dz = m_depth.data[ y*m_depth.width + x ];

			// map color
			float r = *(m_color.ptr(x,y)),
				  g = *(m_color.ptr(x,y)+1),
				  b = *(m_color.ptr(x,y)+2),
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

bool RGBDFrame::loadDir( QString path )
{
	QDir d(path);

	// Assume rgbd-demo file naming convention
	if( !d.exists("depth.raw") || !d.exists("color.png") )
	{
		QMessageBox::warning( NULL, QObject::tr("Error"),
			QObject::tr("Invalid RGBD data folder: ") + d.path() );	
		return false;
	}

	// Load color + depth data from disk
	if( !updateColor( d.filePath("color.png") ) || 
		!updateDepth( d.filePath("depth.raw") ))
		return false;

	std::cout << "Loaded succesfully " << path.toStdString() << std::endl;
	m_path = path;
	return true;
}

bool RGBDFrame::updateColor( QString filepath )
{
	// Load PNG
	QImage img;
	if( !img.load( filepath ) )
	{
		QMessageBox::warning( NULL, QObject::tr("Error"),
			QObject::tr("Could not load image: ") +  filepath );	
		return false;
	}

	// Resize buffer if required
	m_color.update( img.width(), img.height(), 3 );

	// Brute force copy (slow)
	int ofs=0;
	for( int y=0; y < img.height(); y++ )
		for( int x=0; x < img.width(); x++, ofs++ )
		{			
			QRgb rgb = img.pixel(x,y);
			m_color.data[ofs*3  ] = qRed  (rgb) / 255.f;
			m_color.data[ofs*3+1] = qGreen(rgb) / 255.f;
			m_color.data[ofs*3+2] = qBlue (rgb) / 255.f;
		}
	
	return true;
}

bool RGBDFrame::updateDepth( QString filepath )
{
	// Open RAW in binary mode
	using namespace std;
	ifstream f( filepath.toStdString().c_str(), ios::binary );
	if( !f.is_open() )
	{
		QMessageBox::warning( NULL, QObject::tr("Error"),
			QObject::tr("Could not load image: ") + filepath );	
		return false;
	}
	
	// Get filesize
	streampos fsize = f.tellg();
	f.seekg( 0, ios::end );
	fsize = f.tellg() - fsize;
	f.seekg( 0, ios::beg );

	// (Re-)Allocate file cache if neccessary
	static vector<char> cache(640*480*4);
	if( cache.size()!=(int)fsize )
		cache.resize( (int)fsize );

	// Cache whole file
	f.read( &cache[0], fsize );
	f.close();

	//--
	// Parse data	

	// First two integers are width and height of depth image
	int h = *(int*)(static_cast<void*>( &cache[0] )),
	    w = *(int*)(static_cast<void*>( &cache[4] ));

	// Sanity check
	if( w*h*4 != ((int)fsize-8) )
	{
		QMessageBox::warning( NULL, QObject::tr("Error"),
			QObject::tr("RAW depth file format mismatch in: ") + filepath );
		return false;
	}

	// Copy raw data (fast)
	assert( sizeof(float)==4 ); // sanity
	m_depth.update( w, h, 1 );
	memcpy( (void*)&(m_depth.data[0]), (void*)&(cache[8]), w*h*4 );

	return true;
}
