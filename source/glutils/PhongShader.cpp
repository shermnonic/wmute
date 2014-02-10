#include "PhongShader.h"

PhongShader::PhongShader()
: m_program(NULL)
{	
}

bool PhongShader::init()
{
	if( m_program )
		delete m_program;
	m_program = NULL;

	m_program = new GL::GLSLProgram;

#if 1 // LOAD SHADERS FROM DISK
	if( !m_program->load_from_disk( "shaders/phong.vs", "shaders/phong.fs" ) )
#else // USE BUILT-IN SHADERS
	if( !m_program->load( s_vertexShader, s_fragmentShader ) )
#endif
	{
		std::cerr << "PhongShader: Failed to load GLSL program!" << std::endl;
		return false;
	}

	return true;
}

void PhongShader::bind()
{
	if( m_program ) m_program->bind();
}

void PhongShader::release()
{
	if( m_program ) m_program->release();
}

void PhongShader::setDefaultLighting()
{
	GLenum light = GL_LIGHT0;
	
	glColor4f( 1,1,1,1 );
	glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
	glEnable( GL_COLOR_MATERIAL );
	
	glLightModelf (GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0);
	glLightModelf (GL_LIGHT_MODEL_TWO_SIDE    , 0.0);
	
	// Default ambience
	
	GLfloat lmKa[] = {0.2, 0.2, 0.2, 1.0 };
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, lmKa );	
	
	// Default spotlight

	GLfloat spot_direction[] = {.0, .0, -1.0 };
	GLint spot_exponent =   0;   
	GLint spot_cutoff   = 180;

	glLightfv( light, GL_SPOT_DIRECTION, spot_direction);
	glLighti ( light, GL_SPOT_EXPONENT , spot_exponent );
	glLighti ( light, GL_SPOT_CUTOFF   , spot_cutoff   );
	
	glLightf( light, GL_CONSTANT_ATTENUATION , 1.0 );
	glLightf( light, GL_LINEAR_ATTENUATION   ,  .0 );
	glLightf( light, GL_QUADRATIC_ATTENUATION,  .0 );
		
	// Light
	
	GLfloat light_ambient[]  = {  .0,  .0,  .0, 1.0 };
	GLfloat light_diffuse[]  = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_position[] = {  .0,  .0, -1.0 };  
	
	glLightfv( light, GL_AMBIENT,  light_ambient  );
	glLightfv( light, GL_DIFFUSE,  light_diffuse  );
	glLightfv( light, GL_SPECULAR, light_specular );
	glLightfv( light, GL_POSITION, light_position );

	// Default material parameters
	
	GLfloat material_Ka[] = {0.2, 0.2, 0.2, 1.0};
	GLfloat material_Kd[] = {0.8, 0.8, 0.8, 1.0};
	GLfloat material_Ks[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat material_Ke[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat material_Se = 0.1;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT  , material_Ka);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE  , material_Kd);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR , material_Ks);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION , material_Ke);
	glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, material_Se);		
}

//------------------------------------------------------------------------------
// 	Shader
//------------------------------------------------------------------------------
#define GLUTILS_SHADER_STRING(S) #S;

// REMARK:
// Per-pixel lighting works correctly if all OpenGL materials and lighting
// parameters are fully specified. But if GL_COLOR_MATERIAL is enabled, the
// current version of the shader doesn't produce correct result.
// Observation: 
// 	- For SHININESS==0 we get "black border" on default lit shapes.

// Per-pixel lighting (vertex shader)
std::string PhongShader::s_vertexShader = GLUTILS_SHADER_STRING
(
	varying vec3 N;
	varying vec3 v;

	void main(void)
	{ 
		// v needed in fragment shader for lighting calculation.
		// Normalize and multiplication by NormalMatrix are required to achieve
		// equivalent results to OpenGL fixed pipeline.
		v = vec3(gl_ModelViewMatrix * gl_Vertex);   
		N = normalize(gl_NormalMatrix * gl_Normal);		
		
		gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	}	
)

// Per-pixel lighting (fragment shader)
std::string PhongShader::s_fragmentShader = GLUTILS_SHADER_STRING
(
	varying vec3 N;
	varying vec3 v;

	void main(void)
	{		
		vec3 L = normalize(gl_LightSource[0].position.xyz - v);
		vec3 E = normalize(-v);
		vec3 R = normalize(-reflect(L,N));
		
		vec4 Iamb = gl_FrontLightProduct[0].ambient;
			
		vec4 Idiff = gl_FrontLightProduct[0].diffuse * max(dot(N,L), 0.0);     
		Idiff = clamp(Idiff, 0.0, 1.0);
		
		vec4 Ispec = gl_FrontLightProduct[0].specular
		  * pow( max(dot(R,E),0.0), 0.3 * gl_FrontMaterial.shininess );
		Ispec = clamp(Ispec, 0.0, 1.0);

		gl_FragColor = gl_FrontLightModelProduct.sceneColor 
					 + Iamb + Idiff + Ispec;
	}	
)
