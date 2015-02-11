#include "ShaderModule.h"
#include "ShaderPrecompiler.h"
#include <glutils/GLError.h>
#include <iostream>
#include <ctime>
#include <cstdlib> // for atof()
using std::cerr;
using std::endl;
using std::string;
#ifdef GL_NAMESPACE
using GL::checkGLError;
#endif

//----------------------------------------------------------------------------
//  Factory registration
//----------------------------------------------------------------------------
#include "ModuleFactory.h"
MODULEFACTORY_REGISTER( ShaderModule, "ShaderModule" )

//----------------------------------------------------------------------------
// Default GLSL 120 / GLSL ES 2.0 shader 
//----------------------------------------------------------------------------
const std::string defaultUniforms =
"uniform vec3      iResolution;\n"           // viewport resolution (in pixels)
"uniform float     iGlobalTime;\n"           // shader playback time (in seconds)
"\n";
/* shadertoy uniforms
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform float     iChannelTime[4];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform samplerXX iChannel0..3;          // input channel. XX = 2D/Cube
uniform vec4      iDate;                 // (year, month, day, time in seconds)
*/

const std::string defaultVertexShader =
defaultUniforms + 
"void main(void)\n"
"{\n"
//"    gl_TexCoord[0] = gl_MultiTexCoord0;\n"; // required?
"    gl_Position = gl_Vertex;\n"
"}\n";

const std::string defaultFragmentShader =
defaultUniforms + 
"void main(void)\n"
"{\n"
"	vec2 uv = gl_FragCoord.xy / iResolution.xy;\n"
"	gl_FragColor = vec4(uv,0.5+0.5*sin(iGlobalTime),1.0);\n"
"}";


//=============================================================================
//  ShaderModule implementation
//=============================================================================

//----------------------------------------------------------------------------
ShaderModule::ShaderModule()
: ModuleRenderer( "ShaderModule" ),
  m_initialized(false),
  m_target_initialized(false),
  m_r2t_initialized   (false),
  m_shader_initialized(false),
  m_shader(0),
  m_vshader( defaultVertexShader ),
  m_fshader( defaultFragmentShader ),
  m_channels(4,-1)
{
	options().push_back( &m_opts.width  );
	options().push_back( &m_opts.height );
	m_superOptions = options();
	m_superParameters = parameters();

	setChannelResolution( 0, 512,512 );
	setChannelResolution( 1, 512,512 );
	setChannelResolution( 2, 512,512 );
	setChannelResolution( 3, 512,512 );
}

//----------------------------------------------------------------------------
bool ShaderModule::init()
{
	// Create texture
	if( !m_target_initialized )
	{
		// Create texture id only once!
		if( !m_target.create(GL_TEXTURE_2D) )
		{
			cerr << "ShaderModule::init() : Couldn't create 2D textures!" << endl;
			return false;
		}
		m_target_initialized = true;
	}
	
	// Note on texture format:
	// 8-bit per channel resolution leads to quantization artifacts so we
	// currently use 12-bit per channel which should be hardware supported.
	// On newer hardware it should be safe to set GL_RGBA32F.
	GLint internalFormat = GL_RGBA32F; //GL_RGB12;

	// Allocate GPU mem
	m_target.image( 0, internalFormat, 
	                m_opts.width.value(), m_opts.height.value(), 
	                0, GL_RGBA, GL_FLOAT, NULL );
	
	// Setup Render-2-Texture
	if( !m_r2t_initialized )
	{
		// Since we are using no depth-buffer attachement, r2t has not be
		// setup again on size change. 
		if( !m_r2t.init_no_depthbuffer( m_target.name() ) )
		{
			cerr << "ShaderModule::init() : Couldn't setup render-to-texture!" << endl;
			return false;
		}
		m_r2t_initialized = true;
	}
	
	// Create shader
	if( !m_shader_initialized )
	{
		// Initialize shader
		if( m_shader ) delete m_shader; m_shader=0;
		m_shader = new GLSLProgram();
		if( !m_shader )
		{
			cerr << "ShaderModule::init() : Creation of GLSL shader failed!" << endl;
			return false;
		}
		m_shader_initialized = true;
	}	

	// Compile shader
	if( m_shader )
		if( !compile() )
			// On this call we currently require the shader to compile correctly!
			return false;

	// If the end of init() is reached once, we consider the module initialized
	m_initialized = checkGLError( "ShaderModule::init() : GL error at exit!" );
	return m_initialized;
}

//----------------------------------------------------------------------------
void ShaderModule::destroy()
{
	delete m_shader; m_shader = 0;
	m_r2t.deinit();
	m_target.destroy();
}

//----------------------------------------------------------------------------
bool ShaderModule::compile()
{
	// Preprocessing is applied on each compilation!
	string fshaderProcessed = preprocess(m_fshader);
	return compile( m_vshader, fshaderProcessed );
}

//----------------------------------------------------------------------------
bool ShaderModule::compile( std::string vshader, std::string fshader )
{
	if( !m_shader ) return false;
	if( !m_shader->load( vshader, fshader ) )
	{
		cerr << "ShaderModule::compile() : Compilation of shaders failed!" << endl;
		return false;
	}	
	return checkGLError( "ShaderModule::compile()" );
}

//----------------------------------------------------------------------------
bool ShaderModule::loadShader( const char* filename )
{
	char* fshader_src = GL::GLSLProgram::read_shader_from_disk( filename );
	if( !fshader_src ) 
		return false;

	std::string fshader( fshader_src );
	if( !compile( m_vshader, fshader ) )
		return false;

	// Replace current fragment shader on succesfull compilation
	m_fshader = fshader;
	// Compile again, this time with preprocessing!
	compile();
	return checkGLError( "ShaderModule::loadShader() : GL error at exit!" );
}

//----------------------------------------------------------------------------
#include <fstream>
bool ShaderModule::saveShader( const char* filename ) const
{
	std::ofstream f(filename);
	if( f.good() )
	{
		f << m_fshader;
		f.close();
	}
	else
	{
		std::cerr << "ShaderModule::saveShader() : "
			<< "Could not save shader source to \"" << filename << "\"!" << std::endl;
		return false;
	}
	return true;
}

//----------------------------------------------------------------------------
void ShaderModule::render()
{
	// Initialized on first render() call; by then we should have a valid 
	// OpenGL context.
	if( !m_initialized && !init() ) return;

	if( !m_shader ) return;
	
	// Time is measured w.r.t. to first render() call
	static clock_t t0 = clock();	
	
	// Bind shader and set default uniforms
	m_shader->bind();
	checkGLError( "ShaderModule::render() - After shader bind" );

	int width  = m_opts.width .value(),
		height = m_opts.height.value();

	GLint 
		iResolution = m_shader->getUniformLocation("iResolution"),
		iGlobalTime = m_shader->getUniformLocation("iGlobalTime"),
		iChannel0   = m_shader->getUniformLocation("iChannel0"),
		iChannel1   = m_shader->getUniformLocation("iChannel1"),
		iChannel2   = m_shader->getUniformLocation("iChannel2"),
		iChannel3   = m_shader->getUniformLocation("iChannel3"),
		iChannelResolution = m_shader->getUniformLocation("iChannelResolution");
	if( iResolution >= 0 )
	{
		GLfloat res[3]; 
		res[0] = (GLfloat)width; 
		res[1] = (GLfloat)height; 
		res[2] = (GLfloat)1.f;
		glUniform3f( m_shader->getUniformLocation( "iResolution" ), 
			(GLfloat)width, (GLfloat)height, (GLfloat)1.f );
		checkGLError( "ShaderModule::render() - After glUniform3f()" );
	}
	if( iGlobalTime >= 0 )
	{	
		float time = (float)(clock() - t0) / CLOCKS_PER_SEC;
		glUniform1f( m_shader->getUniformLocation( "iGlobalTime" ), time );
		checkGLError( "ShaderModule::render() - After glUniform1f()" );
	}
	if( iChannel0 >= 0 ) glUniform1i( iChannel0, 0 );
	if( iChannel1 >= 0 ) glUniform1i( iChannel1, 1 );
	if( iChannel2 >= 0 ) glUniform1i( iChannel2, 2 );
	if( iChannel3 >= 0 ) glUniform1i( iChannel3, 3 );
	if( iChannelResolution >= 0 )
	{
		glUniform3fv( iChannelResolution, 4, m_channelResolution );
	}
	checkGLError( "ShaderModule::render() - After shader uniform setup" );

	// Set custom uniforms (found and defined in preprocessing)	
	for( int i=0; i < m_uniformParams.floats.size(); i++ )
	{
		GLint loc = m_shader->getUniformLocation( m_uniformParams.floats[i].key().c_str() );
		if( loc >= 0 )
			glUniform1f( loc, (float)m_uniformParams.floats[i].value() );
	}
	checkGLError( "ShaderModule::render() - After shader custom uniform setup" );

	// Bind textures
	for( int i=0; i < 4; i++ )
	{
		if( m_channels[i] >= 0 )
		{
			glActiveTexture( GL_TEXTURE0 + i );
			glBindTexture( GL_TEXTURE_2D, m_channels[i] );
		}
	}
	glActiveTexture( GL_TEXTURE0 );
	checkGLError( "ShaderModule::render() - After setting texture channels" );
	
	if( m_r2t.bind( m_target.name() ) )
	{
		// Render target resolution sized quad

		int viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );
		glViewport( 0,0, width,height );

		// Set transparent background
		glClearColor( 0.f, 0.f, 0.f, 0.f );
		glClear( GL_COLOR_BUFFER_BIT );
		
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
		
		glBegin(GL_QUADS );
		glTexCoord2f( 0, 0 );	glVertex3i(-1, -1, 0);			
		glTexCoord2f( 1, 0 );	glVertex3i(1, -1, 0);
		glTexCoord2f( 1, 1 );	glVertex3i(1, 1, 0);
		glTexCoord2f( 0, 1 );	glVertex3i(-1, 1, 0);
		glEnd();

		glViewport( viewport[0], viewport[1], viewport[2], viewport[3] );
	}
	else
	{
		cerr << "ShaderModule::render() : Could not bind framebuffer!" << endl;
	}

	// Unbind textures
	for( int i=0; i < 4; i++ )
	{
		glActiveTexture( GL_TEXTURE0 + i );
		glBindTexture( GL_TEXTURE_2D, 0 ); // unbind
	}
	glActiveTexture( GL_TEXTURE0 );
	
	m_shader->release();
	
	m_r2t.unbind();
}

//-----------------------------------------------------------------------------
Serializable::PropertyTree& ShaderModule::serialize() const
{
	static Serializable::PropertyTree cache;
	cache = Super::serialize();	

	cache.put("ShaderModule.FragmentShader",m_fshader);

	return cache;
}

//-----------------------------------------------------------------------------
void ShaderModule::deserialize( Serializable::PropertyTree& pt )
{
	Super::deserialize( pt );

	std::string fshader = pt.get( "ShaderModule.FragmentShader", "" );
	if( !fshader.empty() )
		m_fshader = fshader;
	else
		cerr << "ShaderModule::deserialize : No fragment shader found!" << endl;

	compile();
}

//-----------------------------------------------------------------------------
bool ShaderModule::setShaderSource( const std::string& shader )
{
	m_fshader = shader;
	return compile();
}

//-----------------------------------------------------------------------------
void ShaderModule::setChannelResolution( int idx, int w, int h, int d )
{
	assert( idx>=0 && idx < 4 );
	m_channelResolution[3*idx  ] = (GLfloat)w;
	m_channelResolution[3*idx+1] = (GLfloat)h;
	m_channelResolution[3*idx+2] = (GLfloat)d;

#ifdef _DEBUG
	// DEBUG
	std::cout << "ShaderModule " << this->getName() << " channel " << idx 
		<< " set resolution (" << w << "," << h << "," << d << ")" << std::endl;
#endif
}

//-----------------------------------------------------------------------------
void ShaderModule::setChannel( int idx, ModuleRenderer* m )
{
	assert(m);
	setChannel( idx, m->target() );

	// Try to extract width/height from options
	IntParameter 
	*pw = dynamic_cast<IntParameter*>( m->options().get_param("targetWidth") ),
	*ph = dynamic_cast<IntParameter*>( m->options().get_param("targetHeight") );
	if( pw && ph ) 
	{
		setChannelResolution( idx, pw->value(), ph->value() );
	}
	else
	{
		// Set some default resolution
		setChannelResolution( idx, 512, 512 );
	}	
}

//-----------------------------------------------------------------------------
void ShaderModule::setChannel( int idx, int texId ) 
{ 
	if(idx>=0&&idx<m_channels.size()) 
		m_channels[idx]=texId;  
}

//-----------------------------------------------------------------------------
int ShaderModule::channel( int idx ) const 
{ 
	if(idx>=0&&idx<m_channels.size()) 
		return m_channels[idx];
	return -1; 
}

//-----------------------------------------------------------------------------
int ShaderModule::numChannels() const 
{ 
	return (int)m_channels.size(); 
}

//----------------------------------------------------------------------------
std::string ShaderModule::preprocess( std::string shader )
{	
	using namespace std;

	ShaderPrecompiler pc;

	// Only treat variables and not defines
	string shaderProcessed = pc.precompile( shader );
	ShaderPrecompiler::ShaderVariables& vars = pc.vars();

#if 1
	// Precompile twice - in 1st run defines are parsed and set in the 2nd round
	ShaderPrecompiler::ShaderDefines defs = pc.defs();

	// Collect enum defines
	bool defValueChanged = false;
	vector<EnumParameter> enums;
	for( int i=0; i < defs.size(); i++ )
	{
		string key = defs[i].name;

		// Get enum names
		vector<string> enumNames = defs[i].values;
		if( enumNames.empty() )
		{
			// Set some default enum names
			enumNames.push_back("Default0");
			enumNames.push_back("Default1");
		}

		// Create enum parameter
		EnumParameter p( key, 
			!defs[i].value.empty() ? atoi(defs[i].value.c_str()) : 0, // value & default
			enumNames );

		// Get existing value from options (if available)
		EnumParameter* cur = dynamic_cast<EnumParameter*>( options().get_param( key ) );
		if( cur )
		{
			p.setValue( cur->value() );
			// Set also ShaderDefine value for 2nd precompilation pass
			stringstream ss; ss << cur->value();
			defs[i].value = ss.str();
			defValueChanged = true;
		}

		enums.push_back( p );
	}
	
	// Second precompilation only necessary on define value change
	if( defValueChanged )
		shaderProcessed = pc.precompile( shader, defs );

	// Update options
	m_defineOpts.enums = enums;
	options() = m_superOptions; // Reset module options
	for( int i=0; i < enums.size(); i++ ) // Add define enums
		options().push_back( &m_defineOpts.enums[i] );
#endif
	
	// Collect 'float', 'int' and 'bool' parameters
	vector<DoubleParameter> floats;	
	vector<IntParameter>    ints;
	vector<BoolParameter>   bools; 
	for( int i=0; i < vars.size(); i++ )
	{
		string key = vars[i].name;
		string type = vars[i].type;

		if( type.compare("float")==0 )
		{		
			DoubleParameter p( key );		

			// Set default value (if specified)
			p.setValueAndDefault( !vars[i].value.empty() ? atof(vars[i].value.c_str()) : 1.0 );

			// Get value from existing 'float' (if available)
			DoubleParameter* cur = dynamic_cast<DoubleParameter*>( parameters().get_param( key ) );
			if( cur )
				p.setValue( cur->value() );
		
			floats.push_back( p );
		}
		else
		if( type.compare("int")==0 )
		{
			IntParameter p( key );
			p.setValueAndDefault( !vars[i].value.empty() ? atoi(vars[i].value.c_str()) : 0 );
			IntParameter* cur = dynamic_cast<IntParameter*>( parameters().get_param( key ) );
			if( cur )
				p.setValue( cur->value() );
			ints.push_back( p );
		}
		else
		if( type.compare("bool")==0 )
		{
			BoolParameter p( key );
			p.setValueAndDefault( !vars[i].value.empty() ? atoi(vars[i].value.c_str()) : true );
			BoolParameter* cur = dynamic_cast<BoolParameter*>( parameters().get_param( key ) );
			if( cur )
				p.setValue( cur->value() );
			bools.push_back( p );
		}
		else
		{
			cerr << "ShaderModule::preprocess() : Unsupported parameter "
				"type \"" << type << "\"!" << endl;
		}
	}	
	
	// FIXME: Parameters should not be accessed after changing their instances!
	m_uniformParams.floats = floats;
	m_uniformParams.ints   = ints;
	m_uniformParams.bools  = bools;
	
	// Update parameter list 
	parameters() = m_superParameters; // Reset to inherited state
	for( int i=0; i < m_uniformParams.floats.size(); ++i )
		parameters().push_back( &m_uniformParams.floats[i] );
	for( int i=0; i < m_uniformParams.ints.size(); ++i )
		parameters().push_back( &m_uniformParams.ints[i] );
	for( int i=0; i < m_uniformParams.bools.size(); ++i )
		parameters().push_back( &m_uniformParams.bools[i] );

	return shaderProcessed;
}
