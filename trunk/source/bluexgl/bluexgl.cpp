// bluexgl - OpenGL port of one of my oldest pieces of software.
//           (glvidfx already does this in OpenGL but is not standalone)
// Max Hermann, Jan 2014
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/GL.h>
#include <GL/glut.h>
#include <vector>
#include <iostream>

//------------------------------------------------------------------------------
//	A simple OpenGL application interface
//------------------------------------------------------------------------------

class GLAppInterface
{
public:
	virtual int  create() { return 1; };
	virtual void destroy(){};
	
	virtual void initgl() {}
	virtual void reshape( int width, int height ) = 0;
	virtual void render() = 0;
	virtual void update( double dt ) {}
		
	virtual void keypress( unsigned char key, int x, int y ) {}
};

//------------------------------------------------------------------------------
//	OpenGL 2D screen buffer application interface
//------------------------------------------------------------------------------

class GLApp2DScreen :  public GLAppInterface
{
public:
	GLApp2DScreen()
	: m_dirty(true), m_resized(true)
	{}
	
	virtual int create()
	{
		// create screen texture
		glGenTextures( 1, &m_texid );
		glBindTexture( GL_TEXTURE_2D, m_texid );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
		
		return 1;
	}
	
	virtual void initgl()
	{
		glEnable( GL_TEXTURE_2D );

		glEnable( GL_LINE_SMOOTH );
		glEnable( GL_POINT_SMOOTH );
		glLineWidth( 1.f );
		glPointSize( 1.f );
		
		glDisable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA );

		glClearColor( 0,1,0,1 );
		glColor4f( 1,1,1,1 );
	}
	
	virtual void reshape( int width, int height )
	{
		resizeScreen( width/4, height/4 );
		
		glViewport( 0,0, width,height );
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		glOrtho( -1,1,-1,1, -1,1 ); // (left,right,bottom,top) 
		glMatrixMode( GL_MODELVIEW );	
		glLoadIdentity();
	}
	
	virtual void render()
	{
		if( m_buffer.empty() ) return;

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
		// Set screen texture
		if( m_dirty ) 
		{
			// Download buffer to texture (binds texture as well)
			downloadScreen();
			m_dirty = false;
		}
		else
			// Only bind texture (has not changed since last call)
			glBindTexture( GL_TEXTURE_2D, m_texid );

		// Render textured quad
		glBegin( GL_TRIANGLE_FAN );
		glTexCoord2f(0,1); glVertex2f( -1,-1 );
		glTexCoord2f(1,1); glVertex2f(  1,-1 );
		glTexCoord2f(1,0); glVertex2f(  1, 1 );
		glTexCoord2f(0,0); glVertex2f( -1, 1 );
		glEnd();	
	}
	
	virtual void update( double dt ) = 0;

	void updateScreen()
	{
		m_dirty = true;
	}
	
	void* getScreenPtr()
	{
		return (void*)&(m_buffer[0]);
	}
	
	int getWidth()  const { return m_width;  }
	int getHeight() const { return m_height; }
	
protected:	
	void resizeScreen( int w, int h )
	{
		m_width  = w;
		m_height = h;
		m_buffer.resize( w*h * 3 );
		
		m_resized = true;
	}
	
	void downloadScreen()
	{
		glBindTexture( GL_TEXTURE_2D, m_texid );
		if( m_resized )
		{
			// a resized texture requires a full textimage call ...
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 
			              0, GL_RGB, GL_UNSIGNED_BYTE, getScreenPtr() );
			m_resized = false;
		}
		else
			// ... while subimage is considered to be faster
			glTexSubImage2D( GL_TEXTURE_2D, 0, 0,0, m_width, m_height, GL_RGB,
							 GL_UNSIGNED_BYTE, getScreenPtr() );
	}	
		
private:
	std::vector<unsigned char> m_buffer; /// RGB uchar screen buffer
	int m_width, m_height;
	GLuint m_texid;
	bool m_dirty;
	bool m_resized;
};


//------------------------------------------------------------------------------
//	BlueX application
//------------------------------------------------------------------------------

// "chicken" filter
void filter( int width, int height, unsigned char* data, float chick )
{
	static bool flip=true;
#if 0
	flip=!flip;
#endif
	for( int i=width+1; i < width*height-width-1; ++i )
	{
		int n = flip ? i : (width*height-width-2-i);
		
		unsigned char* d = data;
#if 1
		d[3*n+0] = (d[3*n-3+0] + d[3*n+3+0] + d[3*n-3*width+0] + d[3*n+3*width+0] + (int)(chick)) >> 2;
		d[3*n+1] = d[3*n+0];
		d[3*n+2] = d[3*n+0];
#else
		d[3*n+0] = (d[3*n-3+0] + d[3*n+3+0] + d[3*n-3*width+0] + d[3*n+3*width+0] + (int)(17.f*chick)) >> 2;
		d[3*n+1] = (d[3*n-3+1] + d[3*n+3+1] + d[3*n-3*width+1] + d[3*n+3*width+1] + (int)(13.f*chick)) >> 2;
		d[3*n+2] = (d[3*n-3+2] + d[3*n+3+2] + d[3*n-3*width+2] + d[3*n+3*width+2] + (int)(11.f*chick)) >> 2;
#endif
#if 0		
		d[3*n+0] = d[3*n+0] % (d[3*n+1]%254+1);
		d[3*n+1] = d[3*n+1] % (d[3*n+2]%254+1);
		d[3*n+2] = d[3*n+0] % (d[3*n+0]%254+1);
#endif 
	}
}

class BlueX : public GLApp2DScreen
{
public:
	virtual void update( double dt_ )
	{
		static double dt = 0;
		dt += dt_;
		if( dt < 0.032 ) return; // update at max. 30Hz
	
		if( !m_paused )
			m_chicken += dt;

		filter( getWidth(), getHeight(), (unsigned char*)getScreenPtr(), 0.01*m_chicken );
		updateScreen();
	}

	void keypress( unsigned char key, int x, int y ) 
	{
		if( key == ' ' ) {
			m_paused = !m_paused;
			if( m_paused )
				std::cout << (int)m_chicken << std::endl;
		}
	}

private:
	double m_chicken;
	bool m_paused;
};


//------------------------------------------------------------------------------
//	Program setup
//------------------------------------------------------------------------------

// Globals
BlueX g_bluex; 
GLAppInterface* g_app = &g_bluex;

int init()
{
	// Here, we seem not to have a valid OpenGL context required for app init,
	// therefore create() and initgl() calls are now executed on first call
	// to the global update() function.

	//int errcode = g_app->create();
	//if( errcode ) return errcode;
	//g_app->initgl();
	return 1;
}

void destroy()
{
}

//------------------------------------------------------------------------------
//	GLUT callbacks
//------------------------------------------------------------------------------

bool g_initialized = false;

void update()
{
	static bool firstcall = true;
	if( firstcall )
	{
		g_app->create();
		g_app->initgl();
		firstcall = false;
		g_initialized = true;
	}

	// elapsed time dt
	static double t_base = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	const  double t_now  = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	const double dt = t_now - t_base;
	if( dt < 0.008 ) return; // limit update frequency to 120Hz
	t_base = t_now;

	// updates (e.g. for animation)
	g_app->update( dt );

	glutPostRedisplay();
}

void render()
{
	if( !g_initialized ) return;
	g_app->render();
	glutSwapBuffers();
}

void reshape( int width, int height )
{
	g_app->reshape( width, height );
	glutPostRedisplay();
}

void keyboard( unsigned char key, int x, int y )
{
	g_app->keypress( key, x, y );

	switch( key )
	{
	case 27:
		exit(0);
	}
}

//------------------------------------------------------------------------------
//	main
//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	// setup GLUT
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( 512, 512 );
	glutCreateWindow( "bluexgl" );
	glutReshapeFunc ( reshape  );
	glutKeyboardFunc( keyboard );
	glutDisplayFunc ( render   );
	glutIdleFunc    ( update   );
	
	// init application
	int ret = init();
	if( ret < 0 )
		return ret;

	// exit function
	atexit( destroy );

	// mainloop
	glutMainLoop();
	
	// never reached, glutMainLoop() quits with exit()
	return 0;
}