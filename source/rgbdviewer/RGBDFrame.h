#ifndef RGBDFRAME_H
#define RGBDFRAME_H

#include <QString>
#include <vector>

class RGBDLocalFilter;

/// Single RGBD frame, provides file IO and rudimentary rendering functionality.
/// Currently only the rgbd-demo format is supported where depth and color raw
/// data files are stored in separate directories and with a specific naming
/// convention.
class RGBDFrame
{
	template <class T>
	struct BufferT
	{
		BufferT(): width(0),height(0), channels(0) {}
		int width, height, channels;
		std::vector<T> data;
		void update( int w, int h, int ch )
		{
			// If size has not changed and data is allocated, leave it as it is
			if( width==w && height==h && channels==ch && data.size()==w*h*ch )
				return;
			// Else we have to allocate data anew
			data.resize( w*h*ch );
			width = w;
			height = h;
			channels = ch;
		}
		T* ptr( int x, int y )
		{
			return &data[channels*(y*width + x)];
		}
	};
	typedef BufferT<float> Buffer;
	typedef std::vector<unsigned int> Indexset;
	
public:
	enum RenderModes { RenderPoints, RenderSurface };

	/// Load depth and color from a directory (rgbd-demo format)
	/// rgbd-demo stores depth and color of a frame in a single directory.
	bool loadDir( QString path );

	/// Simple rendering functionality with applied realtime filter
	void render( RGBDLocalFilter* filter=NULL, int mode=RenderSurface );

	/// Debug rendering functionality
	/// Draws points in immediate mode and applies filter in realtime.
	void renderImmediate( RGBDLocalFilter* filter=NULL );

	QString getPath() const { return m_path; }

	void  setTimeCode( float t ) { m_timecode = t; }
	float getTimecode() const { return m_timecode; }

protected:
	bool updateColor( QString filepath );
	bool updateDepth( QString filepath );

private:
	QString m_path;

	// raw input depth and color data buffers
	Buffer m_depth, m_color;

	float m_timecode;

	// vertex position and color buffer (from processed depth and color data)
	Buffer m_vpos, m_vcol;
	Indexset m_vind_triangles;
	void updateVertexBuffer( RGBDLocalFilter* filter=NULL );
	void updateIndexsets( int width, int height );
};

#endif // RGBDFRAME_H
