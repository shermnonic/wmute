// Max Hermann, March 22, 2010
#include "RaycastShader.h"
#include <string>
#include <sstream>
#include <cassert>

using namespace GL;
using namespace std;

//------------------------------------------------------------------------------
bool RaycastShader::init()
{
	// release previous shader
	if( m_shader ) deinit();

	// create shader
	m_shader = new GLSLProgram;
	if( !m_shader )
	{
		cerr << "Error: Creation of Raycast GLSL shader failed!" << endl;
		return false;
	}

	//// Load default set of shaders
	//if( !load_shader( "shader/raycast.vs.glsl", "shader/raycast.fs.glsl" ) )
	//	return false;

	return checkGLError( "RaycastShader::init()" );
}

//------------------------------------------------------------------------------
bool RaycastShader::load_shader( const char* vertex_shader, const char* fragment_shader )
{
	release();

	// load & compile
	if( !m_shader->load_from_disk( vertex_shader, fragment_shader ) )
	{
		cerr << "Error: Compilation of Raycast GLSL shader failed!" << endl;
		return false;
	}

	// get uniform locations
	m_loc_voltex   = m_shader->getUniformLocation( "voltex"   );
	m_loc_fronttex = m_shader->getUniformLocation( "fronttex" );
	m_loc_backtex  = m_shader->getUniformLocation( "backtex"  );
	m_loc_luttex   = m_shader->getUniformLocation( "luttex"  );
	m_loc_isovalue = m_shader->getUniformLocation( "isovalue" );
	m_loc_voxelsize= m_shader->getUniformLocation( "voxelsize" );

	return checkGLError( "RaycastShader::load_shader()" );
}

//------------------------------------------------------------------------------
void RaycastShader::deinit()
{
	if(m_shader) delete m_shader; m_shader=NULL;
}

//------------------------------------------------------------------------------
void RaycastShader::set_voxelsize( float sx, float sy, float sz )
{
	m_voxelsize[0] = sx;
	m_voxelsize[1] = sy;
	m_voxelsize[2] = sz;
}

//------------------------------------------------------------------------------
void RaycastShader::bind( GLuint voltex, GLuint fronttex, GLuint backtex, GLuint luttex )
{
	if( !m_shader ) return;

	// bind shader
	m_shader->bind();
	glUniform1i( m_loc_voltex  , voltex   );
	glUniform1i( m_loc_fronttex, fronttex );
	glUniform1i( m_loc_backtex , backtex  );
	glUniform1i( m_loc_luttex  , luttex  );
	glUniform1f( m_loc_isovalue, m_isovalue );
	glUniform3fv( m_loc_voxelsize, 1, m_voxelsize );

	checkGLError( "RaycastShader::bind()" );
}

//------------------------------------------------------------------------------
void RaycastShader::release()
{
	if( !m_shader ) return;
	m_shader->release();
}

