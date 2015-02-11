#include "SoundModule.h"
#include "SoundInput.h"

#include <glutils/GLError.h>
#ifdef GL_NAMESPACE
using GL::checkGLError;
#endif

#include <cstring> // memcpy
#include <iostream>
#include <cmath> // fabs, log
using std::cerr;
using std::endl;

//----------------------------------------------------------------------------
//  Factory registration
//----------------------------------------------------------------------------
#include "ModuleFactory.h"
MODULEFACTORY_REGISTER( SoundModule, "SoundModule" )


SoundModule::SoundModule()
: ModuleRenderer( "SoundModule" ),
  m_initialized( false ),
  m_soundInput( NULL )  
{
}

void SoundModule::setSoundInput( SoundInput* soundInput )
{
	m_soundInput = soundInput;
}

void SoundModule::updateTexture()
{
	if( !m_initialized && !init() && !m_soundInput)
		return;	
	
	// Get sound data
	short buffer[512];
	int written = m_soundInput->pollSampleData( buffer, 512 );
	float *fft = m_soundInput->pollFFT();
	
	// FFT can directly be copied (or do we have to normalize somehow???)
	memcpy( &m_data[512], fft, sizeof(float)*512 );
	
	// Waveform data has to be converted from short to float
	for( int i=0; i < 512; i++ )
	{
#if 1
		// Decibel mapping

		// sample in [-1,1]
		float sample = (float)buffer[i] / (.5f*65536.f);

		// decibels 
		float sign = (sample > 0.f) ? 1.f : -1.f;
		sample = log(1.f+fabs(2.f*sample));

		m_data[i] = .5f + sign*sample;
#else
		// Raw mapping
		const float scale = (float)(.5 + 1. / 65536.); // Scale to range [0,1]
		m_data[i] = (float)buffer[i] * scale;
#endif
	}
	
	// Download data to GPU
	m_target.image( 0, GL_LUMINANCE16, 512,2, 0, 
	//m_target.SubImage( 0, 0,0, 512,2, 
		GL_LUMINANCE, GL_FLOAT, (void*)m_data );	
}

bool SoundModule::init()
{
	// Create texture
	if( !m_target.create(GL_TEXTURE_2D) )
	{
		cerr << "ImageModule::init() : Couldn't create 2D textures!" << endl;
		return false;
	}
	
	// Fix format and size
	GLint internalFormat = GL_LUMINANCE16; // GL_R16F;
	m_target.image( 0, internalFormat, 512, 2, 0, 
		GL_LUMINANCE, GL_FLOAT, (void*)NULL );	
	
	m_initialized = checkGLError( "ShaderModule::init() : GL error at exit!" );
	return m_initialized;	
}

void SoundModule::destroy()
{
	m_target.destroy();
	//delete [] m_data; m_data = NULL;
}

void SoundModule::render()
{
	// Initialized on first render() call; by then we should have a valid 
	// OpenGL context.	
	if( !m_initialized && !init() ) return;	
	
	if( m_soundInput )
		updateTexture();
}
