#include "RGBDFrame.h"
#include <QImage>
#include <QObject>
#include <QDir>
#include <QRgb>
#include <QMessageBox>
#include <fstream>
#include <cassert>
#include <iostream>

#include <GL/glew.h>
void RGBDFrame::render()
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
			glColor3fv( m_color.ptr(x,y) ); // Assume identical size
			//  m_color.ptr(floor(dy*m_color.height), floor(dx*m_color.width)) );		

			glVertex3f( 2*dx-1, 2*dy-1, -dz );
		}

	glEnd();
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
