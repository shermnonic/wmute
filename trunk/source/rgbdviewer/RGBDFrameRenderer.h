#ifndef RGBDFRAMERENDERER_H
#define RGBDFRAMERENDERER_H
#include "RGBDFrame.h"

class RGBDLocalFilter;

/// Render \a RGBDFrame with applied realtime filter
class RGBDFrameRenderer
{
	typedef RGBDFrame::Buffer         Buffer;
	typedef std::vector<unsigned int> Indexset;

public:
	enum RenderModes { RenderPoints, RenderSurface };

	RGBDFrameRenderer();

	/// Render \a RGBDFrame with applied realtime filter
	void render( RGBDFrame& frame, RGBDLocalFilter* filter=NULL, 
	             int mode=RenderSurface );

protected:
	void updateVertexBuffer( RGBDFrame& frame, RGBDLocalFilter* filter );
	void updateIndexsets( int width, int height );

private:	
	Buffer m_vpos,  // vertex position buffer
		   m_vcol;  // vertex color buffer
	Indexset m_vind_triangles; // index buffer for triangles
};

#endif // RGBDFRAMERENDERER_H
