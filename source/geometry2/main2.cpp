#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>          // time(), clock()
#include <cstdlib>        // rand()
#include <cstdio>         // printf()
#include <vector>
#include <GL/glew.h>
#include "Geometry2.h"
#include "Screenshot2.h"
#include "gl2ps.h"

#include <GL/glut.h>

using namespace std;

const int MAX_LEVELS = 6;
const int IMG_WIDTH  = 8096;
const int IMG_HEIGHT = 8096;
const char IMG_PREFIX[] = "geometry-";

const char usage[] = "\
geometry2 by www.386dx25.de, 2011 \n\
Keys:\n\
	1    Icosahedron            \n\
	2    Penrose tiling         \n\
	3    Superquadric           \n\
	\n\
	w    wireframe				\n\
	s    shade model			\n\
	i    light inside/outside	\n\
	l    lighting				\n\
	c    color					\n\
	B    switch black/white		\n\
	m    immediate mode			\n\
	(/)  line width				\n\
	\n\
	r    reset parameters       \n\
	R    randomize parameters   \n\
	x/X  adjust parameter 1		\n\
	y/Y  adjust parameter 2		\n\
	\n\
	,/.  adjust subdivision level \n\
	\n\
	p    export Postscript		\n\
	S    high-res output    	\n\
";

// globals
SimpleGeometry*    g_geomPtr = NULL;
Penrose            g_geomPenrose;
Superquadric       g_geomSuperquadric;
Icosahedron        g_geomIco;
SphericalHarmonics g_geomSH;

std::vector<float> g_colors;
double g_linewidth = 1.0;
std::string g_switches = "wsilcB";  // switches for serialization

Screenshot2 g_screenshot;
extern void reshape( int width, int height ); // defined in glutmain.cpp

// lighting
GLfloat light0_position[] = {0.0,0.0,-1.0, 0.0 };

// color modes
enum ColorFunc { RandomColors=0, RainbowGradient, GrayGradient, RandomTint, 
                 NumColorModes };
int g_colorMode = RandomTint;

// forwards
void setupLight();
void setupColors( int n, std::vector<float>& colors, int func );
void export_ps();
void draw_immediate( SimpleGeometry* geom, float* colorPtr=NULL );
void draw_array( SimpleGeometry* geom, float* colorPtr=NULL );
void save_state( const char* filename );
bool load_state( const char* filename );

float frand()
{
	static bool firstCall = true;
	if( firstCall ) {
		srand( (unsigned int)time(NULL) );
		firstCall = false;
	}

	return (float)(rand()%RAND_MAX)/(float)RAND_MAX;
};

template<typename T> T clamp( const T& val, T min, T max )
{
	if( val < min ) return min; else
	if( val > max ) return max; else
	return val;
}

void setGeometry( SimpleGeometry* g )
{
	if( g_geomPtr == g ) return;
	g_geomPtr = g;
	if( !g ) return;
	setupColors( g->num_vertices(), g_colors, (int)g_colorMode );
}

//------------------------------------------------------------------------------
// Implementation of glutmain.cpp functions:
//	- bool frame()      which is called every frame
//	- bool init()		which is called once right after GLinit()
//	- void shutdown()	which is called before exiting the application.
//------------------------------------------------------------------------------

extern bool toggle[256]; // keystate as provided by glutmain.cpp

void internal_render_frame()
{
	// toggle BW / WB
	if( toggle['B'] ) {
		glColor4f( 0,0,0,1 );
		glClearColor( 1,1,1,1 );
	} else {
		glColor4f( 1,1,1,1 );
		glClearColor( 0,0,0,1 );
	}

	// toggle wireframe
	if( !toggle['w'] ) 
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	else
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	// shade model
	if( !toggle['s'] )
		glShadeModel( GL_SMOOTH );
	else
		glShadeModel( GL_FLAT );

	// lighting
	if( toggle['i'] ) { light0_position[2] = -1.0; }
	             else { light0_position[2] =  1.0; }
	if( !toggle['l'] )
	{
		glLightfv( GL_LIGHT0, GL_POSITION, light0_position );
		glEnable( GL_LIGHTING );
	}
	else
		glDisable( GL_LIGHTING );

	// line width
	if( toggle['('] ) { if( g_linewidth > 1.0 ) g_linewidth-=1.0; toggle['(']=false; };
	if( toggle[')'] ) { if( g_linewidth < 20.0 ) g_linewidth+=1.0; toggle[')']=false; };
	glLineWidth( g_linewidth );
	
	// render scene
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	// toggle immediate / array
	float* colorPtr = NULL;
	if( toggle['c'] ) colorPtr = &g_colors[0];
	if( g_geomPtr )
	{
		if( toggle['m'] )
			draw_immediate( g_geomPtr, colorPtr );
		else
			draw_array( g_geomPtr, colorPtr );
	}
}

void createIcosahedron( float cx, float cz )
{
	printf("Platonic constants (%.9f, %.9f)\n",cx,cz);
	g_geomIco.clear();
	g_geomIco.setPlatonicConstants( cx, cz );
	g_geomIco.create();
	setupColors( g_geomIco.num_vertices(), g_colors, (int)g_colorMode );
}

void icosahedronControls()
{
	// randomize Platonic constants
	if( toggle['R'] )
	{
		createIcosahedron( frand()*6.0-3.0, frand()*6.0-3.0 );
		toggle['R'] = false;
	}

	// icosahedron constants
	if( toggle['r'] )
	{
		double cx = 1.6180339887498948482045, // golden ratio
			   cz = 1.0; // subdivision re-scaling
		createIcosahedron( cx, cz );
		toggle['r'] = false;
	}
	
	double dx = 0.01;
	double cx = g_geomIco.getPlatonicConstantsX(),
		   cz = g_geomIco.getPlatonicConstantsZ();
	if( toggle['y'] ) { g_geomIco.clear(); g_geomIco.setPlatonicConstants( cx, cz-dx ); g_geomIco.create(); toggle['y']=false; }
	if( toggle['Y'] ) { g_geomIco.clear(); g_geomIco.setPlatonicConstants( cx, cz+dx ); g_geomIco.create(); toggle['Y']=false; }
	if( toggle['x'] ) { g_geomIco.clear(); g_geomIco.setPlatonicConstants( cx-dx, cz ); g_geomIco.create(); toggle['x']=false; }
	if( toggle['X'] ) { g_geomIco.clear(); g_geomIco.setPlatonicConstants( cx+dx, cz ); g_geomIco.create(); toggle['X']=false; }
}

void quadricControls()
{
	Superquadric& sq = g_geomSuperquadric;
	double alpha = sq.alpha(),
		   beta  = sq.beta(),
		   dx = 0.1;
	if( toggle['x'] ) { alpha -= dx; toggle['x']=false; }
	if( toggle['X'] ) { alpha += dx; toggle['X']=false; }
	if( toggle['y'] ) { beta  -= dx; toggle['y']=false; }
	if( toggle['Y'] ) { beta  += dx; toggle['Y']=false; }
	if( toggle['r'] ) { alpha = 0.5; beta = 0.5; toggle['r']=false; }
	if( toggle['R'] ) { alpha = frand()*4.0; beta = frand()*4.0; toggle['R'] = false; }
	alpha = clamp(alpha,0.01,3.99);
	beta  = clamp(beta ,0.01,3.99);
	if( alpha != sq.alpha() || beta != sq.beta() )
	{
		sq.setQuadric( alpha, beta );
		sq.create();
	}
}

void shControls()
{
	SphericalHarmonics& sh = g_geomSH;
	int l = sh.getL(),
		m = sh.getM();
	if( toggle['x'] ) { l--; toggle['x']=false;	}
	if( toggle['X'] ) { l++; toggle['X']=false; }
	if( toggle['y'] ) { m--; toggle['y']=false; }
	if( toggle['Y'] ) { m++; toggle['Y']=false; }
	if( l!=sh.getL() || m!=sh.getM() )
	{
		sh.setLM( l, m );
		sh.update();
	}
}

bool frame()
{
	// Controls to select geometry
	if( toggle['1'] ) { setGeometry(&g_geomIco);          toggle['1']=false; }
	if( toggle['2'] ) { setGeometry(&g_geomPenrose);      toggle['2']=false; }
	if( toggle['3'] ) { setGeometry(&g_geomSuperquadric); toggle['3']=false; }
	if( toggle['4'] ) { setGeometry(&g_geomSH);           toggle['4']=false; }

	// Render

	if( !g_geomPtr ) return true;

	internal_render_frame();

	if( toggle['C'] )
	{
		g_colorMode = (g_colorMode+1)%NumColorModes;
		setupColors( g_geomPtr->num_vertices(), g_colors, (int)g_colorMode );
		printf("Color mode %d\n", g_colorMode);
		toggle['C'] = false;
	}

	// specific controls
	if( dynamic_cast<SphericalHarmonics*>(g_geomPtr) )
		shControls();
	else
	if( dynamic_cast<Icosahedron*>(g_geomPtr) )
		icosahedronControls();
	if( dynamic_cast<Superquadric*>(g_geomPtr) )
		quadricControls();

	// adjust subdivision levels
	if( toggle[','] && g_geomPtr->getLevels()>0 )
	{
		g_geomPtr->clear();
		g_geomPtr->create( g_geomPtr->getLevels()-1 );
		setupColors( g_geomPtr->num_vertices(), g_colors, (int)g_colorMode );
		toggle[','] = false;
	}
	if( toggle['.'] && g_geomPtr->getLevels() < MAX_LEVELS )
	{
		g_geomPtr->clear();
		g_geomPtr->create( g_geomPtr->getLevels()+1 );
		setupColors( g_geomPtr->num_vertices(), g_colors, (int)g_colorMode );
		toggle['.'] = false;
	}

	// export postscript
	if( toggle['p'] ) {
		export_ps();
		toggle['p'] = false;
	}

	// export screenshot
	if( toggle['S'] ) {
		//internal_render_frame();
		g_screenshot.render( internal_render_frame, reshape );
		save_state( (g_screenshot.getLastFilename()+".state").c_str() );
		toggle['S'] = false;
	}

	// reshape to specific window size
	if( toggle[' '] )
	{
		glutReshapeWindow( 1024, 1024 );		
		toggle[' '] = false;
	}
	
	return true;
}

void welcome()
{
	cout << usage << endl;
	cout << "Output resolution is " << IMG_WIDTH << "x" << IMG_HEIGHT << endl;
	cout << "Output prefix is " << IMG_PREFIX << endl;
}

bool init( int argc, char** argv )
{
	glutSetWindowTitle("geometry2");

	// Setup all geometries

	// setup Penrose tiling
#if 0
	Icosahedron penroseGenerator;
	penroseGenerator.create( 0 );
	g_geomPenrose.setGenerator( penroseGenerator );
#endif
	g_geomPenrose.create( 4 );

	// setup Icosahedron
	g_geomIco.setPlatonicConstants( // New golden ratio method
		1.6180339887498948482045, // golden ratio
		1.0                       // subdivision re-scaling
		);
		// Pre-defs for Bendels method (with 2 constants and custom scaling)
		//  .587654321, -0.723456789    the moon
		// 1.287654321, 1.723456789     spikes
		//  .525731112119133606, .850650808352039932   icosahedron
	g_geomIco.create( 4 );

	// setup Superquadric
	g_geomSuperquadric.setQuadric( 0.1, 0.1 );
	g_geomSuperquadric.create();

	// setup Spherical Harmonics
	g_geomSH.setLevels( 4 );
	g_geomSH.create();

	// setup colors
	setGeometry( &g_geomSH );

	g_screenshot.setup( IMG_WIDTH, IMG_HEIGHT, IMG_PREFIX );

	// set GL states
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
	setupLight();

	// restore state (currently only Icosahedron supported)
	if( argc > 1 ) {
		if( load_state( argv[1] ) )
		{
			cout << "Succesfully applied state from " << argv[1] << endl;
			g_geomIco.clear();
			g_geomIco.create();
		}
		else
			cout << "Failed to apply state from " << argv[1] << endl;
	}
	
	return true;
}

void shutdown()
{
	g_screenshot.destroy();
}

//------------------------------------------------------------------------------
//	Serialization (currently only Icosahedron supported)
//------------------------------------------------------------------------------

void save_state( const char* filename )
{
	ofstream f(filename);
	if( !f.is_open() ) {
		cerr << "Error: Saving state to " << filename << "!" << endl;
		return;
	}

	f << "# geometry2 state" << endl;

	string switches = g_switches;
	for( size_t i=0; i < switches.length(); ++i )
		f << switches[i] << ' ' << toggle[switches[i]] << endl;

	f << "linewidth " << g_linewidth << endl;

	// TODO: save camera

	f << "ico.levels " << g_geomIco.getLevels() << endl
	  << "ico.platonicX " << g_geomIco.getPlatonicConstantsX() << endl
	  << "ico.platonicZ " << g_geomIco.getPlatonicConstantsZ() << endl;

	f.close();
}

bool load_state( const char* filename )
{
	ifstream f(filename);
	if( !f.is_open() ) {
		cerr << "Error: Loading state from " << filename << "!" << endl;
		return false;
	}

	// temp vars for g_ico params
	int levels;  bool levels_=false;
	double x,y;  bool x_=false, y_=false;

	string line;	
	while( getline(f,line) )
	{
		if( line[0] == '#' ) continue;

		stringstream ss( line );
		string token;
		if( !(ss >> token) ) goto load_state__fail;
		
		// state
		string switches = g_switches;
		if( token.length()==1 && switches.find(token[0])!=string::npos )
		{
			bool b;
			if( !(ss >> b) ) goto load_state__fail;
			toggle[token[0]] = b;
			
			continue;
		}

		if( token.find("linewidth")==0 )  ss >> g_linewidth;
		else if( token.find("ico.levels")==0 ) { ss >> levels; levels_=true; }
		else if( token.find("ico.platonicX")==0 ) { ss >> x; x_=true; }
		else if( token.find("ico.platonicZ")==0 ) { ss >> y; y_=true; }
	}

	if( levels_ ) g_geomIco.setLevels(levels);
	if( x_&&y_  ) g_geomIco.setPlatonicConstants(x,y);

	return true;

load_state__fail:
	cerr << "Error: Parsing state file " << filename << "!" << endl;
	f.close();
	return false;
}

//------------------------------------------------------------------------------

void export_ps()
{   
	FILE *fp;
	int state = GL2PS_OVERFLOW, buffsize = 0;
	
	time_t seconds = time(NULL);
	
	char filename[1024];
	sprintf( filename, "%s-%d.pdf", IMG_PREFIX, seconds );
	
	fp = fopen( filename, "wb" );
	if( !fp )
	{
		cerr << "Couldn't open file '" << filename << "'!" << endl;
		return;
	}

	printf( "Generating Postscript output...\n" );

    while( state == GL2PS_OVERFLOW )
	{
      buffsize += 1024*1024;
	  gl2psLineWidth( 1 );
      gl2psBeginPage("Geometry2", "www.386dx25.de", 
		             NULL, GL2PS_PDF, GL2PS_BSP_SORT, 
                     GL2PS_USE_CURRENT_VIEWPORT | GL2PS_NO_BLENDING | 
					 GL2PS_OCCLUSION_CULL | GL2PS_NO_PS3_SHADING, // | GL2PS_DRAW_BACKGROUND
                     GL_RGBA, 0, NULL, 0, 0, 0,  buffsize, fp, filename);
      //frame();
	#if 0
	  // FIXME: PS text output not working.
	  char info[1024]; sprintf( info, "Platonic X=%.8f, Z=%.8f", 
		  g_ico.getPlatonicConstantsX(), g_ico.getPlatonicConstantsZ() );
	  glRasterPos2i( 50, 50 );
	  glColor3f( 1,0,0 );
	  gl2psText( info, "Helvetica", 11 );
	#endif
	  if( g_geomPtr )
		draw_immediate( g_geomPtr, toggle['c'] ? &g_colors[0] : NULL );
      state = gl2psEndPage();
    }
    fclose(fp);
    printf( "Current GL-Viewport exportet to %s\n", filename );
}

//------------------------------------------------------------------------------

void setupLight()
{
	// gl light 0
	GLfloat light0_ambient[]  = {  .1,  .1,  .1, 1.0 };
	GLfloat light0_diffuse[]  = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat light0_specular[] = { 1.0, 1.0, 1.0, 1.0 };
//	GLfloat light0_position[] = {-0.6, 0.6, 0.6, 0.0 };
	glLightfv( GL_LIGHT0, GL_AMBIENT,  light0_ambient  );
	glLightfv( GL_LIGHT0, GL_DIFFUSE,  light0_diffuse  );
	glLightfv( GL_LIGHT0, GL_SPECULAR, light0_specular );
	glLightfv( GL_LIGHT0, GL_POSITION, light0_position );
	glEnable( GL_LIGHT0 );
	glEnable( GL_LIGHTING );

	// gl states 
	glEnable( GL_COLOR_MATERIAL );
	glShadeModel( GL_SMOOTH );
}

//------------------------------------------------------------------------------

// helper functions for color gradient
float fract( float f )
{
	return f - floor(f);
}

void clamp_vector( int n, float* v, float min, float max )
{
	for( int i=0; i < n; ++i )
		v[i] = v[i] < min ? min : (v[i] > max ? max : v[i]);
}

/// Return RGB color from HLS'
/// \param h hue in [0,360] degrees
/// \param[out] rgb resulting rgb color (pointer to float[3])
void rgb_from_hls( float h, float l, float s, float* rgb )
{
	// custom base colors (rainbow)
	const int n=5;
	static float hls[n*3] = { 
	                          0, 0, 1,  // Blue
							  0, 1, 1,  // Cyan*
						      0, 1, 0,  // Green
					          1, 1, 0,  // Yellow
	                          1, 0, 0,  // Red
					        };

	int   sector = (int)floor( h / (360./(n-1)) );
	float lambda =      fract( h / (360./(n-1)) );

	// linear interpolation between the two base colors
	float tmp[3];
	for( int i=0; i < 3; ++i )
	{
		int j0 = 3*sector + i,
			j1 = 3*(sector+1) + i;
		tmp[i] = hls[j0] + lambda*( hls[j1] - hls[j0] ); // (A)

		// linear interpolation between grey value at center of cone and tmp
		rgb[i] = .5*l + s*tmp[i];
	}
	clamp_vector( 3, rgb, 0, 1 );
}

void setupColors( int n, std::vector<float>& colors, int func )
{
	const int baseSize = 1024;
	colors.resize( n*4 );
	for( int i=0; i < n; ++i )
	{
		float di = (float)(i%baseSize) / (float)(baseSize-1);

		if( func == RainbowGradient )
		{
			// create nice color gradient ("rainbow")
			float hue = 360.f * di;
			float rgb[3];
			rgb_from_hls( hue, .5f, 1.f, rgb );

			// rgb
			for( int j=0; j < 3; ++j )
				colors[i*4 + j] = (unsigned char)255.f*rgb[j];
			// alpha
				colors[i*4 + 3] = 1.0; //255.f*(i/511.f); // linear ramp
		}
		else
		if( func == GrayGradient )
		{
			float gray = di;
			for( int j=0; j < 3; ++j )
				colors[i*4 + j] = (unsigned char)255.f*gray;
			// alpha
				colors[i*4 + 3] = 1.0; //255.f*(i/511.f); // linear ramp
		}
		else
		if( func == RandomTint )
		{
			const float ambient[3] = { .2f, .4f, .6f };
			const float tint[3]    = { .2f, .1f, .5f };
			for( int j=0; j < 3; ++j )
				colors[i*4+j] = ambient[j] + frand()*tint[j];
			colors[i*4+3] = 1.0; 
		}
		else // default is "RandomColors"
		{
			// random palette
			for( int j=0; j < 3; ++j )
				colors[i*4+j] = frand();
			// alpha
				colors[i*4+3] = 1.0; 
		}
	}
}

//------------------------------------------------------------------------------

void draw_immediate( SimpleGeometry* geom, float* colorPtr )
{
	// flat shading for VBOs only possible with custom vertex shader
	// so we misuse the immediate mode here for correct flat shading
	bool per_face_normal = true;

	float* vp = geom->get_vertex_ptr();
	float* np = geom->get_normal_ptr();
	int* fp = geom->get_index_ptr();
	glBegin( GL_TRIANGLES );
	for( int i=0; i < geom->num_faces(); ++i )
	{
		if( per_face_normal ) 
		{
			// compute face normal on the fly by averaging vertex normals
			vec3 n0, n1, n2;
			n0.set_( &np[ fp[3*i+0] * 3 ] );
			n1.set_( &np[ fp[3*i+1] * 3 ] );
			n2.set_( &np[ fp[3*i+2] * 3 ] );
			vec3 n = (n0+n1+n2)/3.f;

			glNormal3f( n.x, n.y, n.z );
		}

		if(colorPtr) glColor4fv( &colorPtr[ fp[3*i+0] * 4 ] );
		if(!per_face_normal) glNormal3fv( &np[ fp[3*i+0] * 3 ] );
		glVertex3fv( &vp[ fp[3*i+0] * 3 ] );
		if(colorPtr) glColor4fv( &colorPtr[ fp[3*i+1] * 4 ] );
		if(!per_face_normal) glNormal3fv( &vp[ fp[3*i+1] * 3 ] );
		glVertex3fv( &vp[ fp[3*i+1] * 3 ] );
		if(colorPtr) glColor4fv( &colorPtr[ fp[3*i+2] * 4 ] );
		if(!per_face_normal) glNormal3fv( &vp[ fp[3*i+2] * 3 ] );
		glVertex3fv( &vp[ fp[3*i+2] * 3 ] );
	}
	glEnd();
}

void draw_array( SimpleGeometry* geom, float* colorPtr )
{
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, geom->get_vertex_ptr() );
	glNormalPointer(    GL_FLOAT, 0, geom->get_normal_ptr() );

	if( colorPtr )
	{
		glEnableClientState( GL_COLOR_ARRAY );
		glColorPointer( 4, GL_FLOAT, 0, colorPtr );
	}
	
	glDrawElements( GL_TRIANGLES, geom->num_faces()*3, GL_UNSIGNED_INT, 
	                geom->get_index_ptr() );
	
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
}
