// Compare improved Noise shader vs. software implementation
// Max Hermann, August 03, 2010
//
// Remarks:
// - Looks now pretty the same for apertures greater than 15.0, but there
//   are still differences visible for smaller apertures like 5.0.
//
// TODO:
// - Optimize by using precomputed lookup tables (see GPUGems2)
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

using namespace GL;

//-----------------------------------------------------------------------------
//	class PerlinNoise
//-----------------------------------------------------------------------------

/// Perlin Noise
/// Java reference implementation adapted from http://mrl.nyu.edu/~perlin/noise/
class PerlinNoise
{
public:
	static unsigned char s_permutation[512];
	static float s_gradients[3*16];

	static float noise( float x, float y, float z )
	{
		static unsigned char* hash = s_permutation;

		// integer part for indexing hash table
		int X = (int)x & 255,
			Y = (int)y & 255,
			Z = (int)z & 255;
		// fractional part
		x -= (int)x;
		y -= (int)y;
		z -= (int)z;

		float u = fade(x),
			   v = fade(y),
			   w = fade(z);

		int A = hash[X  ]+Y,  AA = hash[A]+Z,  AB = hash[A+1]+Z,
			B = hash[X+1]+Y,  BA = hash[B]+Z,  BB = hash[B+1]+Z;

		return lerp(w, lerp(v, lerp(u, grad(hash[AA  ], x  , y  , z   ),
                                       grad(hash[BA  ], x-1, y  , z   )),
                               lerp(u, grad(hash[AB  ], x  , y-1, z   ),
                                       grad(hash[BB  ], x-1, y-1, z   ))),
                       lerp(v, lerp(u, grad(hash[AA+1], x  , y  , z-1 ),
                                       grad(hash[BA+1], x-1, y  , z-1 )),
                               lerp(u, grad(hash[AB+1], x  , y-1, z-1 ),
                                       grad(hash[BB+1], x-1, y-1, z-1 ))));
	}

	// turbulence (same as fBm w/ abs)
	static float turbulence( float x, float y, float z, 
	                         int octaves, float lacunarity=2.0, float gain=0.5)
	{
		float sum  = 0.0,
			  freq = 1.0,
			  amp  = 1.0;
		for( int i=0; i < octaves; ++i )
		{
			sum += fabs( noise(freq*x,freq*y,freq*z) )*amp;
			freq *= lacunarity;
			amp *= gain;
		}
		return sum;
	}

	// fractal sum (same as turbulence w/o abs)
	static float fBm( float x, float y, float z, 
	                  int octaves, float lacunarity=2.0, float gain=0.5 )
	{
		float sum  = 0.0,
			  freq = 1.0,
			  amp  = 1.0;
		for( int i=0; i < octaves; ++i )
		{
			sum += noise(freq*x,freq*y,freq*z)*amp;
			freq *= lacunarity;
			amp *= gain;
		}
		return sum;
	}

	// Ridged multifractal
	// See "Texturing & Modeling, A Procedural Approach", Chapter 12
	static float ridge( float h, float offset )
	{
		h = abs(h);
		h = offset - h;
		h = h * h;
		return h;
	}

	static float ridgedmf( float x, float y, float z, 
	                       int octaves, float lacunarity=2.0, float gain=0.5,
						   float offset=1.0 )
	{
		float sum  = 0,
		      freq = 1.0, 
			  amp  = 0.5,
		      prev = 1.0;
		for( int i=0; i < octaves; ++i ) 
		{
			float n = ridge(noise(freq*x,freq*y,freq*z), offset);
			sum += n*amp*prev;
			prev = n;
			freq *= lacunarity;
			amp *= gain;
		}
		return sum;
	}


	/// fade curve
	static float fade( float t )	
		{ return t*t*t * (6*t*t - 15*t + 10); }  // improved noise
		//{ return t*t * (3 - 2*t);              // classical noise
	/// linear interpolation
	static float lerp( float t, float a, float b ) { return a + t*(b-a); }
	/// dot product between gradient and fractional position	
	static float grad( int hash, float x, float y, float z )
	{
	  #if 0
		// tricky bit-fiddling gradient dot-product lookup
		int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
		double u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
			 v = h<4 ? y : h==12||h==14 ? x : z;
		return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
	  #else
		int h = hash & 15;
		static float* g = s_gradients;
		return g[3*h+0]*x + g[3*h+1]*y + g[3*h+2]*z;
	  #endif
	}
};

float PerlinNoise::s_gradients[3*16] = 
{
	1,1,0,   -1,1,0,   1,-1,0,   -1,-1,0,
	1,0,1,   -1,0,1,   1,0,-1,   -1,0,-1,
	0,1,1,   0,-1,1,   0,1,-1,   0,-1,-1,
	1,1,0,   0,-1,1,   -1,1,0,   0,-1,-1   // this line taken from GPUGems2
								           // (unclear why its not repeated in GPUGems2)
/*	
	1,1,0,   -1,1,0,   1,-1,0,   -1,-1,0   // repeat first 4 entries for 
	                                       // indexing using modulo arithmetic
*/
};

unsigned char PerlinNoise::s_permutation[512] = { 151,160,137,91,90,15,
   131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
   190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
   88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
   77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
   102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
   135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
   5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
   223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
   129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
   251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
                                                  151,160,137,91,90,15,
   131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
   190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
   88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
   77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
   102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
   135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
   5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
   223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
   129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
   251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
   };

//-----------------------------------------------------------------------------
//	class NoiseShader
//-----------------------------------------------------------------------------

class NoiseShader
{
	static std::string s_vshader, s_fshader;

public:
	NoiseShader();
	~NoiseShader();

	bool init();
	void deinit();

	void bind( GLuint unit_permtex=0, GLuint unit_gradtex=1 );
	void release();

private:
	GL::GLSLProgram *m_shader;
	GL::GLTexture    m_permtex,
		             m_gradtex;
	GLint m_loc_permtex,
	      m_loc_gradtex;
};

#define SHADERSTRING(S) #S

//"void main() { gl_Position = ftransform(); }";  // minimal vertex shader
//#version 120 \n
std::string NoiseShader::s_vshader = SHADERSTRING(
	varying vec3 pos;
	varying vec4 color;
	varying vec3 tc;
	void main(void)
	{
		tc = gl_MultiTexCoord0.xyz;
		pos = gl_Vertex.xyz;
		color = gl_Color;
		gl_Position = ftransform();
	}	
);

//"void main() { gl_FragColor = vec4(0,1,0,1); }"; // test fragment shader
//#version 120 \n
//  / 256.0 ).x * 256.0; }
std::string NoiseShader::s_fshader = SHADERSTRING(
	varying vec3 pos;
	varying vec4 color;
	varying vec3 tc;

	uniform sampler1D permtex;
	uniform sampler1D gradtex;

	vec3  fade( vec3 t )  { return t*t*t * (6.0*t*t - 15.0*t + 10.0); }
	float hash( float x ) { return texture1D( permtex, x ).x; }
	float grad( float x, vec3 p )
	{		
		x = x*16.0; /*same effect as: x = mod( x*256.0, 16.0 );*/
		return dot( texture1D(gradtex,x).xyz, p );
	}

	/* unoptimized 1:1 Perlin noise implementation */
	float inoise( vec3 p )
	{
		vec3 P = mod( floor(p), 256.0 );
		p -= floor(p);
		vec3 f = fade(p);

		P = P / 256.0;
		const float one = 1.0 / 256.0;

		float A = hash(P.x    )+P.y; float AA = hash(A)+P.z; float AB = hash(A+one)+P.z;
		float B = hash(P.x+one)+P.y; float BA = hash(B)+P.z; float BB = hash(B+one)+P.z;

		return mix( mix( mix( grad(hash(AA    ), p               ), 
		                      grad(hash(BA    ), p+vec3(-1.0, 0.0, 0.0)), f.x ),
		                 mix( grad(hash(AB    ), p+vec3( 0.0,-1.0, 0.0)),
						      grad(hash(BB    ), p+vec3(-1.0,-1.0, 0.0)), f.x ), f.y ),
		            mix( mix( grad(hash(AA+one), p+vec3( 0.0, 0.0,-1.0)), 
		                      grad(hash(BA+one), p+vec3(-1.0, 0.0,-1.0)), f.x ),
		                 mix( grad(hash(AB+one), p+vec3( 0.0,-1.0,-1.0)),
						      grad(hash(BB+one), p+vec3(-1.0,-1.0,-1.0)), f.x ), f.y ),
		            f.z );
	}

	float turbulence( vec3 p, int octaves, float lacunarity = 2.0, float gain = 0.5 )
	{
		float sum = 0.0;
		float freq = 1.0;
		float amp = 1.0;
		for( int i=0; i < octaves; ++i )
		{
			sum += abs(inoise(p*freq))*amp;
			freq *= lacunarity;
			amp *= gain;
		}
		return sum;
	}

	float fBm( vec3 p, int octaves, float lacunarity = 2.0, float gain = 0.5 )
	{
		float sum = 0.0;
		float freq = 1.0;
		float amp = 1.0;
		for( int i=0; i < octaves; ++i )
		{
			sum += inoise(p*freq)*amp;
			freq *= lacunarity;
			amp *= gain;
		}
		return sum;
	}

	// Ridged multifractal
	// See "Texturing & Modeling, A Procedural Approach", Chapter 12
	float ridge( float h, float offset )
	{
		h = abs(h);
		h = offset - h;
		h = h * h;
		return h;
	}

	float ridgedmf( vec3 p, int octaves, float lacunarity = 2.0, float gain = 0.5,
		            float offset = 1.0 )
	{
		float sum = 0.0;
		float freq = 1.0;
		float amp = 0.5;
		float prev = 1.0;
		for( int i=0; i < octaves; ++i )
		{
			float n = ridge( inoise(p*freq), offset );
			sum += n*amp*prev;
			prev = n;
			freq *= lacunarity;
			amp *= gain;
		}
		return sum;
	}

	void main()
	{
		float n = ridgedmf( tc, 3 );
			// fBm( tc, 5 )*0.5+0.5;
			// turbulence( tc, 2 );  
			// inoise( tc )*0.5 + 0.5;
		vec3 g = texture1D( gradtex, tc.x ).xyz;
		float c = texture1D( permtex, tc.x ).x;
		if( tc.y < -0.5 )
			gl_FragColor = vec4( c,c,c, 1.0 );
		else
		if( tc.y <  0.0 )
		{
			gl_FragColor = vec4( g*.5 + vec3(.5,.5,.5), 1.0 );			
		}
		else
			gl_FragColor = vec4( n,n,n, 1.0 );
	}
);
/*
	Following code fails with SHADERSTRING macro!
		float A = hash(P.x  )+P.y,  AA = hash(A)+P.z,  AB = hash(A+1)+P.z,
			  B = hash(P.x+1)+P.y,  BA = hash(B)+P.z,  BB = hash(B+1)+P.z;
*/

NoiseShader::NoiseShader()
: m_shader(NULL) 
{}

NoiseShader::~NoiseShader()
{
	deinit();
	
	// FIXME: When to destroy GLTexture objects?
}

void NoiseShader::bind( GLuint unit_permtex, GLuint unit_gradtex )
{
	if( !m_shader ) return;
#if 1
	m_permtex.Bind( unit_permtex );
	m_gradtex.Bind( unit_gradtex );

	m_shader->bind();
	glUniform1i( m_loc_permtex, unit_permtex );
	glUniform1i( m_loc_gradtex, unit_gradtex );
#else
	m_shader->bind();
#endif
}

void NoiseShader::release()
{
	if( !m_shader) return;
	m_shader->release();
}

void NoiseShader::deinit()
{
	if( m_shader) delete m_shader; m_shader=NULL;
}

bool NoiseShader::init()
{
	using namespace std;

	// release previous shader
	if( m_shader ) deinit();

	// create shader
	m_shader = new GLSLProgram;
	if( !m_shader )
	{
		cerr << "Error: Creation of NoiseShader GLSL shader failed!" << endl;
		return false;
	}

	// load shader
	if( !m_shader->load( s_vshader, s_fshader ) )
	{
		cerr << "Error: Loading NoiseShader failed!" << endl;
		return false;
	}

	// get uniform locations
	m_loc_permtex = m_shader->getUniformLocation( "permtex" );
	m_loc_gradtex = m_shader->getUniformLocation( "gradtex" );

	// create textures
	if( !m_permtex.Create(GL_TEXTURE_1D) || !m_gradtex.Create(GL_TEXTURE_1D) )
	{
		cerr << "Error: Creation of 2D textures failed!" << endl;
		return false;
	}
	unsigned char* perm_ptr = PerlinNoise::s_permutation;
	float* grad_ptr = PerlinNoise::s_gradients;

	// rescale permutation entries from [0,255] to [0.f,1.f]
	float perm_data[256];
	for( int i=0; i < 256; ++i ) perm_data[i] = perm_ptr[i]/255.f;
	float grad_data[16*3];
	for( int i=0; i < 16*3;++i ) grad_data[i] = grad_ptr[i];

	// 8bit luminance texture sufficient for unsigned char permutation table
	m_permtex.Image( 0, GL_LUMINANCE, 256, 0, GL_LUMINANCE, GL_FLOAT, perm_data );
	// gradients stored for simplicity in real float texture
	m_gradtex.Image( 0, GL_RGBA32F, 16, 0, GL_RGB, GL_FLOAT, grad_data );
	m_permtex.SetWrapMode( GL_REPEAT );
	m_permtex.SetFilterMode( GL_NEAREST );
	m_gradtex.SetWrapMode( GL_REPEAT );
	m_gradtex.SetFilterMode( GL_NEAREST );

	return checkGLError( "NoiseShader::init()" );
}


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
	glClearColor( 0.1,0.23,0.17,1 );
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
