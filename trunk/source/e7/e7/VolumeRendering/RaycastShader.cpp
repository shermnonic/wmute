// Max Hermann, March 22, 2010
#include "RaycastShader.h"
#include <string>
#include <sstream>
#include <cassert>

//linux/ unix specific includes
#ifndef WIN32
#include <stdio.h>
#endif
using namespace GL;
using namespace std;

//------------------------------------------------------------------------------
//  string utility function
//------------------------------------------------------------------------------

int string_replace( std::string& text, std::string marker, const char* replacement );
int string_replace( std::string& s, std::string marker, std::string replacement );

int string_replace( std::string& text, std::string marker, const char* replacement )
{
	std::string repl_s( replacement );
	return string_replace( text, marker, repl_s );
}

int string_replace( std::string& s, std::string marker, std::string replacement )
{
	int num_replacements = 0;
	std::size_t pos = s.find( marker );
	while( pos != std::string::npos )
	{
		s.replace( pos, marker.length(), replacement );		
		num_replacements++;
		pos = s.find( marker );
	}
	return num_replacements;
}

std::string string_from_number( int i )
{
	std::stringstream ss;
	ss << i;
	return ss.str();
}

//------------------------------------------------------------------------------
bool RaycastShader::init( std::string fs_path, std::string vs_path )
{
	if( fs_path.empty() )
		fs_path = m_fs_path; // use last path
	else
		m_fs_path = fs_path; // store given path

	if( vs_path.empty() )
		vs_path = m_vs_path; // use last path
	else
		m_vs_path = vs_path; // store given path

	// release previous shader
	if( m_shader ) deinit();

	// create shader
	m_shader = new GLSLProgram;
	if( !m_shader )
	{
		cerr << "Error: Creation of Raycast GLSL shader failed!" << endl;
		return false;
	}

	// read shaders into strings
	char* buf_fs = GLSLProgram::read_shader_from_disk( fs_path.c_str() );
	if( !buf_fs ) return false;
	char* buf_vs = GLSLProgram::read_shader_from_disk( vs_path.c_str() );
	if( !buf_vs ) return false;
	string fs( buf_fs );
	string vs( buf_vs );
	delete [] buf_fs;
	delete [] buf_vs;

	// replace our custom preprocessor macros according to init config
	string_replace( fs, "<__opt_ISOSURFACE__>", (m_rendermode==RenderIsosurface || m_rendermode==RenderSilhouette) ? "1" : "0" );
	string_replace( fs, "<__opt_MIP__>"       , m_rendermode==RenderMIP        ? "1" : "0" );
	string_replace( fs, "<__opt_SILHOUETTE__>", m_rendermode==RenderSilhouette ? "1" : "0" );
	string_replace( fs, "<__opt_WARP__>"      , m_warp       ? "1" : "0" );
	string_replace( fs, "<__opt_MULTIWARP__>" , m_multiwarp  ? "1" : "0" );
	string_replace( fs, "<__opt_MEANWARP__>"  , m_meanwarp   ? "1" : "0" );
	string_replace( fs, "<__opt_DEPTHTEX__>"  , m_depth      ? "1" : "0" );
	string_replace( fs, "<__opt_CHANNELS__>", string_from_number( m_num_channels ) );
	string_replace( fs, "<__opt_WIDTH__>"   , string_from_number( m_size[0] ) );
	string_replace( fs, "<__opt_HEIGHT__>"  , string_from_number( m_size[1] ) );
	string_replace( fs, "<__opt_DEPTH__>"   , string_from_number( m_size[2] ) );
	string_replace( fs, "<__opt_COLORMODE__>" , string_from_number( m_colormode ) );

//cout << "DEBUG: Raycast fragment shader:" << endl << fs << "***EOF***" << endl;

	if( fs.find("<__")!=string::npos || fs.find("__>")!=string::npos )
	{
		cerr << "Error: Mismatch in shader pre-processing!" << endl;
		return false;
	}

	// load & compile
	if( !m_shader->load( vs, fs ) )
	{
		cerr << "Error: Compilation of Raycast GLSL shader failed!" << endl;
		return false;
	}

	// get uniform locations
	m_loc_stepsize    = m_shader->getUniformLocation( "stepsize"    );
	m_loc_voltex      = m_shader->getUniformLocation( "voltex"      );
	m_loc_fronttex    = m_shader->getUniformLocation( "fronttex"    );
	m_loc_backtex     = m_shader->getUniformLocation( "backtex"     ); 
	m_loc_isovalue    = m_shader->getUniformLocation( "isovalue"    );
	m_loc_alpha_scale = m_shader->getUniformLocation( "alpha_scale" );
	if( m_warp )
	{
		m_loc_warpmode[0] = m_shader->getUniformLocation( "warpmode0" );
		m_loc_lambda[0]   = m_shader->getUniformLocation( "lambda0" );

		if( m_multiwarp )
		{
			for( int i=1; i < MaxWarpModes; ++i )
			{
				char warpmode_s[255], lambda_s[255];
				sprintf( warpmode_s, "warpmode%d", i );
				sprintf( lambda_s, "lambda%d", i );
				m_loc_warpmode[i] = m_shader->getUniformLocation( warpmode_s );
				m_loc_lambda[i]   = m_shader->getUniformLocation( lambda_s );
			}
		}
		else
		{
			m_loc_warp_ofs = m_shader->getUniformLocation( "warp_ofs" );
		}

		if( m_meanwarp )
		{
			m_loc_meanwarptex = m_shader->getUniformLocation( "meanwarptex" );
		}
	}
	if( m_depth )
	{
		m_loc_depthtex = m_shader->getUniformLocation( "depthtex" );
	}

	return checkGLError( "RaycastShader::init()" );
}

//------------------------------------------------------------------------------
void RaycastShader::deinit()
{
	if(m_shader) delete m_shader; m_shader=NULL;
}

//------------------------------------------------------------------------------
void RaycastShader::bind( GLuint first )
{
	GLuint curtex = first;

	if( !m_shader ) return;

	// bind shader
	m_shader->bind();
	glUniform1i( m_loc_fronttex   , curtex++ );
	glUniform1i( m_loc_backtex    , curtex++ );
	glUniform1i( m_loc_voltex     , curtex++ );
	glUniform1f( m_loc_isovalue   , m_isovalue    );
	glUniform1f( m_loc_alpha_scale, m_alpha_scale );
	glUniform1f( m_loc_stepsize   , m_stepsize    );
	if( m_warp )
	{
		glUniform1i( m_loc_warpmode[0], curtex++ );
		glUniform1f( m_loc_lambda[0]  , m_lambda[0] );

		if( m_multiwarp )
		{
			for( int i=1; i < MaxWarpModes; ++i ) 
			{
				glUniform1i( m_loc_warpmode[i], curtex++ );
				glUniform1f( m_loc_lambda[i]  , m_lambda[i] );
			}
		}
		else
		{
			glUniform3f( m_loc_warp_ofs, m_warp_ofs[0], m_warp_ofs[1], m_warp_ofs[2] );
		}

		if( m_meanwarp )
		{
			glUniform1i( m_loc_meanwarptex, curtex++ );
		}
	}
	if( m_depth )
	{
		glUniform1i( m_loc_depthtex, curtex++ );
	}
}

//------------------------------------------------------------------------------
void RaycastShader::release()
{
	if( !m_shader ) return;
	m_shader->release();
}

