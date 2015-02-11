#include "TransferFunction.h"
#include <glutils/GLError.h>
#include <iostream>

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

void TransferFunction::getColor( float scalar, float& r, float &g, float &b ) const
{
	// Lookup table
	int n = sizeof(tf_poettkow) / (3 * sizeof(float));
	float* tf = tf_poettkow;
	
	// Compute indices into lookup tables
	float s = scalar * (n-1);
	int i0 = (int)floor(s);
	int i1 = (int)ceil (s);

	// Mixture coefficient
	float alpha = s - i0;

	// Clamp
	if( i0 > n-1 ) i0=n-1;
	if( i1 > n-1 ) i1=n-1;

	// Return mixed color
	r = (1.f-alpha)*tf[3*i0+0] + alpha*tf[3*i1+0];
	g = (1.f-alpha)*tf[3*i0+1] + alpha*tf[3*i1+1];
	b = (1.f-alpha)*tf[3*i0+2] + alpha*tf[3*i1+2];
}

bool TransferFunction::create()
{
	int n = sizeof(tf_poettkow) / (3 * sizeof(float));
	
	// Create texture
	if( !m_tex.create( GL_TEXTURE_1D ) )
	{
		std::cerr << "TransferFunction::createTexture() : Could not create "
			"GL texture for color lookup table!" << std::endl;
		return false;
	}	
	
	// Download data to GPU
	m_tex.image( 0, GL_RGBA32F, n, 0, GL_RGB, GL_FLOAT, (void*)tf_poettkow );
		
	// Set texture parameters
	m_tex.setWrapMode( GL_CLAMP_TO_EDGE );
	m_tex.setFilterMode( GL_LINEAR );
	
	return true;
}

void TransferFunction::destroy()
{
	m_tex.destroy();
}

void TransferFunction::bind( int tex_unit )
{
	m_tex.bind( tex_unit );
}

void TransferFunction::release()
{
	m_tex.unbind();
	GL::CheckGLError("TransferFunction::release()");
}

void TransferFunction::draw() const
{
	// Bar geometry
	float major = .3f,
		  minor = .02f,
		  border = .02f;
	bool vertical = false,
	     centered = true,
		 outer = vertical ? true : false;

	float x0,y0, x1,y1;
	{
		float pos = (float)(outer ? (1. - border) : border);

		// define horizontal bar
		if( centered )
		{
			// y centered
			y0 = major;
			y1 = 1.f - major;
		}
		else
		{
			// ?
		}

		x0 = pos - minor/2.f;
		x1 = pos + minor/2.f;

		// flip if required
		if( !vertical ) 
		{
			// x centered
			std::swap( x0, y0 );
			std::swap( x1, y1 );
		}
	}

	// Store relevant state
	glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT );

	//// Store current viewport
	//GLint viewport[4];
	//glGetIntegerv( GL_VIEWPORT, viewport );

	// Store current projection and modelview
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();

	// Set orthographic projection
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0,1,0,1 ); // should be the same as: glOrtho( 0,1,0,1, -1,1 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// Make sure we are not using any shader!
	glUseProgram( 0 );
	glActiveTexture( GL_TEXTURE0 );

	// Draw colorbar
	glColor4f( 1.f,1.f,1.f,1.f );
	glEnable( GL_TEXTURE_1D );
	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );
	glBindTexture( GL_TEXTURE_1D, m_tex.name() ); 	// <- bind() is not const!
	glBegin( GL_QUADS );
	glMultiTexCoord1f( GL_TEXTURE0, vertical ? 0.f : 0.f );	glVertex2f( x0, y0 );
	glMultiTexCoord1f( GL_TEXTURE0, vertical ? 1.f : 0.f );	glVertex2f( x0, y1 );
	glMultiTexCoord1f( GL_TEXTURE0, vertical ? 1.f : 1.f );	glVertex2f( x1, y1 );
	glMultiTexCoord1f( GL_TEXTURE0, vertical ? 0.f : 1.f );	glVertex2f( x1, y0 );
	glEnd();

	// Restore previous projection and modelview
	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );

	//// Restore current viewport
	//glViewport( viewport[0], viewport[1], viewport[2], viewport[3] );

	// Restore previous state
	glPopAttrib();
}
