#ifndef RGBDFRAME_H
#define RGBDFRAME_H

#include <QString>
#include <vector>

/// Single RGBD frame, provides file IO and preliminary rendering function
/// Currently only the rgbd-demo format is supproted where depth and color raw
/// data files are stored in separate directories and with a specific naming
/// convention.
class RGBDFrame
{
public:
	// Load depth and color from a directory (rgbd-demo format)
	// rgbd-demo stores depth and color of a frame in a single directory
	bool loadDir( QString path );

	// Debug rendernig functionality (draws immediate mode points)
	void render();

	QString getPath() const { return m_path; }

	void  setTimeCode( float t ) { m_timecode = t; }
	float getTimecode() const { return m_timecode; }

protected:
	bool updateColor( QString filepath );
	bool updateDepth( QString filepath );

private:
	QString m_path;

	struct Buffer
	{
		Buffer(): width(0),height(0), channels(0) {}
		int width, height, channels;
		std::vector<float> data;
		void update( int w, int h, int ch )
		{
			// If size does not change and data is allocated, leave it as is
			if( width==w && height==h && channels==ch && data.size()==w*h*ch )
				return;
			// Else we have to allocate data anew
			data.resize( w*h*ch );
			width = w;
			height = h;
			channels = ch;
		}
		float* ptr( int x, int y )
		{
			return &data[channels*(y*width + x)];
		}
	};
	Buffer m_depth, m_color;

	float m_timecode;
};

#endif // RGBDFRAME_H
