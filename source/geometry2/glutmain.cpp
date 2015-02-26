/**
* @file glutmain.cpp
*
* @brief Quick hack to support an GLUT window.
*	Supports animation. User must implement
*	- bool frame()      which is called every frame
*	- bool init()		which is called once right after GLinit()
*	- void shutdown()	which is called before exiting the application.
*/

//#define USE_GLUI
#define USE_MICHAELS_TRACKBALL

#include <iostream>
#include <stdlib.h>  // exit

//#define GLEW_STATIC 1
#include <GL/glew.h>
#ifdef USE_GLUI
#include <GL/glui.h>
#endif
#include <GL/glut.h>

//-------------------------------[ GLOBALS ]-----------------------------------
#ifdef USE_MICHAELS_TRACKBALL
	#include "trackball.inl"
	GLdouble        g_dXRot = 0.0;
	GLdouble        g_dYRot = 0.0;
	GLdouble        g_dZoom = -5.0;
	GLdouble        g_dTransX = 0;
	GLdouble        g_dTransY = 0;
	GLdouble        g_dTransZ = 0;
	int             g_iViewWidth = 0;
	int             g_iViewHeight = 0;
	GLdouble        g_dSpeed = 10.0;
	int             g_iOldX = -1;
	int             g_iOldY = -1;
	int             g_iButton = -1;
#endif // USE_MICHAELS_TRACKBALL

int main_window;
bool toggle[256];

int mousekey=0;	// mouse button status ( 0=none, 1=left, 2=right, 3=middle )

//-----------------------------[ PROTOTYPES ]----------------------------------
// following functions must be implemented by the user.
// if init() or frame() return false the program exits.
extern void welcome();
extern bool init( int argc, char** argv );
extern bool frame();
extern void shutdown();
extern void mouse_special( int button, int state, int x, int y );
extern bool motion_special( int x, int y );
#ifdef USE_GLUI
	extern void setupGLUI();
#endif

#define MOUSE_SPECIAL_KEY 'n'

//-----------------------------------------------------------------------------

void display()
{
#ifdef USE_MICHAELS_TRACKBALL
	// do trackball
    float   aaf_rot_matrix[ 4 ][ 4 ];
	glLoadIdentity( );
	normalize_quat( curquat );
	build_rotmatrix( aaf_rot_matrix, curquat );
	glTranslatef( 0.0, 0.0, g_dZoom );
	glMultMatrixf( &( aaf_rot_matrix[ 0 ][ 0 ] ) );
	glTranslatef( g_dTransX, g_dTransY, g_dTransZ );
#else
	glLoadIdentity( );
	glTranslatef( 0,0,-3 );
#endif

	frame();

//	glFlush();
//	glFinish();
	glutSwapBuffers();
}

void idle()
{
#ifdef USE_GLUI
	if ( glutGetWindow() != main_window )
	glutSetWindow(main_window);
	glutPostRedisplay();
#else
	display();
#endif
}

void reshape( int width, int height )
{
	glViewport( 0,0, width,height );
	
	//window_width = width;
	//window_height = height;
#ifdef USE_MICHAELS_TRACKBALL
    g_iViewWidth  = width;
    g_iViewHeight = height;
#endif	

	float aspect = (float)width/(float)height;

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45, aspect, 0.1, 42 );
	glMatrixMode( GL_MODELVIEW );
}

void keyboard( unsigned char key, int x, int y )
{	
	switch( key )
	{
		case 27:
		case 'q':
			shutdown();
			exit(0);
			break;

		default:
			toggle[key] = !toggle[key];
	}	
}

void mouse( int button, int state, int mousex, int mousey )
{
    if( toggle[MOUSE_SPECIAL_KEY] )
    {
        mouse_special( button, state, mousex, mousey );
        return;
    }

	if( state==GLUT_UP )
	{
		mousekey = 0;
#ifdef USE_MICHAELS_TRACKBALL
		g_iOldX = -1;
		g_iOldY = -1;
		g_iButton = -1;
#endif
	}
	else
	if( (button==GLUT_LEFT_BUTTON)&&(state==GLUT_DOWN) )
	{
		mousekey = 1;
#ifdef USE_MICHAELS_TRACKBALL
		g_iOldX = mousex;
		g_iOldY = mousey;
		g_iButton = mousekey;
#endif
	}
	else
	if( (button==GLUT_RIGHT_BUTTON)&&(state==GLUT_DOWN) )
	{
		mousekey = 2;
#ifdef USE_MICHAELS_TRACKBALL
		g_iOldX = mousex;
		g_iOldY = mousey;
		g_iButton = mousekey;
#endif
	}
	else
	if( (button==GLUT_MIDDLE_BUTTON)&&(state==GLUT_DOWN) )
	{
		mousekey = 3;
#ifdef USE_MICHAELS_TRACKBALL
		g_iOldX = mousex;
		g_iOldY = mousey;
		g_iButton = mousekey;
#endif
	}
}

void motion( int mousex, int mousey )
{
    if( motion_special( mousex, mousey ) ) return;
#ifdef USE_MICHAELS_TRACKBALL
	switch( mousekey )
	{
	case 1:
        g_dXRot += ( ( double ) ( g_iOldX - mousex ) ) / g_iViewWidth;
        g_dYRot += ( ( double ) ( g_iOldY - mousey ) ) / g_iViewHeight;
        trackball( lastquat,
                   0.5 * ( 2.0 * g_iOldX - g_iViewWidth ) / g_iViewWidth,
                   0.5 * ( g_iViewHeight - 2.0 * g_iOldY ) / g_iViewHeight,
                   0.5 * ( 2.0 * mousex - g_iViewWidth ) / g_iViewWidth,
                   0.5 * ( g_iViewHeight - 2.0 * mousey ) / g_iViewHeight );
        add_quats( lastquat, curquat, curquat );
		break;

	case 2:
        if( g_iOldX != -1 )
            g_dZoom = g_dZoom + ( mousey - g_iOldY ) * ( g_dSpeed - g_dZoom ) / 5000.0;
		break;

	case 3:
        GLfloat aaf_mat[4][4];

        build_rotmatrix( aaf_mat, curquat );

		g_dTransX -= aaf_mat[0][0]*( g_iOldX - mousex ) * ( g_dSpeed - g_dZoom ) / 10000.0;
        g_dTransY -= aaf_mat[1][0]*( g_iOldX - mousex ) * ( g_dSpeed - g_dZoom ) / 10000.0;
        g_dTransZ -= aaf_mat[2][0]*( g_iOldX - mousex ) * ( g_dSpeed - g_dZoom ) / 10000.0;
        g_dTransX += aaf_mat[0][1]*( g_iOldY - mousey ) * ( g_dSpeed - g_dZoom ) / 10000.0;
        g_dTransY += aaf_mat[1][1]*( g_iOldY - mousey ) * ( g_dSpeed - g_dZoom ) / 10000.0;
        g_dTransZ += aaf_mat[2][1]*( g_iOldY - mousey ) * ( g_dSpeed - g_dZoom ) / 10000.0;
		break;
	}

    g_iOldX = mousex;
	g_iOldY = mousey;
#endif

#ifdef USE_GLUI
	glutPostRedisplay();
#endif
}

void setupGLUT( int argc, char** argv )
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( 512, 512 );
	main_window = glutCreateWindow( "GLUT" );

#ifdef USE_GLUI
	GLUI_Master.set_glutReshapeFunc( reshape );  
	GLUI_Master.set_glutKeyboardFunc( keyboard );
	GLUI_Master.set_glutSpecialFunc( NULL );
	GLUI_Master.set_glutMouseFunc( mouse );
	glutMotionFunc( motion );
	std::cout << "Using GLUI " << GLUI_Master.get_version() << std::endl;
#else
	glutReshapeFunc( reshape );
	glutKeyboardFunc( keyboard );
	glutMouseFunc( mouse );
	glutMotionFunc( motion );
	glutIdleFunc( idle );
#endif
	//glutMotionFunc( motion );
	glutDisplayFunc( display );
}

void setupGLEW()
{
	GLenum err = glewInit();
	if( GLEW_OK != err )
	{
		std::cout << "Error initializing GLEW library:" << std::endl;
		std::cout << glewGetErrorString(err) << std::endl;
		exit(-1);
	}
	std::cout << "Using GLEW " << glewGetString( GLEW_VERSION ) << std::endl;
}

int main( int argc, char** argv )
{
	welcome();

	setupGLUT( argc, argv );

	glewExperimental = GL_TRUE;
	setupGLEW();

	if( !glewIsSupported("GL_VERSION_1_3") )
		return false;

#ifdef USE_GLUI
	setupGLUI();
#endif

	init( argc, argv );

#ifdef USE_MICHAELS_TRACKBALL
	// init trackball
    trackball( curquat, 0.0, 0.0, 0.0, 0.0 );
#endif

	glutMainLoop();

	shutdown();

	return 0;
}
