#include "SimpleRaycaster.h"
#include "LookupTable.h"
#include <iostream>
#include <cassert>

using namespace std;

bool SimpleRaycaster::init( int width, int height )
{
	m_width  = width;
	m_height = height;

	// --- Create textures ---
	
	if( !m_lut_tex   .Create(GL_TEXTURE_1D) ||
		!m_front_tex .Create(GL_TEXTURE_2D) || 
		!m_back_tex  .Create(GL_TEXTURE_2D) ||
		!m_screen_tex.Create(GL_TEXTURE_2D) )
	{
		cerr << "Error: Couldn't create 2D textures!" << endl;
		return false;
	}
	// allocate GPU mem
	m_front_tex  .Image( 0, GL_RGBA32F, width,height, 0, GL_RGBA, GL_FLOAT, NULL );
	m_back_tex   .Image( 0, GL_RGBA32F, width,height, 0, GL_RGBA, GL_FLOAT, NULL );
	m_screen_tex .Image( 0, GL_RGBA32F, width,height, 0, GL_RGBA, GL_FLOAT, NULL );	
	m_lut_tex    .Image( 0, GL_RGBA32F, m_lut_size, 0, GL_RGBA, GL_FLOAT, NULL );

	// --- Setup shader ---

	if( !m_raycast_shader.init() )
	{
		cerr << "Error: Failed to initialize raycast shader!" << endl;
		return false;
	}
	
	// --- Initialize RenderToTexture ---	
	
	if( !m_r2t.init( width, height ) )
	{
		cerr << "Error: Failed to setup rendering to texture!" << endl;
		return false;
	}
	
	// --- Create 3D texture ---
	if( !m_volume_tex.Create(GL_TEXTURE_3D) )
	{
		cerr << "Error: Coudln't create 3D texture!" << endl;
		return false;
	}
	
	// Set required texture parameters, should be hardware supported
	m_volume_tex.SetWrapMode  ( GL_CLAMP_TO_EDGE ); 
	m_volume_tex.SetFilterMode( GL_LINEAR );	
	
	return true;
}

bool SimpleRaycaster::downloadVolume( int width, int height, int depth, 
                                      GLenum elementType, void* dataPtr )
{
	m_res[0] = width;
	m_res[1] = height;
	m_res[2] = depth;

	m_raycast_shader.set_voxelsize( 1.f/width, 1.f/height, 1.f/depth );

	// Default spacing
	m_spacing[0] = m_spacing[1] = m_spacing[2] = 1.0;

	// Required for odd texture sizes
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	
	// This class only supports 1-component scalar data
	GLint  internalFormat = GL_ALPHA8;
	GLenum format         = GL_ALPHA;
	
	// Allocate GPU memory
	if( !m_volume_tex.Image( 0, internalFormat, width, height, depth, 0, 
	                         format, GL_UNSIGNED_BYTE, NULL ) )
	{
		cerr << "Error: Couldn't allocate 3D texture!" << endl;
		return false;
	}
	
	// --- download data ---	
	
	if( !m_volume_tex.SubImage( 0, 0,0,0, width, height, depth,
	                            format, elementType, dataPtr ) )
	{
		cerr << "Error: Failed to download volume to GPU!" << endl;
		return false;
	}	
	
	return true;
}

void SimpleRaycaster::setLookupTable( LookupTable* lut )
{
	m_lut = lut;

	if( !lut )
		return;

	std::vector<float> buffer;
	lut->getTable( buffer, m_lut_size );

	m_lut_tex.SetWrapMode( GL_CLAMP_TO_EDGE );
	m_lut_tex.SetFilterMode( GL_LINEAR );
	m_lut_tex.Image( 0, GL_RGBA32F, m_lut_size, 0, GL_RGBA, GL_FLOAT, 
		             (void*)(&buffer[0]) );
}

void SimpleRaycaster::setSpacing( float spacingX, float spacingY, float spacingZ )
{
	m_spacing[0] = spacingX;
	m_spacing[1] = spacingY;
	m_spacing[2] = spacingZ;
}

void SimpleRaycaster::destroy()
{
	// FIXME: On my Windows machine releasing the following OpenGL resources
	//        will produce a crash. The problem could be that at this point 
	//        we don't have a valid OpenGL context.
	//m_raycast_shader.deinit();
	//m_r2t.deinit();
	
	m_front_tex .Destroy();
	m_back_tex  .Destroy();
	m_screen_tex.Destroy();
	m_lut_tex   .Destroy();
	m_volume_tex.Destroy();
}

void SimpleRaycaster::render()
{
	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glDisable( GL_TEXTURE_2D );

	generateStartEndPositions();
	traverseRays();
	
	// Render result as screen sized quad (ignoring aspect ratio issue)
	glColor4f( 1,1,1,1 );
	glEnable( GL_TEXTURE_2D );
	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );

	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );	

	m_screen_tex.Bind();
	drawTexQuad( viewport[2], viewport[3] );

	glPopAttrib();
}

void SimpleRaycaster::drawRGBCube()
{
	/* 
		   6-----7     Arbitrary cube notation                                  
	      /|    /|                                                              
         2-----3 |     Faces defined CCW, e.g. (0,1,3,2)                        
	     | 5---|-4     Colors directly correspond to canonical coordinates
	     |/    |/      which are given in texture coordinates.
	     0-----1       tc0=(0,0,1), tc7=(1,1,0)                 	   
	*/
	float v[8][3] = { {-1,-1,+1}, {+1,-1,+1}, {-1,+1,+1}, {+1,+1,+1},
	                  {+1,-1,-1}, {-1,-1,-1}, {-1,+1,-1}, {+1,+1,-1} };
	float tc[8][3] = { {0,0,1}, {1,0,1}, {0,1,1}, {1,1,1},
	                  {1,0,0}, {0,0,0}, {0,1,0}, {1,1,0} };
	int   f[6][4] = { {0,1,3,2}, {1,4,7,3}, {4,5,6,7}, {5,0,2,6},
	                  {2,3,7,6}, {5,4,1,0} };

	// Consider volume aspect ratio
	float scale[3] = { 1.f, 1.f, 1.f };
	scale[1] = m_res[1] / (float)m_res[0];
	scale[2] = m_res[2] / (float)m_res[0];

	// Consider voxel spacing, normalize largest spacing to 1
	float maxSpacing = std::max( std::max(m_spacing[0],m_spacing[1]), m_spacing[2] );
	scale[0] *= m_spacing[0] / maxSpacing;
	scale[1] *= m_spacing[1] / maxSpacing;
	scale[2] *= m_spacing[2] / maxSpacing;

	// Scale vertices
	for( int i=0; i < 8; ++i )
		for( int c=0; c < 3; ++c )
			v[i][c] *= scale[c];
		
	glBegin( GL_QUADS ); // GL_TRIANGLE_FAN
	for( int i=0; i < 6; ++i )
		for( int j=0; j < 4; ++j )
		{
			glColor3fv   ( tc[f[i][j]] );
			glTexCoord3fv( tc[f[i][j]] );
			glVertex3fv  ( v[f[i][j]] );
		}
	glEnd();
}

void SimpleRaycaster::drawTexQuad( int width, int height, GLenum unit )
{
	float s0 = 0.f,
	      s1 = 1.f,
	      t0 = 0.f,
	      t1 = 1.f;

	float w = (float)width,
		  h = (float)height;

	// Set projection & modelview to match 1:1 pixel to coordinates
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D( 0,width,0,height ); 
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	glActiveTexture( unit );
	
	glBegin( GL_QUADS );	
	glMultiTexCoord2f( unit, s0,t0 );  glVertex2f( 0,0 );
	glMultiTexCoord2f( unit, s1,t0 );  glVertex2f( w,0 );
	glMultiTexCoord2f( unit, s1,t1 );  glVertex2f( w,h );
	glMultiTexCoord2f( unit, s0,t1 );  glVertex2f( 0,h );
	glEnd();

	// Restore old projection & modelview
	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );	
}


void SimpleRaycaster::generateStartEndPositions()
{
	glPushAttrib( GL_ALL_ATTRIB_BITS );
	
	//--------------------------------------------------------------------------
	// Step 1) Light ray setup
	// - rasterize front and back faces of volume geometry
	// - store in separate textures "back" and "front"
	// - ray-direction = back - front

	glFrontFace( GL_CCW );
	glEnable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST ); // ?
	glClearColor( 0,0,0,1 ); // set clearcolor to 0 (indicating zero ray length)

	// --- a) Render back faces ---	 
	m_r2t.begin( m_back_tex.GetID() );
	glClear( GL_COLOR_BUFFER_BIT );
	glCullFace( GL_FRONT );	
	drawRGBCube();	
	m_r2t.end();

	// --- b) Render front faces ---
	m_r2t.begin( m_front_tex.GetID() );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glCullFace( GL_BACK );
	drawRGBCube();	
	m_r2t.end();	
	
	glPopAttrib();
}

void SimpleRaycaster::traverseRays()
{
	//--------------------------------------------------------------------------
	// Step 2) Traverse ray
	// - use single-pass shader with loops (see RaycastShader)

	glClearColor( 1,1,0,0 ); 
	
	m_r2t.begin( m_screen_tex.GetID() );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	// Bind textures to specific texture units
	m_volume_tex.Bind( 0 );
	m_front_tex .Bind( 1 );
	m_back_tex  .Bind( 2 );
	m_lut_tex   .Bind( 3 );
	m_raycast_shader.bind( 0, 1, 2, 3 );	
	
	// Invoke fragment shader & perform raycasting for each pixel
	drawTexQuad( m_width, m_height );
	
	m_raycast_shader.release();	
	m_r2t.end();	
}
