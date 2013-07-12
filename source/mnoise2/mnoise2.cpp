////////////////////////////////////////////////////////////////////////////////
// MNOISE2 with improved gui
//
// TODO:
// - path/parameter recording (determinstic random + seed)
// - playback and movie rendering
// - dump/load state (allows to collect several states interactively in lores
//                    and perform highres offscreen rendering subsequently)
// - some sort of bookmark
// - export mesh functionality (allow further processing e.g. in Meshlab)
// - modify light direction
//
// RECENTLY DONE:
// - started WiiMote support :-)
// - screenshot button
//   => TGA/PNG export, see Screenshot.cpp
// - hi/lores toggle (preview and animation vs. high-quality rendering)
//   => by rendering to offscreen render buffer (OpenGL>=2.0?)
//
// (c) 2004-2011 386dx25.de
////////////////////////////////////////////////////////////////////////////////

#include "mnoise2.h"

#ifndef WRITTEN_IMAGES_COMMAND_LINE_PROGRAM
  #ifndef USE_PLAIN_GLUT_INSTEAD_OF_GLUI 
    #define GLUI_GUI
  #endif 
#endif

#ifdef USE_WIIMOTE
 #ifndef WIN32
  #include <unistd.h>
  #define WIIUSE_PATH		"./wiiuse.so"
 #else
  #ifndef _DEBUG
  #define WIIUSE_PATH		"wiiuse.dll"
  #else
  #define WIIUSE_PATH		"wiiuse_debug.dll"
  #endif
 #endif
#include "wiiuse.h"
#endif

#include <GL/glew.h>
#ifdef SUPPORT_OFFSCREEN_RENDERING
 #include "GL/RenderToTexture.h"
 #include "GL/GLTexture.h"
 #include "Screenshot.h"
#endif
#ifdef WIN32
 #include <windows.h>
#endif
#include <GL/glut.h>
#ifdef GLUI_GUI
 #include <GL/glui.h>
#endif
#include <iostream>
#include <iomanip>
#include <cstdlib>        // atexit(), aoi(), atof()
#include <ctime>          // time(), clock()
#include <cassert>
#include <vector>
#include "MNoise.h"
#include "Frustum.h"
#include "PerlinNoise.h"
#include "primitives.h"
#include "matrix4x4.h"    // unused yet
#include "vector4.h"
#include "gl2ps.h"

using namespace std;

const int verbosity = 3;

/** global variables **********************************************************/
MNoise* mycube=NULL;
Frustum myfrust;
float	isovalue=0.5f;
int		cubecount=0;
float	speed=0;
int		fog=1;
float	clear[3];
float   fgcol[4];
float 	scale=1;
float   cellscale=1.f;

int   linesmooth=1;
float linewidth=1.f;

int blend=0;
int smooth=1;
int shaded=1;
int update=1;
int update_once=0;
int do_normals=1;
int color_material=1;

int dbg_show_octree=0;
int dbg_draw_all=0;

float persistance;
int octaves;

// display list for cube
int dl_cube=0;

int 	glut_main_window;
int		wireframe = 0;
int		overdraw = 1;
int		mode = 1;
float	posz = 0;
float 	view_rotate[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
vector4 trans_vector;
float view_tilt_alpha = 0.f,
      view_tilt_beta = 0.f;

// default mnoise settings
// MC size = 1/2 power of two size
// scale = (MC size)^(-1)
int   FIELDsize = 4;  // power of two exponent
int   MCsize  = 1<<(FIELDsize-1); 
float MCscale = 1./(float)MCsize;

#ifdef SUPPORT_OFFSCREEN_RENDERING
RenderToTexture r2t;
GL::GLTexture r2t_tex;
#endif

bool wiimote_available = false;

// some forwards
void randpos();
void randstate();


#ifdef USE_WIIMOTE
//------------------------------------------------------------------------------
/* WiiMote functions */

namespace WiiMote {

int ids[]  = { 1, 2 };
int leds[] = { WIIMOTE_LED_1, WIIMOTE_LED_2 };
wiimote** wiimotes =NULL;
bool wiimote_debug_info = false;

void handle_event( struct wiimote_t* wm )
{
	if(wiimote_debug_info) cout << "wiimote handle_event() ";
	// OBSOLETE: continous events handled parallel to poll() loop
  #if 0
	if( wm->btns_held || wm->btns )
	{
		if (IS_HELD(wm, WIIMOTE_BUTTON_B) || IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_B)) {
			isovalue -= 0.02;
			mycube->set_isovalue( isovalue );
		}
		if (IS_HELD(wm, WIIMOTE_BUTTON_A) || IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_A) ) {
			isovalue += 0.02;
			mycube->set_isovalue( isovalue );
		}
	}
  #endif
	if( wm->btns )
	{
		if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_MINUS))
			wiiuse_motion_sensing(wm, 0);
		if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_PLUS))
			wiiuse_motion_sensing(wm, 1);

		if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_ONE))
			randpos();
		if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_TWO))
			randstate();
		if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_HOME))
			wiimote_debug_info = !wiimote_debug_info;
	}

	if( WIIUSE_USING_ACC(wm) )
	{
		if(wiimote_debug_info) {
			cout << "orient=("
			<< setw(5) << (int)wm->orient.roll
			<< setw(5) << (int)wm->orient.pitch
			<< setw(5) << (int)wm->orient.yaw
			<< ") gforce=("
			<< setw(5) << setprecision(2) << wm->gforce.x
			<< setw(5) << setprecision(2) << wm->gforce.y
			<< setw(5) << setprecision(2) << wm->gforce.z
			<< ") ";
		}
	}

	if( WIIUSE_USING_IR(wm) )
	{
		if(wiimote_debug_info) {
			cout << "[IR:";	
			for( int i=0; i < 4; ++i )
				if( wm->ir.dot[i].visible )
					cout << i ;
			cout << "] ";
		}
	}

	if(wiimote_debug_info) cout << "\n";
}

void handle_read( struct wiimote_t* wm, byte* data, unsigned short len )
{
	cout << endl;
	cout << "--- DATA READ [" << wm->unid << "]" << endl;
}

//void handle_ctrl_status( struct wiimote_t* wm, int attachment, int speaker, int ir, int led[4], float battery_level )
void handle_ctrl_status( struct wiimote_t* wm )
{
	printf("\n\n--- CONTROLLER STATUS [wiimote id %i] ---\n", wm->unid);
	printf("attachment:      %i\n", wm->exp.type);
	printf("speaker:         %i\n", WIIUSE_USING_SPEAKER(wm));
	printf("ir:              %i\n", WIIUSE_USING_IR(wm));
	printf("leds:            %i %i %i %i\n", WIIUSE_IS_LED_SET(wm, 1), WIIUSE_IS_LED_SET(wm, 2), WIIUSE_IS_LED_SET(wm, 3), WIIUSE_IS_LED_SET(wm, 4));
	printf("battery:         %f %%\n", wm->battery_level);
}

void handle_disconnect( wiimote* wm )
{
	cout << endl;
	cout << "--- DISCONNECTED [" << wm->unid << "]" << endl;
}

int init()
{
#ifdef ALT_WIIUSE
	cout << "wiiuse version: " << wiiuse_version() << endl;
#else
	// load wiiuse library
	const char* version = wiiuse_startup( WIIUSE_PATH );
	if( !version )
	{
		cerr << "Failed to load wiiuse library." << endl;
		return -1;
	}
#endif

	// initialize array of wiimote objects
#ifdef ALT_WIIUSE
	wiimotes = wiiuse_init( 2 );
#else
	wiimotes = wiiuse_init( 2, ids, 
		// callbacks
		handle_event, handle_ctrl_status, handle_disconnect );
#endif

	// find wiimote device
	int found = wiiuse_find( wiimotes, 2, 5/*timeout 5sec*/ );
	if( !found )
	{
		cerr << "No wiimotes found!" << endl;
		return -2;
	}
	
	// connect wiimotes
	int connected = wiiuse_connect( wiimotes, 2 );
	if( connected )
		cout << "Connected to " << connected << " wiimotes (of " << found << " found)." << endl;
	else
	{
		cerr << "Failed to connect to any wiimote." << endl;
		return -3;
	}
	
	for( int i=0; i < 2; ++i  )
	{
		// set LED and rumble once
		wiiuse_set_leds( wiimotes[i], leds[i] );
		wiiuse_rumble( wiimotes[i], 1 );
	  #ifndef WIN32
		usleep(200000);
	  #else
		Sleep(200);
	  #endif
		wiiuse_rumble( wiimotes[i], 0 );

		// print status
		wiiuse_status( wiimotes[i] );

		// setup
		wiiuse_motion_sensing( wiimotes[i], 0 );
		wiiuse_set_ir        ( wiimotes[i], 0 );
		wiiuse_set_orient_threshold( wiimotes[i], 0.1 );
	}

	// success
	return 1;
}

void shutdown()
{
	wiiuse_disconnect( wiimotes[0] );
	wiiuse_disconnect( wiimotes[1] );
#ifdef ALT_WIIUSE
	wiiuse_cleanup( wiimotes, 2 );
#else
	wiiuse_shutdown();
#endif
}
	
} // namespace wiimote
#endif // USE_WIIMOTE

//------------------------------------------------------------------------------
/* called during startup */
#include "GLError.h"
void init()
{
	if( verbosity > 2 ) cout << "init()" << endl;

	// --- setup offscreen rendering ---
#ifdef SUPPORT_OFFSCREEN_RENDERING
	if( !r2t_tex.Create( GL_TEXTURE_2D ) )
	{
		cerr << "Error: Couldn't create render texture!" << endl;
		exit(-921);
	}
	r2t_tex.Image( 0, GL_RGB, OFFSCREEN_WIDTH,OFFSCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL );
	if( !r2t.init( r2t_tex.GetWidth(),r2t_tex.GetHeight(),r2t_tex.GetID(), true ) )
	{
		cerr << "Error: Couldn't initialize rendering to texture!" << endl;
		exit(-77);
	}
	GL::checkGLError("init()");
#endif
}

void setup_mnoise()
{
	// --- setup MNoise ---
	delete mycube; mycube=NULL;  // delete any previous instance
	mycube = new MNoise( FIELDsize, MCscale, MCsize );
	if( !mycube )
		assert(false); //exit(-1);
	
	mycube->build();
	
	persistance = mycube->get_persistance();
	octaves = mycube->get_octaves();
	cellscale = mycube->get_scale();
}

void deinit()
{
	if( verbosity > 2 ) cout << "deinit()" << endl;
#ifdef USE_WIIMOTE
	if( wiimote_available )
		WiiMote::shutdown();
#endif
#ifdef SUPPORT_OFFSCREEN_RENDERING
	r2t_tex.Destroy();
#endif
	delete mycube; mycube=NULL;
}

//------------------------------------------------------------------------------
void reshape( int width, int height )
{
	float aspect = (float)width / (float)height;
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 90/*120*/, aspect, 0.1, 50 );
	glMatrixMode( GL_MODELVIEW );

}

//------------------------------------------------------------------------------

void draw_internal()
{
	// for debug purposes frustum culling can be disabled
	if( dbg_draw_all )
	{
		glColor3f( 0,1,0 );
		mycube->draw_all();
	}
	else
		mycube->draw();
}

void draw_frustum()
{
	// extract frustum from current modelview and projection-matrix
	float proj[16];
	float modl[16];
	glGetFloatv( GL_PROJECTION_MATRIX, proj );
	glGetFloatv( GL_MODELVIEW_MATRIX, modl );
	myfrust.extract_frustum( modl, proj, true );

	mycube->set_frustum( &myfrust );		

	if( update || !dl_cube || update_once )
	{		
		glNewList( dl_cube, GL_COMPILE );
		draw_internal();
		glEndList();
		update_once = 0;
	}
	
	glCallList( dl_cube );
	
	#ifdef DEBUG_CUBE
	// draw control block
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( 0,0,-4 );
	draw_internal();
	glPopMatrix();
	#endif
}

//------------------------------------------------------------------------------
void render()
{
	glClearColor( clear[0], clear[1], clear[2], 1 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// -- Camera

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glTranslatef( 0,0, posz );
	glMultMatrixf( view_rotate );

	glRotatef( -view_tilt_alpha, 0,0,1 );
	glRotatef( view_tilt_beta, 1,0,0 );

	// -- Debug visualization of octree

	if( dbg_show_octree )
	{
		glDisable( GL_LIGHTING );
		glDisable( GL_FOG );
		glColor3f( 1,0,0 );
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glLineWidth( linewidth );
		mycube->draw_octree();
	}

	// -- States

	if( color_material )
		glEnable( GL_COLOR_MATERIAL );
	else
		glDisable( GL_COLOR_MATERIAL );
	
	glColor4fv( fgcol );
	if( shaded )
		glEnable( GL_LIGHTING );
	else
		glDisable( GL_LIGHTING );

	GLfloat fogcolor[4] = { clear[0], clear[1], clear[2], 0.5 };
	glFogfv( GL_FOG_COLOR, fogcolor );
	glFogi( GL_FOG_MODE, GL_LINEAR );
	glFogf( GL_FOG_DENSITY, 1.0 );
	glFogf( GL_FOG_START, 0.7f );
	glFogf( GL_FOG_END, 1.0f );

	if( fog )
		glEnable( GL_FOG );
	else
		glDisable( GL_FOG );	
	
	if( wireframe ) 
	{
		// push states
		glPushAttrib( GL_ALL_ATTRIB_BITS );
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glLineWidth( linewidth );

		// antialiased lines?
		float line_alpha = 1.f;
		if( linesmooth )
		{
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			glEnable( GL_BLEND );
			glEnable( GL_LINE_SMOOTH );
			line_alpha = 0.8f;
		}
		else
		{
			glDisable( GL_LINE_SMOOTH );
		}

		// line color
		float linecol[4];
		for( int k=0; k < 3; ++k ) 
			// inverse color for overdraw lines
			linecol[k] = overdraw ? (1.f - fgcol[k]) : fgcol[k];
		linecol[3] = line_alpha;
		glColor4fv( linecol );

		// draw isosurface (wireframe)
		draw_frustum();
		
		if( overdraw )
		{
			// overdraw again with surface (filled)
			glColor4fv( fgcol );
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			draw_frustum();
		}

		// restore states
		glPopAttrib();
	}
	else	
	{	
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		draw_frustum();
	}
}

//------------------------------------------------------------------------------
/* move forward in noisefield */
void move_forward( float stepsize )
{
	// move in direction the camera is facing
	matrix4x4 R( view_rotate );
	vector4 t( 0,0,-stepsize*1,0 );		

	t = t*R;
		
	trans_vector = trans_vector + t;
	
	// update position in noisefield
	mycube->set_posx( trans_vector[0] );
	mycube->set_posy( trans_vector[1] );
	mycube->set_posz( trans_vector[2] );
}

//------------------------------------------------------------------------------
/* reset globals to default and update GLUI */
void reset( int )
{
	// clear color
	clear[0] = 1;
	clear[1] = 1;
	clear[2] = 1;

	// foreground color (default = inverse clear color)
	for( int k=0; k < 3; ++k )
		fgcol[k] = 1.f - clear[k];
	fgcol[3] = 1.f;

#ifdef WRITTEN_IMAGES_COMMAND_LINE_PROGRAM

	color_material = 1;
	shaded = 0;
	blend = 0;
	smooth = 0;
	wireframe = 1;
	fog = 0;
	overdraw = 0;
	scale = 1;
	posz = 0;
	linesmooth = 0;
	linewidth = 1;
	speed = 0;
	posz = 0;
	update = 1;
	do_normals = 1;
	mode = 1;

	float R[16] = { 0.923461, -0.104063, -0.369311, 0, 
		           -0.134515, 0.813625, -0.565615, 0,
                    0.35934, 0.572001, 0.737353, 0, 
					0, 0, 0, 1 };
	for( int i=0; i < 15; ++i )
		view_rotate[i] = R[i];

	trans_vector = vector4( 1.25126, 563.585, 193.304, 1 );

#else // #ifdef WRITTEN_IMAGES_COMMAND_LINE_PROGRAM
	
	shaded = 1;
	blend = 0;
	smooth = 1;	
	wireframe = 0;
	overdraw = 0;
	speed = 0;
	posz = 0;
	scale = 1;
	mode = 1;
	update = 1;
	do_normals = 1;
	
	for( int i=0; i < 16; i++ ) view_rotate[i]=0;
	view_rotate[0] = 1;
	view_rotate[5] = 1;
	view_rotate[10] = 1;
	view_rotate[15] = 1;
	
	trans_vector = vector4( 0,0,0,0 );

#endif // #else #ifdef WRITTEN_IMAGES_COMMAND_LINE_PROGRAM

	/*
	if( blend ) glEnable( GL_BLEND ); else glDisable( GL_BLEND );
	if( smooth ) glShadeModel( GL_SMOOTH ); else glShadeModel( GL_FLAT );
	*/

#ifdef GLUI_GUI
	GLUI_Master.sync_live_all();
#endif
	
	move_forward(0);
	
	mycube->set_mode( mode );
}

//------------------------------------------------------------------------------

void writestate( std::ostream& os=std::cout )
{
	os << "[mnoise2_MarchingNoiseParameters]" << endl;
	os << "isovalue = " << isovalue << endl
	   << "persistance = " << persistance << endl
	   << "octaves = " << octaves << endl
	   << "FIELDsize = " << FIELDsize << endl
	   << "MCsize = " << MCsize << endl
	   << "MCscale = " << MCscale << endl;

	os << "[mnoise2_RenderSettings]" << endl;
	os << "clear = " << clear[0] << " " << clear[1] << " " << clear[2] << endl
	   << "fgcol = " << fgcol[0] << " " << fgcol[1] << " " << fgcol[2] << endl
	   << "color_material = " << color_material << endl
	   << "shaded = " << shaded << endl
	   << "blend = " << blend << endl
	   << "smooth = " << smooth << endl
	   << "wireframe = " << wireframe << endl
	   << "fog = " << fog << endl
	   << "overdraw = " << overdraw << endl
	   << "scale = " << scale << endl
	   << "posz = " << posz << endl
	   << "linesmooth = " << linesmooth << endl
	   << "linewidth = " << linewidth << endl
	   << "speed = " << speed << endl
	   << "posz = " << posz << endl
	   << "update = " << update << endl
	   << "do_normals = " << do_normals << endl
	   << "mode = " << mode << endl;

	os << "view_rotate = ";
	for( int i=0; i < 16; ++i )
		 os << view_rotate[i] << " ";
	os << endl;

	os << "trans_vector = ";
		for( int i=0; i < 4; ++i )
			os << trans_vector[i] << " ";
	os << endl;
}

void dumpstate()
{
	writestate( cout );
}

//------------------------------------------------------------------------------

void reseed( int )
{
#ifdef MNOISE_USE_IMPROVEDNOISE	
#else
	PerlinNoise::reseed();
	if( verbosity > 1 )
		printf("New seed = %X\n",PerlinNoise::get_seed());
#endif
}

float frand()
{
	return (float)(rand()%RAND_MAX)/(float)RAND_MAX;
};

void randpos()
{
	float x = frand()*1000;
	float y = frand()*1000;
	float z = frand()*1000;
	mycube->set_posx( x );
	mycube->set_posy( y );
	mycube->set_posz( z );
		trans_vector.set( x,y,z,1 );
	if( verbosity > 1 )
		printf("New position at (%5.2f,%5.2f,%5.2f)\n",x,y,z);
	update_once=1;
};

void randstate()
{	
	// clear color
	clear[0] = frand();
	clear[1] = frand();
	clear[2] = frand();

	// foreground color (default = inverse clear color)
	for( int k=0; k < 3; ++k )
		fgcol[k] = 1.f - clear[k];
	fgcol[3] = 1.f;

	color_material	= rand()%2;
	shaded			= rand()%2;
	blend			= rand()%2;
	smooth			= rand()%2;
	wireframe		= rand()%2;
	fog				= rand()%2;
	overdraw		= rand()%2;
	//scale			= 1;
	//posz			= 0;
	//speed			= 0;
	linesmooth		= rand()%2;
	linewidth		= 2.9f*frand()+.1f;
	//update		= 1;
	do_normals	= 1;
	mode			= rand()%4;

	/*
	float R[16] = { 0.923461, -0.104063, -0.369311, 0, 
		           -0.134515, 0.813625, -0.565615, 0,
                    0.35934, 0.572001, 0.737353, 0, 
					0, 0, 0, 1 };
	for( int i=0; i < 15; ++i )
		view_rotate[i] = R[i];

	trans_vector = vector4( 1.25126, 563.585, 193.304, 1 );
	*/

	/*
	if( blend ) glEnable( GL_BLEND ); else glDisable( GL_BLEND );
	if( smooth ) glShadeModel( GL_SMOOTH ); else glShadeModel( GL_FLAT );
	*/

#ifdef GLUI_GUI
	GLUI_Master.sync_live_all();
#endif

	move_forward(0);
	
	mycube->set_mode( mode );
	update_once = true;
}

//------------------------------------------------------------------------------
void export_ps( std::string filename="" )
{   
	FILE *fp;
	int state = GL2PS_OVERFLOW, buffsize = 0;
	
	time_t seconds = time(NULL);
	
	char autoname[1024];
	sprintf( autoname, "gl2ps-screenshot-%d.eps", seconds );
	if( filename.empty() )
		filename = std::string(autoname);
	
	fp = fopen( filename.c_str(), "wb" );
	if( !fp )
	{
		cerr << "Couldn't open file '" << filename << "'!" << endl;
		return;
	}

    while( state == GL2PS_OVERFLOW )
	{
      buffsize += 1024*1024;
      gl2psBeginPage("test", "gl2psTestSimple", NULL, GL2PS_EPS, GL2PS_BSP_SORT, 
                     GL2PS_DRAW_BACKGROUND | GL2PS_USE_CURRENT_VIEWPORT, 
                     GL_RGBA, 0, NULL, 0, 0, 0,  buffsize, fp, "out.eps");
      render();
      state = gl2psEndPage();
    }
    fclose(fp);
    printf( "Current GL-Viewport exportet to %s\n", filename.c_str() );
}

//------------------------------------------------------------------------------
#ifdef SUPPORT_OFFSCREEN_RENDERING
#include "GLError.h"
void export_offscreen( std::string filename )
{
	int w = r2t_tex.GetWidth(), 
		h = r2t_tex.GetHeight();

	std::cout << "export_offscreen() filename=\"" << filename << "\"" << std::endl;

	GL::checkGLError("export_offscreen() - begin");

	glPushAttrib( GL_ALL_ATTRIB_BITS );	GL::checkGLError("export_offscreen() - PushAttrib()");

	r2t.bind( r2t_tex.GetID() );  	GL::checkGLError("export_offscreen() - r2t.bind()");
	glViewport(0,0,w,h);            GL::checkGLError("export_offscreen() - viewport()");	
	reshape(w,h);                   GL::checkGLError("export_offscreen() - reshape()");

	// HACK: heuristically thicken lines for higher resolution
	float tmp_linewidth = linewidth;
#ifdef SCREENSHOT_AUTO_ADJUST_LINEWIDTH
	linewidth *= (double)w / 1024;
#endif
	render();                       GL::checkGLError("export_offscreen() - render()");
	linewidth = tmp_linewidth;

  #ifdef SCREENSHOT_SUPPORT_PNG
	Screenshot::savePNG( filename ); GL::checkGLError("export_offscreen() - savePNG()");
  #else
	Screenshot::saveTGA( filename ); GL::checkGLError("export_offscreen() - saveTGA()");
  #endif
	r2t.unbind();                   GL::checkGLError("export_offscreen() - r2t.unbind()");

	glPopAttrib();                  GL::checkGLError("export_offscreen() - PopAttrib()");
}

// dummy for GLUI_Update_CB
void export_offscreen( int )
{
  #ifdef SCREENSHOT_SUPPORT_PNG
	export_offscreen( Screenshot::autoName("offscreen-",".png") );
  #else
	export_offscreen( Screenshot::autoName("offscreen-",".tga") );
  #endif
}

#endif

//------------------------------------------------------------------------------
#define ID_FIRST 50
#define ID_ISOVALUE_CHANGE 50
#define ID_SPEED_CHANGE 51
#define ID_PERSISTANCE_CHANGE 52
#define ID_OCTAVES_CHANGE 53
#define ID_BLEND_CHANGE 54
#define ID_SMOOTH_CHANGE 55
#define ID_SCALE_CHANGE 56
#define ID_MODE_CHANGE 57
#define ID_NORMALS_CHANGE 58
#define ID_CELLSCALE_CHANGE 59
#define ID_LAST 59

void callback( int id )
{
	switch( id )
	{
		case ID_ISOVALUE_CHANGE:
			mycube->set_isovalue( isovalue );
			break;

		case ID_PERSISTANCE_CHANGE:
			mycube->set_persistance( persistance );
			break;

		case ID_OCTAVES_CHANGE:
			mycube->set_octaves( octaves );
			break;
			
		case ID_SPEED_CHANGE:
			break;
		
		case ID_SCALE_CHANGE:
			mycube->set_scale( scale );
			break;
		
		case ID_MODE_CHANGE:
			mycube->set_mode( mode );
			break;
		
		case ID_NORMALS_CHANGE:
			mycube->set_compute_normals( do_normals );
			break;

		case ID_CELLSCALE_CHANGE:
			mycube->set_scale( cellscale );
			break;
			
		case ID_BLEND_CHANGE:
		case ID_SMOOTH_CHANGE:
			if( blend ) glEnable( GL_BLEND ); else glDisable( GL_BLEND );
			if( smooth ) glShadeModel( GL_SMOOTH ); else glShadeModel( GL_FLAT );
			break;			
	}
}

//------------------------------------------------------------------------------
void glut_idle()
{
#ifdef USE_WIIMOTE
	using namespace WiiMote;
	if( wiimote_available ) {
 #ifdef ALT_WIIUSE
		if( wiiuse_poll( wiimotes, 2 ) ) 
		{
			int i=0;
			switch (wiimotes[i]->event) {
			case WIIUSE_EVENT:	handle_event( wiimotes[i] );  break;
			case WIIUSE_STATUS: handle_ctrl_status( wiimotes[i] ); break;
			}
		}
 #else
		wiiuse_poll( wiimotes, 2 );
		handle_event( wiimotes[0] );
 #endif
		// handle continous events parallel to poll (and genrated events)
		wiimote* wm = WiiMote::wiimotes[0];
		{
			if( IS_PRESSED(wm, WIIMOTE_BUTTON_B) ) {
				isovalue -= 0.02;
				mycube->set_isovalue( isovalue );
			}
			if( IS_PRESSED(wm, WIIMOTE_BUTTON_A) ) {
				isovalue += 0.02;
				mycube->set_isovalue( isovalue );
			}

			if( IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_PLUS) ) {
				mode = (mode+1)%4;
				mycube->set_mode( mode );
			}

			if( IS_PRESSED(wm, WIIMOTE_BUTTON_UP  ) ) speed += 0.001;
			if( IS_PRESSED(wm, WIIMOTE_BUTTON_DOWN) ) speed -= 0.001;
			if( IS_PRESSED(wm, WIIMOTE_BUTTON_HOME) ) speed = 0.;
		}

	  #if 0
		cout << "orient=("
				<< setw(5) << (int)wm->orient.roll
				<< setw(5) << (int)wm->orient.pitch
				<< setw(5) << (int)wm->orient.yaw
			 << ")  " << endl;
	  #endif
		// TODO: smooth wiimote input
		view_tilt_alpha = wm->orient.roll;
		view_tilt_beta = wm->orient.pitch;
	}
#endif

	if( glutGetWindow() != glut_main_window )
		glutSetWindow( glut_main_window );

	cubecount = mycube->get_cubecount();
#ifdef GLUI_GUI
	GLUI_Master.sync_live_all();
#endif		
	move_forward( speed );
		
	//if( speed > 0. )
		// HACK:
		// update only if needed to allow wiiuse to achieve 100hz meantime
		glutPostRedisplay();
}

//------------------------------------------------------------------------------
void glut_keyboard( unsigned char key, int x, int y )
{

	switch( key )
	{
	case 'w':
		move_forward(0.1);		
		break;	
	case 'r':
		randstate();
		break;

	case 'o':
		dbg_show_octree=(dbg_show_octree+1)%2;
		break;

	case 'a':
		dbg_draw_all=(dbg_draw_all+1)%2;
		break;

	case 27:
	case 'q':
		exit(0);
		break;		
	}
}

//------------------------------------------------------------------------------
void glut_reshape( int width, int height )
{
	if( (height<=10) || (width<=10) ) return;
#ifdef GLUI_GUI
	int tx, ty, tw, th;
	GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
	glViewport( tx, ty, tw, th );
#endif
	reshape( width, height );
	glutPostRedisplay();	
}

//------------------------------------------------------------------------------
void glut_display()
{
	render();
	
	glutSwapBuffers();
}

//------------------------------------------------------------------------------
void glui_callback( int control )
{
	//switch( control )
	//{
	//}
}

//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	using namespace std;

	/* parse command line */
	
	glutInit( &argc, argv );

#ifdef WRITTEN_IMAGES_COMMAND_LINE_PROGRAM

	// usage
	if( argc <= 1 )
	{
		cout << "usage: " << argv[0] << " <output1.png> [<output2.pn> ...]" << endl;
		return 0;
	}

	// read filenames from command line
	vector<string> filenames;
	for( int i=1; i < argc; ++i )
		filenames.push_back( argv[i] );

	// hardcoded options (see also reset())
	struct Params {
		int   FIELDsize;
		float MCscale;
		int   MCsize;
		float isovalue;
		float persistance;
		int   octaves;
		int   mode;
	} 
	WrittenImagesCandidates[5] = 
	{ 
	//  Fsiz MCsc MCsiz  iso        pers oct mode
		{ 6, 1./8. , 32, -0.076183, 0.75, 1, 1 },   // FAVOURITE 

		{ 5, 1./16., 16, -0.076183, 0.75, 1, 1 },   // correct 16^3 field
		{ 5, 1./8. , 16, -0.076183, 0.75, 1, 1 },   // painterly effect
		{ 5, 1./32., 16, -0.076183, 0.75, 1, 1 },   // checkerboard
		{ 5, 1.    , 16, -0.076183, 0.75, 1, 1 }    // "grid lost" chaos
	};
	
	Params wcc_params = WrittenImagesCandidates[0];	
	FIELDsize = wcc_params.FIELDsize;
	MCscale   = wcc_params.MCscale;
	MCsize    = wcc_params.MCsize;
#else
	if( argc > 1 )
	if( argv[1][0] == '?' )
	{
		cout << "usage: " << argv[0] << " [FIELDsize [MCscale [MCsize]]]" << endl;
		cout << "options:" << endl;
		cout << "  FIELDsize  power of two exponent for grid edge length" << endl;
		cout << "  MCscale    evaluation grid scale (no glitch for 1/MCsize)" << endl;
		cout << "  MCsize     evaluation grid size (no glitch for .5*2^FIELDsize)" << endl;
		return 0;
	}

	if( argc > 1 ) { FIELDsize = atoi(argv[1]); MCsize=1<<FIELDsize; MCscale=1./(float)(MCsize/2); }
	if( argc > 2 ) { MCscale   = atof(argv[2]); MCsize=1<<FIELDsize; }
	if( argc > 3 ) { MCsize    = atoi(argv[3]); }
#endif

	if( verbosity > 0 ) 
	{
		//cout << "FIELDsize = " << FIELDsize << " (" << (1<<FIELDsize) << ")" << endl;
		//cout << "MCscale   = " << MCscale << endl;
		//cout << "MCsize    = " << MCsize << endl;
		// -> use dumpstate(cout) to print settings
#ifdef MNOISE_USE_IMPROVEDNOISE
		cout << "ImprovedNoise" << endl;
#else
	#ifdef PERLINNOISE_USE_ICQ_FRAND
		cout << "PerlinNoise" << endl;
	#else
		cout << "PerlinNoise (with Inigo Quilez RNG)" << endl;
	#endif
#endif
	}	


	/* setup WiiMote */
#ifdef USE_WIIMOTE
	if( WiiMote::init() < 0 )
		wiimote_available = false;
	else
		wiimote_available = true;
#endif

	/* create glut window */
		
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowPosition( 200,100 );
	glutInitWindowSize( 1024,768 );
	
	glut_main_window = glutCreateWindow( "mnoise2" );
	glutDisplayFunc( glut_display );
	glutKeyboardFunc( glut_keyboard );

#ifdef SUPPORT_OFFSCREEN_RENDERING
	/* init GLEW */	

	GLenum glew_err = glewInit();
	if( glew_err != GLEW_OK )
	{
		cerr << "GLEW error:" << glewGetErrorString(glew_err) << endl;
		return -33;
	}
#endif

	/* init my stuff */

	init();
	setup_mnoise();
	reset(0);
#ifdef WRITTEN_IMAGES_COMMAND_LINE_PROGRAM
	isovalue    = wcc_params.isovalue;
	persistance = wcc_params.persistance;
	octaves     = wcc_params.octaves;
	mode        = wcc_params.mode;
#endif

	/* set up opengl states */	
	
#ifdef WRITTEN_IMAGES_COMMAND_LINE_PROGRAM
	glut_reshape( 4080/3, 2720/3 );
#else
	glut_reshape( 1600, 1200 ); //( 1024,768 );
#endif
	
	glClearColor( 1,1,1,1 );
	glDisable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glShadeModel( GL_FLAT );
	
	glBlendFunc( GL_SRC_COLOR, GL_ONE_MINUS_DST_COLOR );
	glEnable( GL_BLEND );
	
	/* set up opengl lights */
	
	GLfloat light2_diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };  // { 0.1f, 0.2f, 0.3f, 1.0f };
	GLfloat light_position[] = { 0.5f, 0.5f, 0.5f, 0.0f };
	GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat light2_position[] = { -0.5f, -0.5f, 1.0f, 0.0f };
  #if 0
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.5);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 1.5);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 2.2);
  #endif
	GLfloat mat_specular[] = { 1,1,1,1 };
	GLfloat mat_shininess[] = { 250.0 };

	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular );
	glMaterialfv( GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess );
	glDisable( GL_COLOR_MATERIAL );
		
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT1, GL_DIFFUSE, light2_diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, light2_position);
	glEnable(GL_LIGHT1);

	glEnable(GL_LIGHTING);
	
	/* activate opengl fog */	
	
	GLfloat fogcolor[4] = { 1, 1, 1, 0.5 };
	glFogfv( GL_FOG_COLOR, fogcolor );
	glFogi( GL_FOG_MODE, GL_EXP );
	glFogf( GL_FOG_DENSITY, 1.0 );
	glFogf( GL_FOG_START, 0.3f );
	glFogf( GL_FOG_END, 1.0f );
	glEnable( GL_FOG );
	
	/* prepare display list for cube */
	dl_cube = glGenLists( 1 );
	

#ifdef GLUI_GUI

	/* GLUI */

	GLUI_Master.set_glutReshapeFunc( glut_reshape ); 
	GLUI_Master.set_glutIdleFunc( glut_idle );

	// right side
	
  #ifdef UI_SEPARATE_WINDOWS
	GLUI *glui = GLUI_Master.create_glui( "mnoise2 controls", 0, 400, 500 );
  #else
	GLUI *glui = GLUI_Master.create_glui_subwindow( glut_main_window,
													GLUI_SUBWINDOW_RIGHT );
  #endif
	glui->set_main_gfx_window( glut_main_window );

	GLUI_Rollout* opt_rollout = new GLUI_Rollout( glui, "Options", true );
	GLUI_Checkbox* opt_blend  = new GLUI_Checkbox( opt_rollout, "Blending", &blend, ID_BLEND_CHANGE, (GLUI_Update_CB)callback );
	GLUI_Checkbox* opt_smooth = new GLUI_Checkbox( opt_rollout, "Smooth", &smooth, ID_SMOOTH_CHANGE, (GLUI_Update_CB)callback );
	GLUI_Checkbox* opt_shaded = new GLUI_Checkbox( opt_rollout, "Shaded", &shaded );
	GLUI_Checkbox* opt_fog    = new GLUI_Checkbox( opt_rollout, "Fog", &fog );
	GLUI_Checkbox* opt_wireframe  = new GLUI_Checkbox( opt_rollout, "Wireframe", &wireframe );
	GLUI_Checkbox* opt_linesmooth = new GLUI_Checkbox( opt_rollout, "Linesmooth", &linesmooth );
	GLUI_Checkbox* opt_overdraw   = new GLUI_Checkbox( opt_rollout, "Overdraw", &overdraw );
	GLUI_Checkbox* opt_update     = new GLUI_Checkbox( opt_rollout, "Update", &update );
	GLUI_Checkbox* opt_do_normals = new GLUI_Checkbox( opt_rollout, "Normals", &do_normals, ID_NORMALS_CHANGE, (GLUI_Update_CB)callback);
	GLUI_Checkbox* opt_color_material = new GLUI_Checkbox( opt_rollout, "ColorMaterial", &color_material );
	GLUI_Spinner* linewidth_spinner = new GLUI_Spinner( opt_rollout, 
		"Linewidth", GLUI_SPINNER_FLOAT, &linewidth );
	linewidth_spinner->set_float_limits( 0.1, 10.0, GLUI_LIMIT_CLAMP );

	GLUI_Rollout* col_rollout = new GLUI_Rollout( glui, "Colors", false );
	GLUI_StaticText* clear_text = new GLUI_StaticText( col_rollout, "Background" );
	GLUI_Spinner* clear0_spinner = new GLUI_Spinner( col_rollout, "Red", GLUI_SPINNER_FLOAT, &clear[0] );
	GLUI_Spinner* clear1_spinner = new GLUI_Spinner( col_rollout, "Green", GLUI_SPINNER_FLOAT, &clear[1] );
	GLUI_Spinner* clear2_spinner = new GLUI_Spinner( col_rollout, "Blue", GLUI_SPINNER_FLOAT, &clear[2] );
	clear0_spinner->set_float_limits( 0, 1, GLUI_LIMIT_CLAMP );
	clear1_spinner->set_float_limits( 0, 1, GLUI_LIMIT_CLAMP );
	clear2_spinner->set_float_limits( 0, 1, GLUI_LIMIT_CLAMP );	
	GLUI_StaticText* fgcol_text = new GLUI_StaticText( col_rollout, "Foreground" );
	GLUI_Spinner* fgcol0_spinner = new GLUI_Spinner( col_rollout, "Red", GLUI_SPINNER_FLOAT, &fgcol[0] );
	GLUI_Spinner* fgcol1_spinner = new GLUI_Spinner( col_rollout, "Green", GLUI_SPINNER_FLOAT, &fgcol[1] );
	GLUI_Spinner* fgcol2_spinner = new GLUI_Spinner( col_rollout, "Blue", GLUI_SPINNER_FLOAT, &fgcol[2] );
	fgcol0_spinner->set_float_limits( 0, 1, GLUI_LIMIT_CLAMP );
	fgcol1_spinner->set_float_limits( 0, 1, GLUI_LIMIT_CLAMP );
	fgcol2_spinner->set_float_limits( 0, 1, GLUI_LIMIT_CLAMP );

	//glui->add_statictext( "" );
	//GLUI_Spinner* scale_spinner = 
	//	glui->add_spinner( "Scale", GLUI_SPINNER_FLOAT, &scale, ID_SCALE_CHANGE, (GLUI_Update_CB)callback );
	//scale_spinner->set_float_limits( -3, 5, GLUI_LIMIT_CLAMP );

	glui->add_statictext( "" );
	GLUI_Spinner* isovalue_spinner = 
		glui->add_spinner( "Isovalue", GLUI_SPINNER_FLOAT, &isovalue, ID_ISOVALUE_CHANGE, (GLUI_Update_CB)callback );
	isovalue_spinner->set_float_limits( -2, 2, GLUI_LIMIT_CLAMP );

	GLUI_Spinner* persistance_spinner = 
		glui->add_spinner( "Pers.", GLUI_SPINNER_FLOAT, &persistance, ID_PERSISTANCE_CHANGE, (GLUI_Update_CB)callback );
	persistance_spinner->set_float_limits( 0, 2, GLUI_LIMIT_CLAMP );
	
	GLUI_Spinner* octaves_spinner = 
		glui->add_spinner( "Octaves", GLUI_SPINNER_INT, &octaves, ID_OCTAVES_CHANGE, (GLUI_Update_CB)callback );
	octaves_spinner->set_int_limits( 0, 6, GLUI_LIMIT_CLAMP );

	glui->add_spinner( "Speed", GLUI_SPINNER_FLOAT, &speed, ID_SPEED_CHANGE, (GLUI_Update_CB)callback );

	GLUI_Spinner* cellscale_spinner =
		glui->add_spinner( "Cell scale", GLUI_SPINNER_FLOAT, &cellscale, ID_CELLSCALE_CHANGE, (GLUI_Update_CB)callback );
	cellscale_spinner->set_float_limits( 1.f/128.f, 2.f, GLUI_LIMIT_CLAMP );


	glui->add_statictext( "" );
	glui->add_edittext( "Cubes", GLUI_EDITTEXT_INT, &cubecount );
	
	glui->add_statictext( "" );
	glui->add_button( "Export PS" , 3, (GLUI_Update_CB)export_ps );
#ifdef SUPPORT_OFFSCREEN_RENDERING
  #ifdef SCREENSHOT_SUPPORT_PNG
	glui->add_button( "Export PNG", 4, (GLUI_Update_CB)export_offscreen );	
  #else
	glui->add_button( "Export TGA", 4, (GLUI_Update_CB)export_offscreen );
  #endif
#endif
	
	glui->add_statictext( "" );
	GLUI_Spinner* mode_spinner = 
		glui->add_spinner( "Mode", GLUI_SPINNER_INT, &mode, ID_MODE_CHANGE, (GLUI_Update_CB)callback );
	mode_spinner->set_int_limits( 0, 3, GLUI_LIMIT_CLAMP );

	glui->add_statictext( "" );
	glui->add_button( "Randomize State", 5, (GLUI_Update_CB)randstate );
	glui->add_button( "Dump State", 4, (GLUI_Update_CB)dumpstate );
	glui->add_button( "Random Position", 3, (GLUI_Update_CB)randpos );
	glui->add_button( "Reseed", 2, (GLUI_Update_CB)reseed );
	glui->add_button( "Reset", 1, (GLUI_Update_CB)reset );
	glui->add_button( "Quit",  0, (GLUI_Update_CB)exit );

	// bottom

 #ifdef UI_SEPARATE_WINDOWS
	GLUI *glui2 = GLUI_Master.create_glui( "mnoise2 view", 0, 200, 50 );
 #else
	GLUI *glui2 = GLUI_Master.create_glui_subwindow( glut_main_window, 
													 GLUI_SUBWINDOW_BOTTOM );
 #endif
	glui2->set_main_gfx_window( glut_main_window );
	glui2->add_rotation( "Direction", view_rotate );
	glui2->add_column( false );
	GLUI_Translation* glui_trans = 
		glui2->add_translation( "Translation", GLUI_TRANSLATION_Z, &posz  );
	glui_trans->set_speed( .1f );
	glui2->add_column( false );

	GLUI_Master.sync_live_all();

#else
 #ifdef USE_PLAIN_GLUT_INSTEAD_OF_GLUI
	glutDisplayFunc ( glut_display );
	glutKeyboardFunc( glut_keyboard );
	glutReshapeFunc ( glut_reshape ); 
	glutIdleFunc    ( glut_idle );
 #endif
#endif // GLUI_GUI


	/* mainloop */
	
	for( int i=ID_FIRST; i <= ID_LAST; ++i )
		callback( i );	

#ifdef WRITTEN_IMAGES_COMMAND_LINE_PROGRAM
	// write out some screenshots and exit
	randpos();
	if( verbosity > 2 )
		dumpstate();
	for( int i=0; i < filenames.size(); ++i )
	{
		// stop time per image 
		// (WrittenImages contest is limited to 15s?!)
		clock_t t0 = clock();

		randpos();
		cout << "Writing image to " << filenames[i] << "..." << endl;
     #ifdef WRITTEN_IMAGES_OUTPUT_POSTSCRIPT
		export_ps( filenames[i] );
     #else
		export_offscreen( filenames[i] );
     #endif

		// stop time
		float dt = (clock()-t0)/(float)CLOCKS_PER_SEC;
		cout << "Took " << dt << " seconds" << endl;
	}
	cout << "Finished " << filenames.size() << " images." << endl;
	deinit();

#else
	// start GLUT mainloop
	atexit( deinit );	
	glutMainLoop();
#endif

	return 0;
}
