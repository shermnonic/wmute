#ifndef RGBDFRAME_H
#define RGBDFRAME_H

#include <QString>
#include <vector>
#include "Buffer2D.h"

class RGBDLocalFilter;

/// Single RGBD frame, provides file IO and rudimentary rendering functionality.
/// Currently only the rgbd-demo format is supported where depth and color raw
/// data files are stored in separate directories and with a specific naming
/// convention.
class RGBDFrame
{
public:
	typedef Buffer2D<float> Buffer;

	/// Load depth and color from a directory (rgbd-demo format)
	/// rgbd-demo stores depth and color of a frame in a single directory.
	bool loadDir( QString path );

	/// Debug rendering functionality
	/// Draws points in immediate mode and applies filter in realtime.
	void renderImmediate( RGBDLocalFilter* filter=NULL );

	QString getPath() const { return m_path; }

	void  setTimeCode( float t ) { m_timecode = t; }
	float getTimecode() const { return m_timecode; }

	// Friend functions for RGBDFrameRenderer
	// FIXME: The buffer getters can not be const because buffer access 
	//        currently is done through (unqualified) direct member access.
	Buffer& getDepthBuffer() { return m_depth; }
	Buffer& getColorBuffer() { return m_color; }

protected:
	bool updateColor( QString filepath );
	bool updateDepth( QString filepath );

private:
	QString m_path;

	// raw input depth and color data buffers
	Buffer m_depth, m_color;

	float m_timecode;
};

#endif // RGBDFRAME_H
