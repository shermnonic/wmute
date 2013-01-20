#include "RGBDMovie.h"

#include <QDir>
#include <QStringList>
#include <iostream>
#include <limits>

bool RGBDMovie::loadMovie( QString path, int format )
{
	// Only supported format is rgbd-demo so far.

	// FormatRGBDDemo
	// Directory structure of rgbd-demo realtime color+depth grab:
	//
	// [grab]
	// +- [device-serial-nr]
	//    +- view0000-[timestamp]
	//       +- raw
	//          +- color.png
	//          +- depth.raw
	//    +- view0001-[timestamp]
	//    ...
	//    +- view[#frames]-[timestamp]
	//
	// Base directory [device-serial-nr] with sub-directories for each frame.

	QDir dir( path );
	QStringList filter("view*");
	QStringList views = dir.entryList( filter, QDir::Dirs, QDir::Name );

	m_frames.reserve( views.size() );

	for( int i=0; i < views.size(); i++ )
	{
		// Extract timecode from directory name
		QString timecode_s = views[i].section( '-', 1 );
		bool ok;
		float timecode = timecode_s.toFloat( &ok );
		if( !ok )
		{
			timecode = -1.f; // Special value -1 for invalid timecode
			std::cerr << "Warning: Could not extract timecode for "
				<< views[i].toStdString() << "!" << std::endl;
		}

		// Try to load RGBD frame
		RGBDFrame frame;
		if( frame.loadDir( path + "/" + views[i] + "/raw" ) )
		{
			frame.setTimeCode( timecode );
			m_frames.push_back( frame );
		}
		else
		{
			std::cerr << "Warning: Could not load rgbd frame from "
				<< views[i].toStdString() << "!" << std::endl;
		}
	}

	return m_frames.size()>0;
}


RGBDFrame* RGBDMovie::getFrame( float timecode )
{
	// Linear search
	int bestIdx = -1;
	float bestDist = std::numeric_limits<float>::max();
	for( int i=0; i < m_frames.size(); i++ )
	{
		// Absolute nearest
		// (Alternatively one could only consider positive distance.)
		float dist = m_frames[i].getTimecode() - timecode;
		if( fabs(dist) < bestDist )
		{
			bestIdx = i;
			bestDist = fabs(dist);
		}
	}

	if( bestIdx >= 0 && bestIdx < m_frames.size() )
		return &m_frames[bestIdx];

	return NULL;
}

void RGBDMovie::getTimeRange( float& vmin, float& vmax ) const
{
	// Linear search
	int imin=-1, imax=-1;

	vmin =  std::numeric_limits<float>::max();
	vmax = -std::numeric_limits<float>::max();

	for( int i=0; i < m_frames.size(); i++ )
	{
		float tc = m_frames[i].getTimecode();

		if( tc < vmin ) { vmin = tc; imin = i; }
		if( tc > vmax ) { vmax = tc; imax = i; }
	}
}
