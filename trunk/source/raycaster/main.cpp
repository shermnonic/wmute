// SimpleRaycaster
// Program to demonstrate single pass raycasting using OpenGL 2.0 functionality.
// The implementation uses GLEW for extension handling and GLUT for the user 
// interface. The program requires vertex and fragment shaders in the 
// subdirectory /shaders.
// Max Hermann, 2013
#include "GLConfig.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <iostream>
#include "SimpleRaycaster.h"
#include "VolumeData.h"
#include "LookupTable.h"
#include "Filename.h"

using namespace std;

//------------------------------------------------------------------------------
//	Globals
//------------------------------------------------------------------------------

const char g_usage[] =
"Simple raycaster application \n"
"Max Hermann 2013 \n"
"\n"
"Usage: SimpleRaycaster <volume.mhd> <lookup.table>                         \n"
"\n";

SimpleRaycaster   g_raycaster;
VolumeDataHeader* g_volume;
LookupTable       g_lookuptable;

float g_rotx=0.f, g_roty=0.f;

//------------------------------------------------------------------------------
//	Forward declarations
//------------------------------------------------------------------------------

/// Helper function to load a MHD image volume from disk.
VolumeDataHeader* load_volume( const char* filename, int verb, void** data_ptr );


//------------------------------------------------------------------------------
//	Application menu
//------------------------------------------------------------------------------

enum Menu {
	MENU_SHADER_DEFAULT,
	MENU_SHADER_ISOSURFACE,
	MENU_SHADER_FRONT_CUBE,
	MENU_RELOAD_LUT
};

void menu( int entry )
{
	switch( entry )
	{
	case MENU_SHADER_DEFAULT:
		g_raycaster.getRaycastShader().load_shader
			("shader/raycast.vs.glsl","shader/raycast-minimal.fs.glsl");
		break;
	case MENU_SHADER_ISOSURFACE:
		g_raycaster.getRaycastShader().load_shader
			("shader/raycast.vs.glsl","shader/raycast-isosurface.fs.glsl");
		break;
	case MENU_SHADER_FRONT_CUBE:
		g_raycaster.getRaycastShader().load_shader
			("shader/raycast.vs.glsl","shader/raycast-front-cube.fs.glsl");
		break;
	case MENU_RELOAD_LUT:
		g_lookuptable.reload();
		g_raycaster.setLookupTable( &g_lookuptable );
		break;
	}
	
	glutPostRedisplay();
}

void setupMenu()
{
	glutCreateMenu( menu );
	glutAddMenuEntry( "Shader default",    MENU_SHADER_DEFAULT );
	glutAddMenuEntry( "Shader isosurface", MENU_SHADER_ISOSURFACE );
	glutAddMenuEntry( "Shader front cube", MENU_SHADER_FRONT_CUBE );
	glutAddMenuEntry( "Reload lookup table", MENU_RELOAD_LUT );	
	glutAttachMenu( GLUT_RIGHT_BUTTON );
}

//------------------------------------------------------------------------------
//	Application setup
//------------------------------------------------------------------------------

int init( const char* filename, const char* filename_lookupTable,
	      int width=512, int height=512 )
{	
	// --- Check for required OpenGL version and extensions ---

	const char extRequired[] = 
			// GL core 1.2
			"GL_EXT_texture3D " 
			// GL core 2.0
			"GL_ARB_shading_language_100 " 
			"GL_ARB_fragment_shader "
			"GL_ARB_vertex_shader "
			// GL core 3.0
			"GL_ARB_texture_float "
			"GL_EXT_framebuffer_object ";

	// Required GL extensions
	if( !glewIsSupported( "GL_ARB_vertex_shader" )) //extRequired ))
	{
		cerr << "Error: Insufficient GPU capabilities, maybe installing an "
			    " new driver version will help." << endl
			 << "The following capabilities / extensions are required: " << endl
			 << extRequired << endl;
		return -67;
	}

	// Optional GL extensions
	if( !glewIsSupported("GL_ARB_texture_non_power_of_two") )
	{
		cerr << "Warning: GL_ARB_texture_non_power_of_two not supported, i.e. "
			    "only image volumes width power of two edge lengths are "
				"supported." << endl;
	}

	// --- Load volume dataset ---

	void* dataptr = NULL;
	g_volume = load_volume( filename, 3, &dataptr );

	if( !g_volume )
	{
		cerr << "Error: Couldn't load volume data from " << filename << "!" 
		     << endl;
		return -1;
	}

	// --- Load lookup table ---
	
	if( !g_lookuptable.read( filename_lookupTable ) )
	{
		return -4;
	}

	// --- Init RaycastShader ---
	
	// Setup raycaster
	if( !g_raycaster.init( width, height ) )
	{
		cerr << "Error: Initializing raycaster failed!" << endl;
		return -2;
	}

	// Set default shader
	if( !g_raycaster.getRaycastShader().load_shader
			("shader/raycast.vs.glsl","shader/raycast-minimal.fs.glsl")	)
	{
		cerr << "Error: Could not load default shader!" << endl;
		return -5;
	}

	// Download lookup table
	g_raycaster.setLookupTable( &g_lookuptable );

	// Download volume
	if( !g_raycaster.downloadVolume( 
			g_volume->resX(), g_volume->resY(), g_volume->resZ(), 
			GL_UNSIGNED_BYTE, dataptr ) )
	{
		return -3;
	}
	
	// Some default OpenGL states
	glClearColor( 0,0,1,1 );
	glColor4f( 1,1,1,1 );	

	// --- Setup GLUT menu ---

	setupMenu();
	
	return 1;
}

void destroy()
{
	g_raycaster.destroy();
	if( g_volume ) delete g_volume; g_volume = NULL;
}

//------------------------------------------------------------------------------
//	GLUT callbacks
//------------------------------------------------------------------------------

void render()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	// Camera	
	glLoadIdentity();
	glTranslatef( 0,0,-5.f );
	glRotatef( g_rotx, 1,0,0 );
	glRotatef( g_roty, 0,1,0 );
	
	// Perform raycasting
	g_raycaster.render();

	glutSwapBuffers();
}

void reshape( int width, int height )
{
	float aspect = width / (float)height;
	glViewport( 0, 0, width, height );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 30, aspect, 0.1, 100 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}

void motion( int x, int y )
{
	// Simple camera, where pressing in the right/left and bottom/top of the
	// window produces rotation around y and x axis, respectively.

	int viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );

	g_rotx = (2.f*(y / (float)viewport[3]) - 1.f) *  90.f;
	g_roty = (2.f*(x / (float)viewport[2]) - 1.f) * 180.f;

	glutPostRedisplay();
}

//------------------------------------------------------------------------------
//	main()
//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	if( argc != 3 )
	{
		cout << g_usage << endl;
		return 0;
	}

	// size of render texture
	const int width  = 512, 
		      height = 512;

	// setup GLUT
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( width, height );
	glutCreateWindow( "SimpleRaycaster" );
	glutReshapeFunc ( reshape  );
	glutDisplayFunc ( render   );
	glutMotionFunc  ( motion );	

	// setup GLEW
	GLenum glew_err = glewInit();
	if( glew_err != GLEW_OK )
	{
		cerr << "GLEW error:" << glewGetErrorString(glew_err) << endl;
		return -11;
	}
	
	// init application	
	int ret = init( argv[1], argv[2], width, height ); // "data/Engine.mhd"
	if( ret < 0 )
		return ret;

	// exit function
	atexit( destroy );

	// mainloop
	glutMainLoop();
	
	// never reached, glutMainLoop() quits with exit()
	return 0;
}


//------------------------------------------------------------------------------
//	Helper functions
//------------------------------------------------------------------------------

VolumeDataHeader* load_volume( const char* filename, int verb, void** data_ptr )
{
	Misc::Filename fname( filename );
	VolumeDataHeader* vol = NULL;

	// -- load MHD/DAT header --

	if( verb > 1 ) 
		cout <<"Loading volume dataset \""<< fname.filename <<"\"..."<<endl;

	VolumeDataHeaderLoaderMHD mhd;
	VolumeDataHeaderLoaderDAT dat;
	if( fname.ext == "mhd" )
	{
		if( mhd.load( fname.filename.c_str() ) )
			vol = (VolumeDataHeader*)&mhd;
	}
	else
	if( fname.ext == "dat" )
	{
		if( dat.load( fname.filename.c_str() ) )
			vol = (VolumeDataHeader*)&dat;
	}
	else
	{
		cerr << "Error: Unknown file extension " << fname.ext << "!" << endl;
		return NULL;
	}

	if( !vol )
	{
		cerr << "Error: Couldn't load " << fname.filename << "!" << endl;
		return NULL;
	}

	// -- load RAW volume data --

	std::string raw_filename = fname.path + vol->filename();
	VolumeDataBase* base = VolumeDataBase::load_raw( raw_filename.c_str(), vol );
	*data_ptr = base ? base->void_ptr() : NULL;
	return (VolumeDataHeader*)base;
}
