#ifndef SOUNDMODULE_H
#define SOUNDMODULE_H

#include "RenderSet.h" // for ModuleRenderer

#include <glutils/GLTexture.h>
#ifdef GL_NAMESPACE
using GL::GLTexture;
#endif

class SoundInput;

/**
	\class SoundModule
	
	Provides audio waveform and FFT, requires a pointer to a SoundInput instance.
*/
class SoundModule : public ModuleRenderer
{
public:
	SoundModule();

	void setSoundInput( SoundInput* );

	///@name ModuleRenderer implementation
	///@{
	void render();
	int  target() const { return m_target.GetID(); }	
	void destroy();
	void touch() {}
	void applyOptions() { /* Call init again to change texture size */ init(); }
	///@}	
	
protected:
	// Invoked once in first render() call
	// Creates OpenGL texture
	bool init();

	// Uploads data to texture. Called in render().
	void updateTexture();

private:
	bool        m_initialized;
	SoundInput*	m_soundInput;
	GLTexture   m_target;
	float       m_data[2*512]; // Texture data
};

#endif // SOUNDMODULE_H
