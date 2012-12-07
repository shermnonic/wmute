// Compare improved Noise shader vs. software implementation
// Max Hermann, August 03, 2010
//
// Remarks:
// - Looks now pretty the same for apertures greater than 15.0, but there
//   are still differences visible for smaller apertures like 5.0.
//
// TODO:
// - Optimize NoiseShader by using precomputed lookup tables (see GPUGems2)
// - Implement 4D noise
// - Implement standard functions like turbulence
#include <Engines/GLUT/EngineGLUT.h>
#include <iostream>
#include <exception>
#include <GL/glew.h>
#include <GL/GLSLProgram.h>
#include <GL/GLError.h>
#include <GL/GLTexture.h>
#include <string>
#include <cmath>

#include <Generative/PerlinNoise.h>
#include <Generative/NoiseShader.h>

using namespace GL;

//-----------------------------------------------------------------------------
//	class HelloNoise
//-----------------------------------------------------------------------------

class HelloNoise : public EngineGLUT
{
public:
	bool init( int argc, char* argv[] );
	void destroy();
	void render();
private:
	NoiseShader m_noiseshader;  // Perlin noise shader
	GLTexture   m_noisetex;     // Software noise texture 
	float m_x0,m_y0,m_z0,
	      m_x1,m_y1;
};

bool HelloNoise::init( int argc, char* argv[] )
{
	m_x0 = m_y0 = m_z0 = 0.f;
	m_x1 = m_y1 = 5.f;

	// shader solution

	if( !m_noiseshader.init() )
	{
		std::cerr << "Error: Initializing NoiseShader failed!" << std::endl;
		return false;
	}

	// reference software solution

	float data[512*512];
	for( int xi=0; xi<512; ++xi )
		for( int yi=0; yi<512; ++yi )
		{
			float x = m_x0 + (m_x1-m_x0) * (float)xi/511.f,
			      y = m_y0 + (m_y1-m_y0) * (float)yi/511.f;
			data[yi*512+xi] = PerlinNoise::ridgedmf( x,y,m_z0, 3 );
				//PerlinNoise::fBm( x,y,m_z0, 5 ) * 0.5 + 0.5;
				//PerlinNoise::turbulence( x,y,m_z0, 2 );
				//PerlinNoise::noise( x, y, m_z0 ) * 0.5 + 0.5;

			//data[yi*512+xi] = x/10.f;  // gradient test
		}

	m_noisetex.Create( GL_TEXTURE_2D );
	m_noisetex.Image(0, GL_RGB, 512,512,0,GL_LUMINANCE, GL_FLOAT, data );

	return true;
}

void HelloNoise::destroy()
{
	m_noiseshader.deinit();
	m_noisetex.Destroy(); 
}

void HelloNoise::render()
{
	glClearColor( 0.1f,0.23f,0.17f,1 );
	glClear( GL_COLOR_BUFFER_BIT );
	glEnable( GL_TEXTURE_2D );

	// --- shader solution ---

	m_noiseshader.bind();

	glBegin( GL_QUADS );
	// noise in second quadrant (top left)
	glTexCoord3f( m_x0, m_y0, m_z0 ); glVertex3f( -1, 0,0 );
	glTexCoord3f( m_x1, m_y0, m_z0 ); glVertex3f(  0, 0,0 );
	glTexCoord3f( m_x1, m_y1, m_z0 ); glVertex3f(  0, 1,0 );
	glTexCoord3f( m_x0, m_y1, m_z0 ); glVertex3f( -1, 1,0 );
	// debug texture in third quadrant (bottom left)
	glTexCoord3f( 0, 0,0); glVertex3f( -1,-1,0 );
	glTexCoord3f( 1, 0,0); glVertex3f(  0,-1,0 );
	glTexCoord3f( 1,-1,0); glVertex3f(  0, 0,0 );
	glTexCoord3f( 0,-1,0); glVertex3f( -1, 0,0 );
	glEnd();

	m_noiseshader.release();


	// --- reference software solution ---

	m_noisetex.Bind( 0 );

	glBegin( GL_QUADS );
	// reference in first quadrant (top right)
	glTexCoord3f( 0, 0,0); glVertex3f(  0, 0,0 );
	glTexCoord3f( 1, 0,0); glVertex3f(  1, 0,0 );
	glTexCoord3f( 1, 1,0); glVertex3f(  1, 1,0 );
	glTexCoord3f( 0, 1,0); glVertex3f(  0, 1,0 );
	glEnd();

	m_noisetex.Unbind();
}

int main( int argc, char* argv[] )
{
	using namespace std;
	
	HelloNoise hello;
	try
	{
		hello.set_window_title("Hello Noise!");
		hello.run( argc, argv );
	}
	catch( exception& e )
	{
		cerr << "Exception caught:" << endl << e.what() << endl;
	}	
	return 0; // never reached, put cleanup code into EngineGLUT::destroy()
}
