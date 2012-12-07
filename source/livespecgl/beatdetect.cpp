// beatdetect - simple beat detector, Max Hermann, Dec. 2011
// better alternatives:
// http://aubio.org/, http://www.vamp-plugins.org
#include <cstdlib> // atexit()
#include <GL/glut.h>
#include <bass.h>
#include <iostream>
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// forwards
class BeatDetector;

//-----------------------------------------------------------------------------
//	globals
//-----------------------------------------------------------------------------

HRECORD g_chan;	// recording channel

const unsigned long BUFLEN=2048;  // size of sample buffer

bool g_keys[256];

BeatDetector* g_bd = NULL;

const unsigned int FFTLEN=1024;
float g_fft[FFTLEN];



//-----------------------------------------------------------------------------
//	EnergyTracker
//-----------------------------------------------------------------------------
class EnergyTracker
{
public:
	EnergyTracker( int buflen_=43, float sensibility_=1.3f )
	{
		if( (buflen_>0) && (buflen_<1000000) )
			buflen = buflen_;
		else buflen=43;

		history = new double[buflen];
		pos = 0;

		for( int i=0; i < buflen; i++ ) history[i]=0.;

		beat = false;
		sensibility = sensibility_;
	};

	~EnergyTracker()
	{
		delete [] history;
	};

	void hit( double instantEnergy, bool checkbeat=true )
	{
		pos = (pos+1)%buflen;
		history[pos] = instantEnergy;

		// stupid variant ;-)
		//memcpy( &history[1], &history[0], sizeof(double)*(buflen-2) );	
		//history[0] = instantEnergy;

		if( checkbeat )
			beat = (instantEnergy > sensibility*getAverage());
	};

	bool isBeat()
	{
		return beat;
	};
	
	void setSensibility( float s ) { sensibility = s; }	
	float getSensibility() const { return sensibility; }

	double getAverage()
	{
		double E=0;
		for( int k=0; k < buflen; k++ ) E += history[k]*history[k];
		E /= (double)buflen;
		return sqrt(E);
	};

private:
	// get() functions really needed?
	double get( int i )
	{
		if( (i>=0) && (i<buflen) ) return history[i];
		else return 0;
	};
	
	double getLast()
	{
		return history[pos];
	}

private:
	double* history;
	int     buflen;
	int     pos;
	
	bool    beat;
	double  sensibility;
};

//-----------------------------------------------------------------------------
//	BeatDetector
//-----------------------------------------------------------------------------
class BeatDetector
{
public:
	BeatDetector(): m_power(0.0) 
	{
		m_et = new EnergyTracker( 43, 1.7 );
	}
	
	~BeatDetector()
	{
		delete m_et;
	}

	void setSensibility( float s ) { m_et->setSensibility(s); }	
	float getSensibility() const { return m_et->getSensibility(); }
	
	// assume 40Hz
	void poll( short* buf, int len, float dt=1/40.f )
	{
		// compute instant energy
		double instantEnergy = 0.;
		for( int i=0; i < len; ++i )
			instantEnergy += buf[i]*buf[i];

		// detect beat
		m_et->hit( instantEnergy, true );
		if( m_et->isBeat() ) m_power = 1.;

		// dampen power
		if( m_power > 0. ) m_power -= 100.*dt*dt;		
	}
	
	double getPower() const { return m_power; }
	
private:
	EnergyTracker* m_et;
	double m_power;
};


//-----------------------------------------------------------------------------
//	BASS stuff
//-----------------------------------------------------------------------------

// Recording callback - not doing anything with the data
BOOL CALLBACK DuffRecording(HRECORD handle, const void *buffer, DWORD length, void *user)
{
	return TRUE; // continue recording
}

void checkBASSError( unsigned long err )
{
	using namespace std;

	if( err==0xFFFFFFFF )
	{
		cerr << "BASS error: ";
		err=BASS_ErrorGetCode();
		if( err != BASS_OK )
		{
			if( err==BASS_ERROR_HANDLE  ) cerr << "Music stream is not a valid handle! ";
			if( err==BASS_ERROR_NOPLAY  ) cerr << "Channel is not playing! ";
			if( err==BASS_ERROR_ILLPARAM) cerr << "Buffer length is invalid! ";
			if( err==BASS_ERROR_MEM     ) cerr << "Insufficient memory! ";
			if( err==BASS_ERROR_BUFLOST ) cerr << "Memory leak detected! ";
		}
		cerr << endl;
	}
}

//-----------------------------------------------------------------------------
//	GLUT callbacks
//-----------------------------------------------------------------------------

void init()
{
	g_bd = new BeatDetector();
}

void display();
void update( int data )
{
	unsigned long err;
	
	// get samples
	static short buf[BUFLEN];
	err = BASS_ChannelGetData( g_chan, buf, BUFLEN*sizeof(short) );
	checkBASSError( err );
	
	// get FFT
	err = BASS_ChannelGetData( g_chan, g_fft, BASS_DATA_FFT2048 );
	checkBASSError( err );
	//for( int i=0; i < FFTLEN; ++i )
	//	g_fft[i] = 0.5;

	// beat detector
	if( g_bd )
	{
		g_bd->poll( buf, BUFLEN );
	}
	
	// redraw
	display();

	// 25ms = 40Hz
	glutTimerFunc( 25, update, 0 );	
}

void idle()
{
	if( g_bd )
	{
		bool u=false;
		float s = g_bd->getSensibility();
		if( g_keys['s'] ) { g_bd->setSensibility( s-0.1 ); g_keys['s']=false; u=true; }
		if( g_keys['S'] ) { g_bd->setSensibility( s+0.1 ); g_keys['S']=false; u=true; }
		if( u )
			std::cout << "sensibility = " << g_bd->getSensibility() << "\n";
	}
}

void draw_quad( float x0, float y0, float x1, float y1 )
{
	glBegin( GL_QUADS );
	glVertex2f( x0, y0 );
	glVertex2f( x1, y0 );
	glVertex2f( x1, y1 );
	glVertex2f( x0, y1 );
	glEnd();
}

void rainbow( int j, int n, float& r, float& g, float &b )
{
	float i = j*255.f / n;

	float rad = i*2.*M_PI / 256.;
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

void display()
{
	using namespace std;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();

	float s = 1.f;
	if( g_bd )
		s = g_bd->getPower();

	glColor3f( 1,1,1 );
	draw_quad( -.5*s,-.5*s, .5*s,.5*s );

	// "normal" FFT

	static int FLEN = 368/2;   // display only first 386/2 bins of 1024 FFT
	if( g_keys[','] ) { FLEN-=10; g_keys[',']=false; cout<<"flen="<<FLEN<<"\n";}
	if( g_keys['.'] ) { FLEN+=10; g_keys['.']=false; cout<<"flen="<<FLEN<<"\n";}

	static float spectrum[FFTLEN];
	for( int i=0; i < FLEN; ++i )
	{
		// sqrt scaling to emphasize low values
		spectrum[i] = sqrt(g_fft[i])*3;
	}

	float delta = 2.f/(FLEN-1);
	static float ampscale = 2.f;
	if( g_keys[':'] ) { ampscale*=1.1; g_keys[':']=false; cout<<"ampscale="<<ampscale<<"\n";}
	if( g_keys[';'] ) { ampscale*=0.9; g_keys[';']=false; cout<<"ampscale="<<ampscale<<"\n";}
	for( int i=1; i < FLEN-1; ++i )
	{
		float x0 = i*delta;
		
		float r,g,b;
		rainbow( i, FFTLEN-1, r,g,b );
		glColor3f( r,g,b );

		// smooth spectrum
		float val = (spectrum[i-1]+spectrum[i]+spectrum[i+1])/3.;

		draw_quad( x0-1, -1, x0-1+delta, -1+ampscale*val );
	}

	// draw band separation
	static int BANDS = 7;
	if( g_keys['b'] ) { BANDS--; g_keys['b']=false; cout<<"bands="<<BANDS<<"\n"; }
	if( g_keys['B'] ) { BANDS++; g_keys['B']=false; cout<<"bands="<<BANDS<<"\n"; }

	glColor3f( .8, .8, .8 );
	glBegin( GL_LINES );
	for( int b=0; b < BANDS; ++b )
	{
		// exponentially sized bins
		float lglen = log((double)FFTLEN)/log(2.0); // log basis 2 (FFTLEN is power of 2)
		float db = b/(float)(BANDS-1);
		int b1 = std::min((float)FFTLEN-1.f, pow(2.f,db*lglen));
		float x0 = b1/(float)FLEN;

		glVertex2f( -1+x0, -1 );
		glVertex2f( -1+x0, +1 );
	}
	glEnd();
	
	glFlush();
}

void reshape( GLint w, GLint h )
{
	float asp = (float)w/h;
	glViewport( 0,0, w,h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( -asp,asp, -1,1, -1,1 );
	glMatrixMode( GL_MODELVIEW );
}

void onexit()
{
	using namespace std;
	cout << "cu" << endl;
	delete g_bd;
	BASS_RecordFree();
}

void keyboard( unsigned char key, int x, int y )
{	
	switch( key )
	{
		case 27:
		case 'q':
			exit(0);
			break;

		default:
			g_keys[key] = !g_keys[key];
	}
}

//-----------------------------------------------------------------------------
//	main
//-----------------------------------------------------------------------------

int main( int argc, char* argv[] )
{
	using namespace std;

	// --- setup BASS ---

	// check the correct BASS was loaded
	if( HIWORD(BASS_GetVersion()) != BASSVERSION ) 
	{
		cerr << "Error: An incorrect version of BASS.DLL was loaded (2.4 is required)." << endl;
		return -1;
	}

	if( !BASS_RecordInit(-1) )
	{
		cerr << "Error: BASS can't initialize device!" << endl;
		return -3;
	}

	// start recording (44100hz mono 16-bit)
	if( !(g_chan=BASS_RecordStart(44100,1,0,&DuffRecording,0)) ) 
	{
		cerr << "Error: BASS can't start recording!" << endl;
		return -4;
	}


	// --- setup GLUT ---
	
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_SINGLE );
	glutInitWindowSize( 512, 512 );
	glutCreateWindow( "beatdetect" );	
	
	glutDisplayFunc ( display );
	glutIdleFunc    ( idle );
	glutReshapeFunc ( reshape );
	glutKeyboardFunc( keyboard );

	atexit( onexit );
	
	// --- init GL ---
	
	glEnable( GL_LINE_SMOOTH );
	glLineWidth( 1.2f );
	glEnable( GL_POINT_SMOOTH );
	glPointSize( 1.3f );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); 
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );


	init();

	update(0);
	glutMainLoop();
	return 0;
}
