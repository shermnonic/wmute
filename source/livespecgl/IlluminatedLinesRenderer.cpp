#include "IlluminatedLinesRenderer.h"
#include <glutils\Image.h>

// Helper function to load a texture image from disk
int loadTexture( GL::GLTexture& tex, const char* filename, int type=GL_UNSIGNED_BYTE )
{
	using namespace std;

	// Load image
	Image* img =
	        ImageFactory::ref().create_image(Image::get_extension( filename ));
	if( !img )
	{
		cerr << "Error: Format of '" << filename << "' not supported!" << endl;
		return -1;
	}
	if( !img->load( filename ) )
	{
		cerr << "Error: Couldn't load '" << filename << "'!" << endl;
		return -2;
	}

	// Create texture	
	if( !tex.Create() )
	{
		cerr << "Error: Couldn't create texture!" << endl;
		return -3;
	}
	
	// Texture format
	GLint  internalFormat;
	GLenum format;
	switch( img->channels() )
	{
	case 4: internalFormat = GL_RGBA ; format = GL_RGBA ; break;
	case 3: internalFormat = GL_RGB  ; format = GL_RGB  ; break;
	case 1: internalFormat = GL_ALPHA; format = GL_ALPHA; break;
	default:
		cerr << "Error: Image has " << img->channels() << " channels!" << endl;
		return -4;
	}

	// Create empty power-of-two texture
	if( !tex.Image( 0, internalFormat, img->width(), img->height(), 0,
	             format, GL_UNSIGNED_BYTE, NULL ) )
	{
		cerr << "Error: Couldn't allocate texture!" << endl;
		return -5;
	}

	// Download image data to texture	
	if( !tex.SubImage( 0,0,0, img->width(), img->height(), format, type,
	                   img->ptr() ) )
	{
		cerr << "Error: Couldn't download image to texture!" << endl;
		return -6;
	}

	// Free image
	delete img;

	return 0;
}


//=============================================================================
//	TangentColorShader
//=============================================================================

#define SHADERSTRING(S) #S

#if 1 // Test shaders
std::string TangentColorShader::s_vshader = 
//  "void main() { gl_Position = ftransform(); }";   // minimal vertex shader
	"varying vec3 pos;                                                 \n"
	"varying float LT;                                                 \n"
	"varying float VT;                                                 \n"
	"varying vec3 T;                                                   \n"
	"                                                                  \n"
	"void main()                                                       \n"
	"{                                                                 \n"
	"  pos = gl_Vertex.xyz;                                            \n"
	"  gl_Position = ftransform();                                     \n"
	"                                                                  \n"
	"  vec3 lightpos = vec3(-1,1,1);                                   \n"
	"                                                                  \n"
	"       T = normalize( gl_Normal.xyz );                            \n"
	"  vec3 L = normalize( lightpos - pos );                           \n"
	"  vec3 V = normalize( -pos );                                     \n"
	"  LT = dot(L,T);                                                  \n"
	"  VT = dot(V,T);                                                  \n"
	"}                                                                 \n"
	"                                                                  \n"
	"                                                                  \n";
std::string TangentColorShader::s_fshader = 
//  "void main() { gl_FragColor = vec4(0,1,0,1); }"; // minimal fragment shader
	"varying vec3 pos;                                                 \n"
	"varying float LT;                                                 \n"
	"varying float VT;                                                 \n"
	"varying vec3 T;                                                   \n"
	"                                                                  \n"
	"void main()                                                       \n"
	"{                                                                 \n"
	"  gl_FragDepth = gl_FragCoord.z;                                  \n"
	"  gl_FragColor = vec4(.3,.3,.3,0) + vec4(T,0.3);                  \n"
	"}                                                                 \n"
	"                                                                  \n"
	"                                                                  \n";

#else
//#version 120 \n
std::string IlluminatedLinesShader::s_vshader = SHADERSTRING(
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
std::string IlluminatedLinesShader::s_fshader = SHADERSTRING(
	varying vec3 pos;
	varying vec4 color;
	varying vec3 tc;
	void main() 
	{ 
		gl_FragColor = vec4(0,1,0,1); 
	};
);
#endif

void TangentColorShader::bind( GLuint shadingTex )
{
	if( !m_shader ) return;
	m_shader->bind();
}

void TangentColorShader::release()
{
	if( !m_shader) return;
	m_shader->release();
}

void TangentColorShader::deinit()
{
	if( m_shader) delete m_shader; m_shader=NULL;
}

bool TangentColorShader::init()
{
	using namespace std;

	// release previous shader
	if( m_shader ) deinit();

	// create shader
	m_shader = new GL::GLSLProgram;
	if( !m_shader )
	{
		cerr << "Error: Creation of TangentColorShader GLSL shader failed!" << endl;
		return false;
	}

	// load shader
	if( !m_shader->load( s_vshader, s_fshader ) )
	{
		cerr << "Error: Loading TangentColorShader failed!" << endl;
		return false;
	}

	cout << "TangentColorShader compiled and linked just fine\n";

	return GL::checkGLError( "TangentColorShader::init()" );
}


//=============================================================================
//	IlluminatedLinesShader
//=============================================================================

std::string IlluminatedLinesShader::s_vshader = 
//  "void main() { gl_Position = ftransform(); }";   // minimal vertex shader
	"varying vec3 pos;                                                 \n"
	"varying vec3 col;                                                 \n"
	"varying float LT;                                                 \n"
	"varying float VT;                                                 \n"
	"varying vec3 T;                                                   \n"
	"                                                                  \n"
	"void main()                                                       \n"
	"{                                                                 \n"
	"  col = gl_Color.rgba;                                            \n"
	"  pos = gl_Vertex.xyz;                                            \n"
	"  gl_Position = ftransform();                                     \n"
	"                                                                  \n"
	"  vec3 lightpos = vec3(-1,1,1);                                   \n"
	"                                                                  \n"
	"       T = normalize( gl_Normal.xyz );                            \n"
	"  vec3 L = normalize( lightpos - pos );                           \n"
	"  vec3 V = normalize( -pos );                                     \n"
	"  LT = dot(L,T);                                                  \n"
	"  VT = dot(V,T);                                                  \n"
	"}                                                                 \n"
	"                                                                  \n";
std::string IlluminatedLinesShader::s_fshader = 
//  "void main() { gl_FragColor = vec4(0,1,0,1); }"; // minimal fragment shader
	"varying vec3 pos;                                                 \n"
	"varying vec4 col;                                                 \n"
	"varying float LT;                                                 \n"
	"varying float VT;                                                 \n"
	"varying vec3 T;                                                   \n"
	"                                                                  \n"
	"uniform sampler2D shadtex;                                        \n"
	"                                                                  \n"
	"void main()                                                       \n"
	"{                                                                 \n"
	"  vec2 tc;                                                        \n"
	"  tc.x = 0.5*(clamp(LT,-1.0,1.0) + 1.0);                          \n"
	"  tc.y = 0.5*(clamp(VT,-1.0,1.0) + 1.0);                          \n"
	"  vec3 phong = texture2D(shadtex,tc);                             \n"
	"                                                                  \n"
	"  gl_FragDepth = gl_FragCoord.z;                                  \n"
	//"  gl_FragColor = .2*col + vec4(phong,.9); \n" //.x*col;     \n"
	//"  gl_FragColor = .3*col + phong.x*col; \n" //.x*col;     \n"
	"  gl_FragColor = vec4(.2,.2,.2,1) + phong.x*col;     \n"
	"}                                                                 \n"
	"                                                                  \n";

IlluminatedLinesShader::IlluminatedLinesShader()
: m_shader(NULL) 
{}

IlluminatedLinesShader::~IlluminatedLinesShader()
{
	deinit();
	
	// FIXME: When to destroy GLTexture objects?
}

void IlluminatedLinesShader::bind( GLuint shadingTex )
{
	if( !m_shader ) return;
#if 1
	int unit_shadtex = 0;
	m_shadtex.Bind( unit_shadtex );
	m_shader->bind();
	glUniform1i( m_loc_shadtex, unit_shadtex );
#else
	m_shader->bind();
#endif
}

void IlluminatedLinesShader::release()
{
	if( !m_shader) return;
	m_shader->release();
}

void IlluminatedLinesShader::deinit()
{
	if( m_shader) delete m_shader; m_shader=NULL;
}

bool IlluminatedLinesShader::init()
{
	using namespace std;

	// release previous shader
	if( m_shader ) deinit();

	// create shader
	m_shader = new GL::GLSLProgram;
	if( !m_shader )
	{
		cerr << "Error: Creation of IlluminatedLinesShader GLSL shader failed!" << endl;
		return false;
	}

	// load shader
	if( !m_shader->load( s_vshader, s_fshader ) )
	{
		cerr << "Error: Loading IlluminatedLinesShader failed!" << endl;
		return false;
	}

	cout << "IlluminatedLinesShader compiled and linked just fine\n";


	// setup shading texture
	m_loc_shadtex = m_shader->getUniformLocation( "shadtex" );

	int err = loadTexture( m_shadtex, "G:/Wintermute/wmute/trunk/data/textures/shading_LTVT.tga" );
	if( err < 0 )
		return false;

	return GL::checkGLError( "IlluminatedLinesShader::init()" );
}


//=============================================================================
//	IlluminatedLinesRenderer
//=============================================================================

bool IlluminatedLinesRenderer::init()
{
	if( !m_shader.init() )
	{		
		return false;
	}
	return true;
}

void IlluminatedLinesRenderer::destroy()
{
	m_shader.deinit();
}

void IlluminatedLinesRenderer::render()
{
	glClearColor( 0.1,0.23,0.17,1 );
	glClear( GL_COLOR_BUFFER_BIT );
}
