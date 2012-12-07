// Max Hermann, June 6, 2010
#include "VolumeRendererRaycast.h"
#include <GL/glew.h>
#include <GL/glu.h>		// for gluOrtho2D

#ifndef NO_BOOST_FILESYSTEM
#include <Misc/FilesystemTools.h>
#endif

#ifdef WIN32
// disable some VisualStudio warnings
#pragma warning(disable: 4244) 
#endif

using namespace GL;

VolumeRendererRaycast::VolumeRendererRaycast()
: m_verbosity(3)
, m_meanwarp(NULL)
, m_offscreen(true)
, m_debug(false)
{
	m_aspect[0]=m_aspect[1]=m_aspect[2]=1.f;
	m_warps[0]=m_warps[1]=m_warps[2]=m_warps[3]=m_warps[4] = NULL;
}

VolumeRendererRaycast::~VolumeRendererRaycast()
{}	

void VolumeRendererRaycast::reshape_ortho( int w, int h )
{
	glViewport( 0,0,w,h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0,w,0,h ); // 1:1 pixel to coordinates relation
	glMatrixMode( GL_MODELVIEW );
}

void VolumeRendererRaycast::setVolume( GLTexture* vtex )
{
	m_vtex = vtex;
}

void VolumeRendererRaycast::setWarpfield( GLTexture* warp )
{
	m_warps[0] = warp;
	// clear all other warps
}

void VolumeRendererRaycast::setMeanwarp( GLTexture* meanwarp )
{
	m_meanwarp = meanwarp;
}

void VolumeRendererRaycast::setWarpfields( GLTexture* mode0, GLTexture* mode1, 
	                                       GLTexture* mode2, GLTexture* mode3, 
										   GLTexture* mode4 )
{
	m_warps[0] = mode0;
	m_warps[1] = mode1;
	m_warps[2] = mode2;
	m_warps[3] = mode3;
	m_warps[4] = mode4;
}

void VolumeRendererRaycast::setAspect( float ax, float ay, float az )
{
	m_aspect[0] = ax;
	m_aspect[1] = ay;
	m_aspect[2] = az;

	m_cube.set_scale( 2*m_aspect[0], 2*m_aspect[1], 2*m_aspect[2] );
	m_cube.set_texcoord_scale( 1,1,1 );

	// Note:
	// Assume that texture is of same size as volume. This might not be the
	// case on older Hardware, where only power-of-two sizes are allowed.
	// For that case the following texcoord scaling according to the aspect
	// ratios should be implemented:
	//    cube.set_texcoord_scale( (g_vol->resX() / (float)g_vtex.GetWidth ()),
	//                             (g_vol->resY() / (float)g_vtex.GetHeight()),
	//                             (g_vol->resZ() / (float)g_vtex.GetDepth ()) );
}

void VolumeRendererRaycast::setIsovalue( float iso )
{
	m_raycast_shader.set_isovalue( iso );
}

void VolumeRendererRaycast::setZNear( float znear )
{
	m_znear = znear;
}

void VolumeRendererRaycast::setRenderMode( RaycastShader::RenderMode mode )
{
	m_raycast_shader.set_rendermode( mode );
	if( !m_raycast_shader.init() )
		std::cerr << "Fatal Error: Reloading raycast shader!" << std::endl;
}

float VolumeRendererRaycast::getIsovalue() const
{
	return m_raycast_shader.get_isovalue();
}

int VolumeRendererRaycast::getRenderMode() const
{
	return (int)m_raycast_shader.get_rendermode();
}

void VolumeRendererRaycast::getAspect( float& ax, float& ay, float& az ) const
{
	ax = m_aspect[0];
	ay = m_aspect[1];
	az = m_aspect[2];
}

void VolumeRendererRaycast::setOffscreenTextureSize( int width, int height )
{
	// allocate GPU mem
	GLint internalFormat = GL_RGBA32F; //GL_RGB12;
	m_front.Image(0, internalFormat, width,height, 0, GL_RGBA, GL_FLOAT, NULL );
	m_back .Image(0, internalFormat, width,height, 0, GL_RGBA, GL_FLOAT, NULL );
	m_vren .Image(0, internalFormat, width,height, 0, GL_RGBA, GL_FLOAT, NULL );
	// Note:
	// 8-bit per channel resolution leads to quantization artifacts so we
	// currently use 12-bit per channel which should be hardware supported.
	// For volume sizes greater than 1024 we should take an alternative
	// approach to generate start/end positions.
}

//------------------------------------------------------------------------------
//	reinit_shader()
//------------------------------------------------------------------------------
bool VolumeRendererRaycast::reinit_shader( std::string fs_path, std::string vs_path )
{
	assert( m_vtex );
	m_raycast_shader.set_volume_size     ( m_vtex->GetWidth (), 
	                                       m_vtex->GetHeight(), 
	                                       m_vtex->GetDepth () );
	m_raycast_shader.set_warp_enabled    ( m_warps[0]!=NULL );
	m_raycast_shader.set_multiwarp_enabled(m_warps[0]!=NULL && m_warps[1]!=NULL &&
	                                       m_warps[2]!=NULL && m_warps[3]!=NULL &&
										   m_warps[4]!=NULL );
	m_raycast_shader.set_meanwarp_enabled( m_meanwarp!=NULL );
	if( !m_raycast_shader.init( fs_path, vs_path ) )
	{
		std::cerr << "Error: Couldn't init raycast shader!" << std::endl;
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
//	init()
//------------------------------------------------------------------------------
bool VolumeRendererRaycast::init( int texWidth, int texHeight )
{
	using namespace std;

	assert( m_vtex );
	
	// size of render texture
	//int texWidth  = 2048,
	//	texHeight = 2048;  // FIXME: make size of render texture adjustable 

	// --- Find shader source ---

	// find shader source
	string fs_path, vs_path;
#ifndef NO_BOOST_FILESYSTEM
	int verbosity = 3;
	vector<string> search_paths, search_relpaths;
	search_paths.push_back( "./" );
	search_paths.push_back( "../");
	search_paths.push_back( "../../");
	//search_paths.push_back( argv[0] );
	//search_paths.push_back( boost::filesystem::initial_path().string() );  // same as "./"
	search_paths.push_back( Misc::get_executable_path() );	
	//search_relpaths.push_back("../");

	// FIXME: project specific search path
	search_relpaths.push_back("../e7/VolumeRendering/");
	search_relpaths.push_back("../sdmvis/e7/VolumeRendering/");
	search_relpaths.push_back("../trunk/sdmvis/e7/VolumeRendering/");
	search_relpaths.push_back("Release/");
	search_relpaths.push_back("../raycast/");
	search_relpaths.push_back("../../raycast/");

	fs_path = Misc::find_file( "raycast.fs.glsl", search_paths, "shader", search_relpaths, true, verbosity );
	vs_path = Misc::find_file( "raycast.vs.glsl", search_paths, "shader", search_relpaths, true, verbosity );
	cout << "raycast fragment shader: \"" << fs_path << "\"" << endl;
	cout << "raycast vertex shader: \"" << vs_path << "\"" << endl;
#endif
	
	// --- Init RaycastShader ---
	
	m_raycast_shader.set_num_channels ( 1 ); // m_vol->numChannels()	
	m_raycast_shader.set_rendermode   ( RaycastShader::RenderIsosurface );
	m_raycast_shader.set_depth_enabled( true );
	if( !reinit_shader( fs_path, vs_path ) )
		return false;
	
	// --- Setup 2D front and back texture ---
	
	if( !createTextures(texWidth,texHeight) ) return false;


	// --- Init RenderToTexture ---
	
	if( !createRenderToTexture(texWidth,texHeight) ) return false;
	
	return true;	
}

bool VolumeRendererRaycast::createTextures( int texWidth, int texHeight )
{
	using namespace std;
	if( m_verbosity > 1 ) cout << "Creating 2D textures..." << endl;
	if( !m_front.Create(GL_TEXTURE_2D) || 
		!m_back .Create(GL_TEXTURE_2D) ||
		!m_vren .Create(GL_TEXTURE_2D) )
	{
		cerr << "Error: Couldn't create 2D textures!" << endl;
		return false;
	}
	// allocate GPU mem
	setOffscreenTextureSize( texWidth, texHeight );
	return true;
}

void VolumeRendererRaycast::destroyTextures()
{
	using namespace std;
	if( m_verbosity > 1 ) cout << "Destroying 2D textures..." << endl;
	m_front.Destroy();
	m_back .Destroy();
	m_vren .Destroy();
}

void VolumeRendererRaycast::changeTextureSize( int width, int height )
{
	using namespace std;
	if( m_verbosity > 1 ) cout << "Changing texture size to " 
							   << width << " x " << height << endl;

	if( width==getTextureWidth() && height==getTextureHeight() )
		return;

	destroyRenderToTexture();
	destroyTextures();

	if( !createTextures(width,height) )
		throw; // FIXME: replace runtime error by purposeful exception
	if( !createRenderToTexture(width,height) )
		throw; // FIXME: replace runtime error by purposeful exception
}

bool VolumeRendererRaycast::createRenderToTexture( int texWidth, int texHeight )
{
	using namespace std;
	if( m_verbosity > 1 ) cout << "Initializing RenderToTexture..." << endl;
	if( !m_r2t.init( getTextureWidth(),getTextureHeight(), m_front.GetID(), true ) )
	{
		cerr << "Error: Couldn't init rendering to texture!" << endl;
		return false;
	}
}

void VolumeRendererRaycast::destroyRenderToTexture()
{
	m_r2t.deinit();
}

//------------------------------------------------------------------------------
//	destroy()
//------------------------------------------------------------------------------
void VolumeRendererRaycast::destroy()
{
	std::cout << "VolumeRendererRaycast::destroy()" << std::endl;
	m_raycast_shader.deinit();
	destroyRenderToTexture();
	destroyTextures();
}

//------------------------------------------------------------------------------
//	render()
//------------------------------------------------------------------------------

// Helper function
void draw_quad( float W, float H, GLenum unit=GL_TEXTURE0 )
{
	// FIXME: half pixel offset really needed here?
	float s0 = 0.; //(.5f + 0) / (float)g_texwidth;
	float s1 = 1.; //(.5f + g_texwidth-1) / (float)g_texwidth;
	float t0 = 0.; //(.5f + 0) / (float)g_texheight;
	float t1 = 1.; //(.5f + g_texheight-1) / (float)g_texheight;

	// make sure the correct unit is assigned (needed?)
	glActiveTexture( unit );

	// FIXME: do we produce an overdraw when viewSize > texture resolution?
	glBegin( GL_QUADS );
#if 1
	glMultiTexCoord2f( unit, s0,t0 );  glVertex2f( 0,0 );
	glMultiTexCoord2f( unit, s1,t0 );  glVertex2f( W,0 );
	glMultiTexCoord2f( unit, s1,t1 );  glVertex2f( W,H );
	glMultiTexCoord2f( unit, s0,t1 );  glVertex2f( 0,H ); 
#else
	glTexCoord2f( s0,t0 );  glVertex2f( 0,0 );
	glTexCoord2f( s1,t0 );  glVertex2f( W,0 );
	glTexCoord2f( s1,t1 );  glVertex2f( W,H );
	glTexCoord2f( s0,t1 );  glVertex2f( 0,H ); 
#endif
	glEnd();
}

void VolumeRendererRaycast::render()
{
	if( !m_vtex )
		return;
	
	//--------------------------------------------------------------------------
	// Setup

	// save common states
	glPushAttrib( GL_ALL_ATTRIB_BITS );	

	// save projection and modelvie matrix
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glMatrixMode( GL_MODELVIEW );	
	glPushMatrix();	
	
	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	
	int viewWidth  = viewport[2],
	    viewHeight = viewport[3];
	
	int texWidth  = m_front.GetWidth(),
	    texHeight = m_front.GetHeight();	

	// blend mode
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glBlendEquation( GL_FUNC_ADD );
	glEnable( GL_BLEND );
	// REMARK: 
	// - blending defines how raycasting result (a texture) is combined with
	//   the existing scene, especially noticeable for direct volume rendering
	// - other blending functions:
	//		for opacity-weighted color values use: 
	//             ( GL_ONE, GL_ONE_MINUS_SRC_ALPHA )
	//		default blending:
	//             ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA )

	
	//--------------------------------------------------------------------------
	// Step 1) Light ray setup
	// - rasterize front and back faces of volume geometry
	// - store in separate textures "back" and "front"
	// - ray-direction = back minus front

	glFrontFace( GL_CCW );
	glEnable( GL_CULL_FACE );
	glDisable( GL_TEXTURE_3D );

	glTranslatef( -m_aspect[0], -m_aspect[1], -m_aspect[2] );

	// set viewport to match texture dimensions
	glViewport( 0,0, texWidth,texHeight );
	glClearColor( 0,0,0,0 ); // clearcolor (0,0,0) indicates zero ray length

	// --- a) render back faces ---
	m_r2t.bind( m_back.GetID() );

	glClear( GL_COLOR_BUFFER_BIT );

	glCullFace( GL_FRONT );
	m_cube.draw_rgbcube();

	m_r2t.unbind();

	// --- b) render front faces ---
	m_r2t.bind( m_front.GetID(), true );  // w/ depth

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 

	glCullFace( GL_BACK );
	m_cube.draw_rgbcube();

	glDisable( GL_CULL_FACE );
	m_cube.draw_nearclip( (float)m_znear + 0.01f );

	m_r2t.unbind();

	//--------------------------------------------------------------------------
	// Step 2) Traverse ray
	// - use single-pass shader with loops (see RaycastShader)

	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );
	glDisable( GL_CULL_FACE );
	glEnable( GL_BLEND );  // affects shader?
	glColor4f( 1,1,1,1 );

	if( m_offscreen )
	{
		// render texture sized quad
		reshape_ortho( texWidth, texHeight );
		glLoadIdentity();

		// render to texture
		m_r2t.bind( m_vren.GetID(), true );  // w/ depth attachement

		// FIXME: clear needed?
		glClearColor( 0,0,0,0 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}
	else
	{
		// render screen sized quad
		reshape_ortho( viewWidth, viewHeight );
		glLoadIdentity();
	}

	// setup shader
	{	
		// REMARK:
		// Textures must be bound according to the specific texture unit
		// layout specified in RaycastShader::bind().

		// bind textures to specific texture units
		GLuint tex0=1;
		GLuint curtex=tex0;
		m_front.Bind( curtex++ );
		m_back .Bind( curtex++ );
		m_vtex->Bind( curtex++ );
		
		// bind warp textures (if available)
		if( m_warps[0] )
		{
			// FIXME: assume that either only first or all five warps are set!
			for( int i=0; i < 5; ++i )
				if( m_warps[i] )
					m_warps[i]->Bind( curtex++ );
			
			if( m_meanwarp )				
				m_meanwarp->Bind( curtex++ );
		}

		// bind front geometry depth as last unit
		glActiveTexture( GL_TEXTURE0 + curtex++ );
		glBindTexture( GL_TEXTURE_2D, m_r2t.getDepthTex() );

		m_raycast_shader.bind(tex0);
	}


	if( m_offscreen )
	{
		// render volume to texture
		draw_quad( texWidth, texHeight, GL_TEXTURE0 );

		m_raycast_shader.release();
		m_r2t.unbind();

		// render result texture to screen
		{
			reshape_ortho( viewWidth, viewHeight );

			m_vren.Bind( 0 );

			//const GLfloat envcol[4] = { 0,0,0,0 };
			//glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );
			//glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envcol );
			//glDisable( GL_BLEND );
			
			//glBlendFunc( GL_ONE, GL_SRC_ALPHA ); //GL_ONE_MINUS_SRC_ALPHA );

			glAlphaFunc( GL_GREATER, (GLclampf)0.0001 );
			glEnable( GL_ALPHA_TEST );

			glEnable( GL_TEXTURE_2D );
			draw_quad( viewWidth, viewHeight, GL_TEXTURE0 );

			glDisable( GL_ALPHA_TEST );
		}
	}
	else
	{
		// draw quad
		draw_quad( viewWidth, viewHeight, GL_TEXTURE0 );
		m_raycast_shader.release();
	}

	if( m_debug )
	{
		// draw front resp. back texture into lower right corner
		glDisable( GL_BLEND );
		glDisable( GL_DEPTH_TEST );
		glDisable( GL_CULL_FACE );

		glEnable( GL_TEXTURE_2D );

		reshape_ortho( 400, 200 );
		glClear( GL_DEPTH_BUFFER_BIT ); // HACK
		glPushMatrix();
		glLoadIdentity();
		m_back.Bind();
		draw_quad( 200, 200 );
		glTranslatef( 200,0,0 );
		m_front.Bind();
		draw_quad( 200, 200 );
		glPopMatrix();
	}


	//--------------------------------------------------------------------------
	//  Restore OpenGL states

	glActiveTexture( GL_TEXTURE0 );

	// restore modelview
	glPopMatrix();
	
	// restore original projection
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	
	// restore states
	glPopAttrib();	
}
