// livespecgl - audio vis. after [Monro&Pressing 1998], Max Hermann, Sep. 2011
// Wintermute version (Dec 2012)
// - added MonroPressingPhaseSpace
// - added SoundInput wrapper for BASS
// - removed bass_fx dependency (lowpass filter was not working)
#include <cstdlib> // atexit()
#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <ctime>		// time()
#include <vector>
#include <string>
#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif

// Trackball2 is the only dependency on glutils and glm so far, so it could make
// sense for code portability to uncomment it.
#define USE_TRACKBALL
#ifdef USE_TRACKBALL
#include <glutils/Trackball2.h>
#endif

#include "gl2ps.h";

#include "SoundInput.h"
#include "MonroPressingPhaseSpace.h"
#include "IlluminatedLinesRenderer.h"


const char usage[] = "\
livespecgl by www.386dx25.de, 2011 \n\
2D sound embedding as in Monro&Pressing 1998\n\
Keys:\n\
	F    toggle fullscreen      \n\
	f    low pass filter        \n\
	k/K  adjust phase shift     \n\
	m    points / lines / linestrip	\n\
	p/P  point size             \n\
	l/L  line width             \n\
	s/S  scale embedding        \n\
	C    toggle color           \n\
	a    toggle rotation        \n\
	\n\
	#    toggle phase space update \n\
	+    cycle through shaders  \n\
	\n\
	<space> pause music         \n\
	<esc>   exit                \n\
";


const unsigned long     BUFLEN = 8*2048;  // size of sample buffer
const int               K_MAX  = 128;
short                   g_buf[BUFLEN];    // sample buffer

bool                    g_keys[256];

std::vector<float>      g_colors;

MonroPressingPhaseSpace g_pspace;
SoundInput              g_sound;
IlluminatedLinesRenderer g_ilren;
IlluminatedLinesShader  g_ilshader;
TangentColorShader      g_tanshader;

#ifdef USE_TRACKBALL
Trackball2              g_trackball;
#endif

// init is called with valid GL context
bool init()
{
	if( !g_tanshader.init() ) return false;
	if( !g_ilshader.init() ) return false;

	glEnable( GL_LINE_SMOOTH );
	glLineWidth( 1.2f );
	glEnable( GL_POINT_SMOOTH );
	glPointSize( 2.3f );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); 
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );

	return true;
}

void destroy()
{
	g_ilshader.deinit();
	g_tanshader.deinit();
}

// leave musicFile empty to use line/microphone source
int setup_sound( std::string musicFile )
{
	using namespace std;

	if( !g_sound.setupDevice() )
		return -1;

	if( !musicFile.empty() )
	{
		// Use mp3 file as input and play it as well
		if( !g_sound.openFile(musicFile.c_str()) )
			return -2;
		
		cout << ">>>>>>> Playing \"" << musicFile << "\"\n" << endl;
	}
	else
	{
		// Record input from any source (e.g. microphone)
		if( !g_sound.openInput() )
			return -3;

		cout << ">>>>>>> Recording input\n" << endl;
	}

	// Start playback or recording (although not required for recording?)
	g_sound.startInput();
}

void rainbow( int j, int n, float& r, float& g, float &b )
{
	float i = j / (float)n; //j*255.f / n;

	float rad = i*2.*M_PI; // / 256.;
	if( rad <= 2.*M_PI/3. )
		r = cos( .75*rad );
	else
	if( rad >= 4.*M_PI/3. )
		r = cos( .75*( 2.*M_PI - rad ) );
	else
		r = 0.;

	g = std::max( 0., sin( .75*rad ) );
	b = std::max( 0., sin( .75 * (rad - 2.*M_PI/3.) ) );
}

void setup_colors( int N )
{
	if( N*4 == g_colors.size() )
		return;

	g_colors.resize(4*N);
	for( int i=0; i < N; ++i )
	{
		float r,g,b;
		rainbow( (N-i),N, r,g,b );
		g_colors[4*i+0] = 3.*r;
		g_colors[4*i+1] = 3.*g;
		g_colors[4*i+2] = 3.*b;
		g_colors[4*i+3] = 0.2;
	}
}

void display(); // forward
void update( int data )
{
	using namespace std;
	unsigned long err;

	// get samples	
	short* buf = &g_buf[0];

	// Workaround for k-overlap:
	// - copy last k samples from last update to the buffer front
	// - fetch new data starting behind this k-overlap beginning
	for( int i=0; i < K_MAX; ++i )
		buf[i] = buf[BUFLEN-K_MAX+i];

	int numBytesRead = 
		g_sound.pollSampleData( &buf[K_MAX], (BUFLEN-K_MAX) );

	// playback controls
	if( g_keys[' '] ) { g_sound.togglePause(); g_keys[' '] = false;	}
	if( g_keys['R'] ) {	g_sound.restart();     g_keys['R'] = false;	}

	// update phase space
	if( !g_keys['#'] )
		g_pspace.pollWaveSamples( buf, numBytesRead/2 ); //BUFLEN );

	// update colors
	setup_colors( g_pspace.getNumPoints() );

	// redraw
	display();

	// 25ms = 40Hz
	glutTimerFunc( 25, update, 0 );
}

void idle()
{
	// nop
}

void draw_phaseSpace( int mode=0 )
{
	const int KEYNUM = 5;
	const char keyset[KEYNUM] = {'k','K','s','S'};

	if( g_keys['k'] ) g_pspace.setPhaseShift( g_pspace.getPhaseShift()-1 );
	if( g_keys['K'] ) g_pspace.setPhaseShift( g_pspace.getPhaseShift()+1 );

	if( g_keys['s'] ) g_pspace.setScale( g_pspace.getScale() * 0.9 );
	if( g_keys['S'] ) g_pspace.setScale( g_pspace.getScale() * 1.1 );

	for( int i=0; i < KEYNUM; ++i ) g_keys[keyset[i]] = false;

	GLenum prim;
	switch( mode ) {
		default:
		case 0 : prim = GL_POINTS; break;
		case 1 : prim = GL_LINE_STRIP; break;
		case 2 : prim = GL_LINES; break;
	}

	if( g_keys['C'] ) {
		glEnableClientState( GL_COLOR_ARRAY );		
		glColorPointer( 4, GL_FLOAT, 0, &g_colors[0] );
	}

#if 1
	glEnableClientState( GL_NORMAL_ARRAY );
	glNormalPointer( GL_FLOAT, 0, g_pspace.getTangentBuffer() );
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, g_pspace.getPointBuffer() );
	glDrawArrays( prim, 0, g_pspace.getNumPoints() );
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
#else
	glEnableClientState( GL_VERTEX_ARRAY );	
	glVertexPointer( 3, GL_FLOAT, 0, g_pspace.getPointBuffer() );	
	
	glDrawArrays( prim, g_pspace.getStartPoint(), g_pspace.getLenFront() );	

	glColorPointer( 4, GL_FLOAT, 0, &g_colors[4*g_pspace.getLenFront()] );
	glDrawArrays( prim, 0, g_pspace.getLenBack() );

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
#endif
}

float orthoOn()
{
	int viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	float asp = viewport[2]/(float)viewport[3];
	
	// set ortho projection for 2D hud
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( -asp,asp, -1,1, -1,1 );
	glMatrixMode( GL_MODELVIEW );

	return asp;
}

void orthoOff()
{
	// restore projection matrix
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
}

// draw progress when playing from file
void draw_playbackProgress()
{
	float asp = orthoOn();

	// draw progress bar
	float t = g_sound.getStreamProgress();
	glLoadIdentity();
	glColor3f( 1,1,1 );
	glBegin( GL_QUADS );
	glVertex2f( -asp,-.95 );
	glVertex2f( -asp,-1. );
	glVertex2f( -asp+2.*asp*t,-1. );
	glVertex2f( -asp+2.*asp*t,-.95 );
	glEnd();

	orthoOff();
}

void draw_waveform( short* buf, int n )
{
	float asp = orthoOn();
	const float y0 = 0.95f;
	const float height = .15f;

	// draw limits min,max,mean
	glColor3f( .3, .3, .3 );
	glBegin( GL_LINES );
	glVertex2f( -asp        , y0-.5f*height + height*.5f );
	glVertex2f( -asp+2.f*asp, y0-.5f*height + height*.5f );
	glVertex2f( -asp        , y0-.5f*height - height*.5f );
	glVertex2f( -asp+2.f*asp, y0-.5f*height - height*.5f );
	glVertex2f( -asp        , y0-.5f*height );
	glVertex2f( -asp+2.f*asp, y0-.5f*height );
	glEnd();

	// draw waveform
	enum WaveformType { Wavelab, Wavegraph } waveType = Wavegraph;
	glColor3f( 1,1,1 );
	glLineWidth( 1.2f );
	glBegin( (waveType==Wavelab) ? GL_TRIANGLE_STRIP : GL_LINE_STRIP );
	for( int i=0; i < n; i++ )
	{
		// sample in [-1,1]
		float sample = (float)buf[i] / (.5f*65536.f);

		// decibels 
		float sign = (sample > 0.0) ? 1.0 : -1.0;
		sample = log(1+fabs(2*sample));

		float dx = i / (float)(n-1),
			  dy = sign*sample;

		//glColor3fv( &g_colors[4*i] );
		glVertex2f( -asp+2.f*asp*dx, y0-.5f*height + height*dy );
		if( waveType==Wavelab )
			glVertex2f( -asp+2.f*asp*dx, y0-.5f*height );
	}
	glEnd();

	orthoOff();
}

void display()
{
	static bool fullscreen = false;
	if( g_keys['F'] ) 
	{
		fullscreen = !fullscreen;
		g_keys['F'] = false;

		if( fullscreen )
			glutFullScreen();
		else
		{
			glutReshapeWindow( 512, 512 );
			glutPositionWindow( 100, 100 );
		}
	}

	// GL states
	static float pointSize = 2.3f;
	if( g_keys['P'] ) { pointSize*=1.1; g_keys['P']=false; }
	if( g_keys['p'] ) { pointSize*=0.9; g_keys['p']=false; }
	glPointSize( pointSize );

	static float lineWidth = 1.2f;
	if( g_keys['L'] ) { lineWidth*=1.1; g_keys['L']=false; }
	if( g_keys['l'] ) { lineWidth*=0.9; g_keys['l']=false; }
	glLineWidth( lineWidth );

	if( !g_keys['W'] ) glClearColor( 0,0,0,1 ); else glClearColor( 1,1,1,1 );
	glClear( GL_COLOR_BUFFER_BIT );
	glColor4f( 1,1,1,0.6 );

	// camera
	glLoadIdentity();
#ifdef USE_TRACKBALL	
	glm::mat4 modelview = g_trackball.getCameraMatrix();
	glMultMatrixf( &modelview[0][0] );
#endif

	// animation
	static float angle=0.f;	
	if( !g_keys['a'] )
	{
		angle += 1.f; // assume constant update speed of 25hz
		glRotatef( angle, 1, 1, 1 );
	}

	// draw phase space
	
	static int mode=0;
	if( g_keys['m'] ) { mode++; mode%=4; g_keys['m']=false; }

	static int shader = 0;
	if( g_keys['+'] ) { shader = (shader+1)%2; g_keys['+']=false; }
	switch( shader ) 
	{
	case 1: g_ilshader.bind(); break;
	case 2:	g_tanshader.bind();  break;
	}

	draw_phaseSpace( mode%3 );
	if( mode==3 ) draw_phaseSpace( 1 );

	// release any shader
	glUseProgram( 0 );


	if( !g_keys['h'] )
	{
		// draw progress at bottom (when playing from file)
		draw_playbackProgress();

		// draw waveform at top
		draw_waveform( &g_buf[0], BUFLEN );
	}

	glutSwapBuffers();
}

#define IMG_PREFIX "livespecgl2"
void export_PS()
{
	FILE *fp;
	int state = GL2PS_OVERFLOW, buffsize = 0;
	
	time_t seconds = time(NULL);
	
	char filename[1024];
	sprintf( filename, "%s-%d.pdf", IMG_PREFIX, seconds );
	
	fp = fopen( filename, "wb" );
	if( !fp )
	{
		std::cerr << "Couldn't open file '" << filename << "'!\n";
		return;
	}

	printf( "Generating Postscript output...\n" );
    while( state == GL2PS_OVERFLOW )
	{
      buffsize += 1024*1024;
	  gl2psLineWidth( 1 );
      gl2psBeginPage( IMG_PREFIX, "www.386dx25.de", 
		             NULL, GL2PS_PDF, GL2PS_BSP_SORT, 
                     GL2PS_USE_CURRENT_VIEWPORT | GL2PS_NO_BLENDING | 
					 GL2PS_OCCLUSION_CULL | GL2PS_NO_PS3_SHADING, // | GL2PS_DRAW_BACKGROUND
                     GL_RGBA, 0, NULL, 0, 0, 0,  buffsize, fp, filename );
      
	  display();
	  
      state = gl2psEndPage();
    }
    fclose(fp);
    printf( "Current GL-Viewport exportet to %s\n", filename );
}

void reshape( GLint w, GLint h )
{
	float asp = (float)w/h;
	glViewport( 0,0, w,h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	//glOrtho( -asp,asp, -1,1, -1,1 );
	gluPerspective( 45, asp, 0.1, 42 );
	glMatrixMode( GL_MODELVIEW );

#ifdef USE_TRACKBALL
	g_trackball.setViewSize( w, h );
#endif
}

void onexit()
{
	destroy();
	std::cout << "Bye!\n";
}

void keyboard( unsigned char key, int x, int y )
{	
	switch( key )
	{
		case 27:
		case 'q':
			exit(0);
			break;

		case 'E':
			export_PS();
			break;

		default:
			g_keys[key] = !g_keys[key];
	}
}

void mouse( int button, int state, int mousex, int mousey )
{
#ifdef USE_TRACKBALL
	if( state==GLUT_UP )
	{
		g_trackball.stop();
	}
	else
	if( (button==GLUT_LEFT_BUTTON)&&(state==GLUT_DOWN) )
	{
		g_trackball.start( mousex, mousey, Trackball2::Rotate );
	}
	else
	if( (button==GLUT_RIGHT_BUTTON)&&(state==GLUT_DOWN) )
	{
		g_trackball.start( mousex, mousey, Trackball2::Zoom );
	}
	else
	if( (button==GLUT_MIDDLE_BUTTON)&&(state==GLUT_DOWN) )
	{
		g_trackball.start( mousex, mousey, Trackball2::Translate );
	}
#endif
}

void motion( int mousex, int mousey )
{
#ifdef USE_TRACKBALL
	g_trackball.update( mousex, mousey );
#endif
}

int main( int argc, char* argv[] )
{
	using namespace std;

	cout << usage << endl;

	// --- options ---
	string musicFile;  // play this mp3 instead of record input from microphone

	// --- parse command line ---
	vector<string> positional;
	for( int i=1; i < argc; ++i )
	{
		string cur( argv[i] );
		if( cur.find("-") == 0 )
		{
			cout << "Unknown argument \"" << cur << "\" will be ignored!\n";
		}
		else
			positional.push_back( cur );
	}

	// assign positional arguments
	if( positional.size() >= 1 )
	{
		// first argument is mp3 filename
		musicFile = positional[0];
		
		if( positional.size() > 1 )
			cout << "Too many input arguments!\n";
	}

	// --- setup sound input ---
	int err = setup_sound( musicFile );
	if( err < 0 )
		return err;

	// --- setup GLUT ---
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );
	glutInitWindowSize( 512, 512 );
	glutCreateWindow( "livespecgl" );	
	
	glutDisplayFunc ( display );
	glutIdleFunc    ( idle );
	glutReshapeFunc ( reshape );
	glutKeyboardFunc( keyboard );
	glutMouseFunc   ( mouse );
	glutMotionFunc  ( motion );

	// --- setup GLEW ---
	glewExperimental = GL_TRUE;	
	GLenum glew_err = glewInit();
	if( glew_err != GLEW_OK )
	{
		std::cerr << "GLEW error:" << glewGetErrorString(glew_err) << std::endl;
		throw std::runtime_error( "Error on initializing GLEW library!" );
	}


	// Register exit callback
	atexit( onexit );

	// User init
	if( !init() )
	{
		return -1;
	}

	// Initial update
	update(0);	

	glutMainLoop();	
	return 0;
}
