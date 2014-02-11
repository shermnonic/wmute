#include "TransferFunction.h"
#include <glutils/GLError.h>
#include <iostream>

bool TransferFunction::create()
{
// Poettkow probability colormap
	float tf_poettkow[] = {
	 0.3137f,    0.6784f,    0.9020f, 
	 0.4118f,    0.5608f,    0.8235f, 
	 0.5176f,    0.4314f,    0.7255f, 
	 0.6235f,    0.3020f,    0.6392f, 
	 0.7294f,    0.1725f,    0.5490f, 
	 0.8392f,    0.0431f,    0.4549f, 
	 0.8902f,    0.0902f,    0.3725f, 
	 0.9137f,    0.2353f,    0.2941f, 
	 0.9373f,    0.3843f,    0.2118f, 
	 0.9608f,    0.5294f,    0.1333f, 
	 0.9804f,    0.6745f,    0.0588f, 
	 1.0000f,    0.7922f,    0.0431f, 
	 1.0000f,    0.8353f,    0.2392f, 
	 1.0000f,    0.8745f,    0.4314f, 
	 1.0000f,    0.9176f,    0.6196f, 
	 1.0000f,    0.9608f,    0.8314f, 
	 1.0000f,    1.0000f,    1.0000f
	};
	
	int n = sizeof(tf_poettkow) / (3 * sizeof(float));
	
	// Create texture
	if( !m_tex.Create( GL_TEXTURE_1D ) )
	{
		std::cerr << "TransferFunction::createTexture() : Could not create "
			"GL texture for color lookup table!" << std::endl;
		return false;
	}	
	
	// Download data to GPU
	m_tex.Image( 0, GL_RGBA32F, n, 0, GL_RGB, GL_FLOAT, (void*)tf_poettkow );
	
	// Set texture parameters
	m_tex.SetWrapMode( GL_CLAMP_TO_EDGE );
	m_tex.SetFilterMode( GL_LINEAR );
	
	return true;
}

void TransferFunction::destroy()
{
	m_tex.Destroy();
}

void TransferFunction::bind( int tex_unit )
{
	m_tex.Bind( tex_unit );
}

void TransferFunction::release()
{
	m_tex.Unbind();
	GL::CheckGLError("TransferFunction::release()");
}
