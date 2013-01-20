#ifndef RGBDMOVIE_H
#define RGBDMOVIE_H

#include "RGBDFrame.h"
#include <vector>
#include <QString>

class QWidget;

/// Collection of \a RGBDFrame, in-memory
class RGBDMovie
{
public:
	enum Formats { FormatRGBDDemo };

	bool loadMovie( QString path, int format=FormatRGBDDemo, 
	                QWidget* parentForProgressDialog=NULL );

	/// Return number of frames
	int nFrames() const { return (int)m_frames.size(); }

	/// Return frame with timecode closest to given one
	RGBDFrame* getFrame( float timecode );

	void getTimeRange( float& min, float& max ) const;

private:
	std::vector<RGBDFrame> m_frames;
};

#endif // RGBDMOVIE_H
