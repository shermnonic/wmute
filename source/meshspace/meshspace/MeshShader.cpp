#include "MeshShader.h"

MeshShader::MeshShader()
: m_program(NULL)
{	
}

bool MeshShader::init()
{
	// Create color lookup texture
	m_tf.create();

	// Create shader
	if( m_program )
		delete m_program;
	m_program = NULL;

	m_program = new GL::GLSLProgram;

#if 1 // LOAD SHADERS FROM DISK
	if( !m_program->load_from_disk( "shaders/mesh.vs", "shaders/mesh.fs" ) )
#else // USE BUILT-IN SHADERS
	if( !m_program->load( s_vertexShader, s_fragmentShader ) )
#endif
	{
		std::cerr << "MeshShader: Failed to load GLSL program!" << std::endl;
		return false;
	}

	return true;
}

void MeshShader::destroy()
{
	m_tf.destroy();
	delete m_program; m_program = NULL;
}

void MeshShader::bind()
{
	if( !m_program ) return;

	m_tf.bind( 0 );
	glUniform1i( m_program->getUniformLocation("lookup"), 0 );
	m_program->bind();
}

void MeshShader::release()
{
	if( !m_program ) return;	

	m_program->release();
	m_tf.release();
}

void MeshShader::setDefaultLighting()
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
