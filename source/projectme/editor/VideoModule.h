#ifndef VIDEOMODULE_H
#define VIDEOMODULE_H

#include "ModuleRenderer.h"

#include <glutils/GLTexture.h>
#ifdef GL_NAMESPACE
using GL::GLTexture;
#endif

class VideoPlayer;

/**
	\class VideoModule
	
	Provides video playback (no audio), requires a pointer to the VideoPlayer instance.
*/
class VideoModule : public ModuleRenderer
{
public:
	VideoModule();

	void setVideoPlayer( VideoPlayer* );

	///@name ModuleRenderer implementation
	///@{
	void render();
	int  target() const { return m_target.name(); }	
	void destroy();
	void touch() {}
	void applyOptions() { /* Call init again to change texture size */ init(); }
	///@}	
	
protected:
	// Invoked once in first render() call
	// Creates OpenGL texture
	bool init();

private:
	bool         m_initialized;
	VideoPlayer* m_videoPlayer;
	GLTexture    m_target;
};

#endif // VIDEOMODULE_H
