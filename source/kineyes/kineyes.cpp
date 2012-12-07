// kineyes - 3D model follows user movement
// Max Hermann, November 25, 2012
#include <cstdlib> // EXIT_SUCCESS
#include <memory>  // std::auto_ptr
#include <iostream>
#include <GL/glut.h>
#include <XnCppWrapper.h>
#include "NiHandTracker.h"
#include "SimpleEyes.h"

#define OPENNI_CONFIG_XML_PATH "../../data/OpenNI/SamplesConfig.xml"

#define CHECK_RC(nRetVal, what) \
    if (nRetVal != XN_STATUS_OK) { \
      printf("%s failed: %s\n", what, xnGetStatusString(nRetVal)); \
      return nRetVal; \
	}

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
xn::Context		     g_context;
xn::ScriptNode	     g_scriptNode;
xn::DepthGenerator   g_depthGenerator;
xn::ImageGenerator   g_imageGenerator;

HandTracker*         g_handTracker(NULL);

SimpleEyes           g_eyes;

//-----------------------------------------------------------------------------
//  OpenNI functions
//-----------------------------------------------------------------------------

void update()
{
	XnStatus rc;

	// Read a new frame
	rc = g_context.WaitAnyUpdateAll();
	if( rc != XN_STATUS_OK )
	{
		printf( "Read failed: %s\n", xnGetStatusString(rc) );
		exit(rc);
	}	

	// Last detected point
	if( g_handTracker )
	{
		XnPoint3D p = g_handTracker->GetLastPosition();
		printf("(%8.2f,%8.2f,%8.2f)                           \r",p.X,p.Y,p.Z);
	}
}

void destroy()
{
	g_eyes.destroy();

	// Release OpenNI nodes
	g_scriptNode    .Release();
	g_depthGenerator.Release();
	g_context       .Release();

	delete g_handTracker; g_handTracker=NULL;
}

//-----------------------------------------------------------------------------
// Auxiliary drawing functions
//-----------------------------------------------------------------------------

enum { Wireframe=0, Filled=1 };

void drawCube( float xmin, float xmax, float ymin, float ymax, float zmin, float zmax,
	           int mode=Filled )
{
	/*
		   6-----7     Arbitrary cube notation                                  
		  /|    /|                                                              
		 2-----3 |     Faces defined CCW, e.g. (0,1,3,2)                        
		 | 5---|-4     
		 |/    |/      
		 0-----1                                                                
	*/	
	float verts[8][3] = { {0,0,1}, {1,0,1}, {0,1,1}, {1,1,1},
						  {1,0,0}, {0,0,0}, {0,1,0}, {1,1,0} };
	const int faces[6][4] = { 
		{0,1,3,2}, {1,4,7,3}, {4,5,6,7}, {5,0,2,6}, {2,3,7,6}, {5,4,1,0} };
	const int edges[12][2] = { {0,1},{1,3},{3,2},{2,0},
	                           {5,4},{4,7},{7,6},{6,5},
	                           {0,5},{1,4},{3,7},{2,6} };

	float dx = fabs(xmax-xmin),
		  dy = fabs(ymax-ymin),
		  dz = fabs(zmax-zmin);

	// Translate to centroid and scale
	for( int i=0; i < 8; ++i ) {
			verts[i][0] = verts[i][0]*dx + xmin;
			verts[i][1] = verts[i][1]*dy + ymin;
			verts[i][2] = verts[i][2]*dz + zmin;
	}

	if( mode == Wireframe )
	{
		glBegin( GL_LINES );
		for( int i=0; i < 12; ++i )
			for( int j=0; j < 2; ++j )
			{
				int idx = edges[i][j];
				glVertex3fv( verts[idx] );
			}
		glEnd();
	}
	else
	{
		glBegin( GL_QUADS );
		for( int i=0; i < 6; ++i )
			for( int j=0; j < 4; ++j )
			{
				int idx = faces[i][j];
				glVertex3fv( verts[idx] );
			}
		glEnd();
	}
}

void drawScene()
{
	glPushMatrix();
	glScalef( .01, .01, .01 );
	
	// Draw working volume
	glColor3f( 1,1,1 );
	drawCube( -1000,1000, -1000,1000, 480,3600, Wireframe );

	// Draw Kinect/Xtion position at (0,0,0)
	glColor3f( 0,0,1 );
	drawCube( -35,35, -15,15, -10,10, Filled );
	
	// Draw detected position
	XnPoint3D p = g_handTracker->GetLastPosition();	
	glPushMatrix();	
	glTranslatef( p.X, p.Y, p.Z );
	glColor3f( 1,1,0 );
	drawCube( -50,50, -50,50, -50,50, Filled );
	glPopMatrix();

	// Draw eyes
	glPushAttrib( GL_ENABLE_BIT );
	glEnable( GL_CULL_FACE );
	glColor3f( 1,1,1 );
	g_eyes.setPosition( 0, 0, 0 );
	g_eyes.setRadius( 300 );
	g_eyes.setSeparation( 400 );
	g_eyes.setPOI( p.X, p.Y, p.Z );
	g_eyes.draw();
	glPopAttrib();
	

	glPopMatrix();
	
}

//-----------------------------------------------------------------------------
// GLUT callbacks
//-----------------------------------------------------------------------------

void glutIdle()
{
	update();
	glutPostRedisplay();
}

void glutRender()
{
	glClearColor( 0,0,0,1 );
	glClear( GL_COLOR_BUFFER_BIT );

	// Smooth lines
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_BLEND );
	glEnable( GL_LINE_SMOOTH );
	glLineWidth( 0.5 );

#if 1
	glLoadIdentity();
	glTranslatef( 0,0,-60 );
	glRotatef( 30, 1,0,0 );
	glTranslatef( 0,0,-20 );
	drawScene();
#else
	// Wireframe mode
	glColor3f( 1,1,1 );
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glEnable( GL_CULL_FACE );
	
	// Render scene
	glLoadIdentity();
	g_eyes.setPosition( 0, 0, -15 );
	g_eyes.draw();
#endif

	glutSwapBuffers();
}

void glutReshape( GLint w, GLint h )
{
	float aspect = (double)w/h;
	glViewport( 0,0, w,h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45, aspect, 0.1, 100 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}

void glutKeyboard( unsigned char key, int mousex, int mousey )
{
	switch( key )
	{
	case 27: // Escape
		exit(EXIT_SUCCESS);
		break;
	}
		
	int viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	float px = mousex / (float)viewport[2] - .5f,
	      py = (viewport[3]-mousey) / (float)viewport[3] - .5f;
	g_eyes.setPOI( 10.f*px, 0.f, 10.*(py+.5f) );

	printf("\nClicked %4.2f, %4.2f\n",px,py);

	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
//  main()
//-----------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	XnStatus				rc;
	xn::EnumerationErrors	errors;

	// Create an OpenNI context with default settings
	rc = g_context.InitFromXmlFile( OPENNI_CONFIG_XML_PATH, 
	                                g_scriptNode, &errors );
	if (rc == XN_STATUS_NO_NODE_PRESENT)
	{
		XnChar strError[1024];
		errors.ToString(strError, 1024);
		std::cerr << "Creating OpenNI context failed:\n" << strError << "\n";
		return (rc);
	}
	else 
		CHECK_RC(rc,"Creating OpenNI context");
	
	// Find depth generator
	rc = g_context.FindExistingNode( XN_NODE_TYPE_DEPTH, g_depthGenerator );
	CHECK_RC(rc, "No OpenNI depth node!");

	// Find image generator
	rc = g_context.FindExistingNode( XN_NODE_TYPE_IMAGE, g_imageGenerator );
	CHECK_RC(rc, "No OpenNI image node!");

	// Hybrid mode isn't supported in this sample (?)
	xn::DepthMetaData depthMD;
	xn::ImageMetaData imageMD;
	if (imageMD.FullXRes() != depthMD.FullXRes() || 
		imageMD.FullYRes() != depthMD.FullYRes())
	{
		printf ("The device depth and image resolution must be equal!\n");
		return 1;
	}

	// Setup HandTracker
	g_handTracker = new HandTracker( g_context );

	rc = g_handTracker->Init();
	CHECK_RC(rc, "HandTracker::Init()");

	rc = g_handTracker->Run();
	CHECK_RC(rc, "HandTracker::Run()");

	// We're done configuring it. Make it start generating data
	rc = g_context.StartGeneratingAll();
	CHECK_RC(rc, "StartGeneratingAll()");

#if 1
	// GLUT application
	std::cout << "Everything went smooth, starting tracking\n"
		      << "Press ESC to quit\n";
	
	atexit( destroy );

	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_SINGLE );
	glutInitWindowSize( 512, 512 );
	glutCreateWindow( "Little GLUT Test" );
	
	glutIdleFunc( glutIdle );
	glutDisplayFunc( glutRender );
	glutReshapeFunc( glutReshape );
	glutKeyboardFunc( glutKeyboard );
	
	glutMainLoop();	// never returns

#else
	// Console application
	std::cout << "Everything went smooth, starting tracking\n"
		      << "Press any key to stop\n";
	// Main loop
	while( !xnOSWasKeyboardHit() )
	{
		update();
	}
	destroy();
#endif
	return EXIT_SUCCESS;
}
