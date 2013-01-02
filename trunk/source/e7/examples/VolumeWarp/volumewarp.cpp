// e7 VolumeWarp
// Max Hermann, August 08, 2010

// The application comes in two flavours, GLUT and Qt4, configured via CMake
#ifdef VOLUMEWARP_USE_QT
#define MY_E7ENGINE EngineQt
#include <QApplication>
#include <QIcon>
#include <QPixmap>
#include <Engines/Qt/EngineQt.h>
#else
#define MY_E7ENGINE EngineGLUT
#include <Engines/GLUT/EngineGLUT.h>
#endif

#include <iostream>
#include <exception>
#include <GL/glew.h>
#include <GL/GLTexture.h>
#include <VolumeRendering/VolumeData.h>
#include <VolumeRendering/VolumeUtils.h>  // load_volume(), create_volume_tex()
#include <VolumeRendering/VolumeRendererRaycast.h>
//#include <Generative/NoiseShader.h>
#include <Generative/PerlinNoise.h>

#ifdef WIN32
#pragma warning(disable: 4244)
#pragma warning(disable: 4305)
#endif

#define SCREENSHOT_WIDTH  (2*1920) //4096 //4096
#define SCREENSHOT_HEIGHT (2*1080) //2048 //(2048+1024)
#define ASPECT (SCREENSHOT_WIDTH/(float)SCREENSHOT_HEIGHT)
#define TEXTURE_SIZE_CHANGE_POSSIBLE_WITHOUT_DRIVER_CRASH

using namespace std;

/// Raycaster which can warp the input volume by noise displacement fields
class Raycaster : public MY_E7ENGINE
{
	struct State
	{
		State()
			: fov(45)
		{}

		void get( const Raycaster& rc, const VolumeRendererRaycast& vren )
		{
			isovalue = vren.getIsovalue();

			const RaycastShader& rs = vren.getRaycastShader();
			rs.get_warp_ofs( warpOfs );
			renderMode = (int)rs.get_rendermode();
			warpEnabled  = rs.get_warp_enabled();
			warpStrength = rs.get_warp_strength();
			alphaScale   = rs.get_alpha_scale();
			stepSize     = rs.get_stepsize();
			
			//fov          = rc.getFoV();
			//trackball    = rc.getTrackballState();
		}

		void set( Raycaster& rc, VolumeRendererRaycast& vren )
		{
			vren.setIsovalue( isovalue );

			RaycastShader& rs = vren.getRaycastShader();
			rs.set_warp_ofs( warpOfs );
			rs.set_rendermode( renderMode );
			rs.set_warp_enabled( warpEnabled );
			rs.set_warp_strength( warpStrength );
			rs.set_alpha_scale( alphaScale );
			rs.set_stepsize( stepSize );

			//rc.setFoV( fov );
			//rc.setTrackballState( trackball );
		}

		// VolumeRendererRaycast
		float isovalue;
		// RaycastShader
		int   renderMode;
		bool  warpEnabled;
		float warpStrength;
		float warpOfs[3];
		float alphaScale;
		float stepSize;
		// Raycaster
		float fov;
		// Trackball (i.e. camera rotation, translation and zoom)
		TrackballState trackball;
	};

	State interpolate( State s0, State s1, float t )
	{
		State s;
		// linear interpolants
		s.isovalue      = (1-t)*s0.isovalue     + t*s1.isovalue;
		s.warpStrength  = (1-t)*s0.warpStrength + t*s1.warpStrength;
		for( int d=0; d < 3; d++ )
			s.warpOfs[d]= (1-t)*s0.warpOfs[d]   + t*s1.warpOfs[d];
		s.alphaScale    = (1-t)*s0.alphaScale   + t*s1.alphaScale;
		s.stepSize      = (1-t)*s0.stepSize     + t*s1.stepSize;
		// TODO: camera interpolation
		return s;
	}	

public:
	Raycaster(): m_vol(NULL), m_animation(true), m_fov(45.0) {}
	void idle();
	void render();
	bool init( int argc, char* argv[] );
	void destroy();
	void reshape( int w, int h );

	void screenshot( string filename="",int width=-1, int height=-1 );
	void makeMovie();

	float getFoV() const { return m_fov; }
	void setFoV( float fov ) { m_fov = fov; reshape(-1,-1); }

protected:
	bool genWarpTexture( GL::GLTexture& wtex, float (*n4) (int,float,float,float) );

	void onKeyPressed( unsigned char key );

private:
	VolumeDataHeader* m_vol;
	GL::GLTexture     m_vtex;
	GL::GLTexture     m_wtex;
	VolumeRendererRaycast m_vren;

	bool m_animation;
	float m_fov;

	State m_states[2];
};

// warp noise helper functions
// (vector valued noise, channel ch=0,1,2 corresponds to (x,y,z) component)

inline float wnoise_turbulence( int ch, float x, float y, float z )
{
	static float ofs_arr[3] = { 17.f, 177.f, 1777.f };
	float ofs = ofs_arr[ch];
	return PerlinNoise::turbulence( ofs+x, ofs+y, ofs+z, 2 );
}

inline float wnoise_fBm( int ch, float x, float y, float z )
{
	static float ofs_arr[3] = { 17.f, 177.f, 1777.f };
	float ofs = ofs_arr[ch];
	return PerlinNoise::fBm( ofs+x, ofs+y, ofs+z, 5 ) * 0.5 + 0.5;
}

inline float wnoise_ridgedmf( int ch, float x, float y, float z )
{
	static float ofs_arr[3] = { 17.f, 177.f, 1777.f };
	float ofs = ofs_arr[ch];
	return PerlinNoise::ridgedmf( ofs+x, ofs+y, ofs+z, 3 );
}

//-----------------------------------------------------------------------------
void Raycaster::idle()
{
	if( !m_animation ) return;

	float dt = 0.01f; //get_time_delta();
	static float t = 0.f;
	t += dt;

	RaycastShader& rs = m_vren.getRaycastShader();

#if 0
	float wanim = sin(t)*0.11f;
	rs.set_warp_strength( wanim );
#else
	float ofs[3];
	rs.get_warp_ofs( ofs );
	rs.set_warp_ofs( ofs[0]+dt, ofs[1]+dt, ofs[2]+dt );
#endif
	update();
}

//-----------------------------------------------------------------------------
float sign( float a ) { return a < 0 ? -1.f : +1.f; }

void Raycaster::screenshot( string filename, int width, int height )
{
#ifdef VOLUMEWARP_USE_QT
	// FIXME: screenshot() currently not implemented in EngineQt!
#else
	if( filename.empty() )
		filename = autoName("vwarp-",".tga");
	if( width==-1 ) width = SCREENSHOT_WIDTH;
	if( height==-1) height= SCREENSHOT_HEIGHT;
	
	bool prevMode = m_vren.getOffscreen();
	m_vren.setOffscreen( true );
	
  #ifdef TEXTURE_SIZE_CHANGE_POSSIBLE_WITHOUT_DRIVER_CRASH
	int prevWidth = m_vren.getTextureWidth(), // getOffscreenWidth(),
		prevHeight = m_vren.getTextureHeight(); //getOffscreenHeight();
	m_vren.changeTextureSize( width, height );	
  #endif

	this->render();

	m_vren.getR2T()->bind( m_vren.getOffscreenTexture() );
	grabTGA( filename, false, width, height );
	m_vren.getR2T()->unbind();

	m_vren.setOffscreen( prevMode );

  #ifdef TEXTURE_SIZE_CHANGE_POSSIBLE_WITHOUT_DRIVER_CRASH
	m_vren.changeTextureSize( prevWidth, prevHeight );
  #endif
#endif // else VOLUMEWARP_USE_QT
}

void Raycaster::makeMovie()
{
	string in;
	cout << "Do you want to make a movie? [Y/N]? ";
	cin >> in;
	if( !in.empty() && in[0]=='Y' )
	{
		string prefix;
		int nof, width, height;
		cout << "Movie prefix: "; cin >> prefix;
		cout << "Number of frames: "; cin >> nof;
		cout << "Width: ";  cin >> width;
		cout << "Height: "; cin >> height;

		char filename[1000];
		for( int i=0; i < nof; i++ )
		{
			printf("Rendering frame %8d / %8d\n",i+1,nof);

			sprintf(filename,"%s-%08d.tga",prefix.c_str(),i);
			screenshot(filename,width,height);

			// interpolate state
			float dt = i/(float)(nof-1);
			State s = interpolate( m_states[0], m_states[1], dt );
			s.set( *this, this->m_vren );

			// render next frame
			idle();
		}
	}
}

void Raycaster::onKeyPressed( unsigned char key )
{
	static float ews = 0.f;

	RaycastShader& rs = m_vren.getRaycastShader();

	float iso = m_vren.getIsovalue(),
		  ws  = rs.get_warp_strength();
	
	float ofs[3];
	rs.get_warp_ofs( ofs );

	static bool offscreen = true;

	switch( key )
	{
	case 'i': m_vren.setIsovalue( iso > 0.0f ? iso-0.01 : 0.01 ); break;
	case 'I': m_vren.setIsovalue( iso < 1.0f ? iso+0.01 : 1.00 ); break;

	case 'a': m_animation = !m_animation; 
	          cout << "animation " << (m_animation?"on":"off") << "\n"; 
			  break;
	case 'r': rs.init(); 
	          cout << "reinit" << endl; 
			  break;
	case ',': rs.set_warp_strength( ws + 0.01f ); break;
	case '.': rs.set_warp_strength( ws - 0.01f ); break;
	case ';': ews += 0.01f; rs.set_warp_strength( sign(ews) * ews*ews ); break;
	case ':': ews -= 0.01f; rs.set_warp_strength( sign(ews) * ews*ews ); break;
	//case '0': rs.set_warp_strength( 0.f ); break;
	
	case '!': m_states[0].get( *this, this->m_vren ); cout << "Saving state 1\n"; break;
	case '"': m_states[1].get( *this, this->m_vren ); cout << "Saving state 2\n"; break;

	case 'M': makeMovie(); break;
	
	case '1': m_states[0].set( *this, this->m_vren ); cout << "Loading state 1\n"; break;
	case '2': m_states[1].set( *this, this->m_vren ); cout << "Loading state 2\n"; break;
	
	case 'R': ews = 0; rs.set_warp_strength( ews ); break;
	case 'm': rs.cycle_rendermode(); rs.init(); break;
	case '+': rs.set_alpha_scale( rs.get_alpha_scale() + 0.1f ); break;
	case '-': rs.set_alpha_scale( rs.get_alpha_scale() - 0.1f ); break;
	case 'w': rs.set_warp_enabled( !rs.get_warp_enabled() ); rs.init(); break;
	case 'o': rs.set_warp_ofs( ofs[0]+0.01, ofs[1]+0.01, ofs[2]+0.01 ); break;

	case 'L': rs.set_stepsize( rs.get_stepsize() * 1.1 ); break;
	case 'l': rs.set_stepsize( rs.get_stepsize() * 0.9 ); break;

	case 'S': 
		cout << "Taking screenshot..." << endl;
		screenshot();
		break;

	case '5':
		cout << "Generating turbulence warp..." << endl;
		genWarpTexture( m_wtex, wnoise_turbulence );
		break;
	case '6':
		cout << "Generating fBm warp..." << endl;
		genWarpTexture( m_wtex, wnoise_fBm );
		break;
	case '7':
		cout << "Generating ridgedmf warp..." << endl;
		genWarpTexture( m_wtex, wnoise_ridgedmf );
		break;

	case 'T':
		offscreen = !offscreen;
		m_vren.setOffscreen( offscreen );
		break;

	case '(': m_vren.changeTextureSize( 256, 256 ); break;
	case ')': m_vren.changeTextureSize( 512, 512 ); break;
	
	case 27: exit(0);
	}

	//cout << "iso = " << m_vren.getIsovalue() << endl;
	//cout << "alpha = " << rs.get_alpha_scale() << endl;
	//cout << "warp strength = " << rs.get_warp_strength() << endl;
	//cout << "stepsize = " << rs.get_stepsize() << endl;

	if( !m_animation )
		update();
}

//-----------------------------------------------------------------------------

bool Raycaster::genWarpTexture( GL::GLTexture& wtex, float (*n4) (int,float,float,float) )
{
	// Use 3 independent noise fields as displacement maps
	int size=64;
	float* data = new float[size*size*size*3];

	for( int z=0; z<size; ++z )
		for( int y=0; y<size; ++y )
			for( int x=0; x<size; ++x )
			{
				float scale = 3.f;
				float dx = scale * (float)x/(size-1),
					  dy = scale * (float)y/(size-1),
					  dz = scale * (float)z/(size-1);

				data[3*(z*size*size + y*size + x)+0] = n4( 0, dx,dy,dz );
				data[3*(z*size*size + y*size + x)+1] = n4( 1, dx,dy,dz );
				data[3*(z*size*size + y*size + x)+2] = n4( 2, dx,dy,dz );
			}

	// --- Create warp texture ---

	if( !wtex.Create( GL_TEXTURE_3D ) )
	{
		cerr << "Error: Coudln't create 3D texture!" << endl;
		delete [] data;  // free data buffer
		return false;
	}

	// don't forget to set HW supported parameters	
	// GL_CLAMP_TO_EDGE / GL_CLAMP / GL_REPEAT
	wtex.SetWrapMode  ( GL_MIRRORED_REPEAT_ARB ); //GL_REPEAT );
	wtex.SetFilterMode( GL_LINEAR );

	// GPU download
	if( !wtex.Image( 0, /*GL_RGBA8*/ GL_RGBA32F, size,size,size, 0, 
	                 GL_RGB, GL_FLOAT, data ) )
	{
		cerr << "Error: Couldn't upload warp volume to 3D texture!" << endl;
		delete [] data;  // free data buffer
		return false;
	}
	
	delete [] data;  // free data buffer
	return true;
}

//-----------------------------------------------------------------------------
bool Raycaster::init( int argc, char* argv[] )
{
#ifdef VOLUMEWARP_USE_QT
	// WORKAROUND: Make sure that Qt widget has valid GL context!
	this->makeCurrent();
#endif

	int verbosity = 2;

	if( argc != 2 )
	{
		cout << "Usage: " << argv[0] << " <volume.mhd>" << endl;
		return false;
	}
	string filename( argv[1] );

	setFrequentUpdate( false );

	// --- Load volume dataset ---

	void* dataptr = NULL;
	m_vol = load_volume( filename.c_str(), verbosity, &dataptr );
	if( !m_vol )
	{
		cerr << "Error: Couldn't load volume data from '" << filename
		     << "'!" << endl;
		return false;
	}

	// --- Download 3D texture(s) ---

	if( verbosity > 1 ) cout << "Creating 3D volume texture..." << endl;
	if( !create_volume_tex( m_vtex, m_vol, dataptr, verbosity ) )
	{
		return false;
	}

	// --- Create warp texture ---

	if( verbosity > 1 ) cout << "Creating warp texture..." << endl;
	if( !genWarpTexture( m_wtex, wnoise_ridgedmf ) )
	{
		return false;
	}

	// --- Setup raycaster ---

	m_vren.setVolume( &m_vtex );

	m_vren.setWarpfield( &m_wtex );
	m_vren.getRaycastShader().set_warp_enabled( true );
	m_vren.getRaycastShader().set_warp_strength( 0.f );

	m_vren.init();
	m_vren.setAspect(/*m_vol->spacingX()* */1,
	                 /*m_vol->spacingY()* */m_vol->resY()/(float)m_vol->resX(),
	                 /*m_vol->spacingZ()* */m_vol->resZ()/(float)m_vol->resX() );
	m_vren.setZNear( 0.1 );
	m_vren.setRenderMode( RaycastShader::RenderIsosurface ); // RenderMIP
	m_vren.setIsovalue( 0.15 );

	m_vren.setOffscreen( true );

	return true;
}

//-----------------------------------------------------------------------------
void Raycaster::destroy()
{
	cout << "Raycaster::destroy()" << endl;
	m_vtex.Destroy();
	delete m_vol; m_vol=NULL;
}

//-----------------------------------------------------------------------------
void Raycaster::reshape( int w, int h )
{
	if( w<0 || h<0 ) 
	{
		GLint viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );
		if( w<0 ) w = viewport[3];
		if( h<0 ) h = viewport[4];
	}
	float aspect = (float)w/(float)h;

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( m_fov, aspect, 0.1, 142 );
	glMatrixMode( GL_MODELVIEW );
}

//-----------------------------------------------------------------------------
void Raycaster::render()
{
	glClearColor( 0,0,0,1 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	GL::checkGLError("before Raycaster::render()");
	m_vren.render();
	GL::checkGLError("after Raycaster::render()");
}

//-----------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	int ret=0;

#ifdef VOLUMEWARP_USE_QT
	
	// ==== Qt4 application ====

	Q_INIT_RESOURCE( volumewarp );
	QApplication app( argc, argv );

	Raycaster vwarp;
	vwarp.setWindowIcon( QIcon(QPixmap(":/data/icons/icon.png")) );
	vwarp.setWindowTitle("volumewarp");
	vwarp.show();
	vwarp.run( argc, argv ); // EngineQt::run() will return immediately

	// launch Qt main loop
	ret = app.exec();
	QApplication::closeAllWindows();
#else

	// ==== GLUT application ====

	Raycaster vwarp;
	try
	{
		vwarp.set_window_title("volumewarp");
		vwarp.set_window_size(ASPECT*512,512); //( 512, 512 ); //( 720, 576 );  // 1280 x 720
		vwarp.run( argc, argv );
	}
	catch( exception& e )
	{
		cerr << "Exception caught:" << endl << e.what() << endl;
	}	
	// never reached, put cleanup code into EngineGLUT::destroy()
#endif	
	
	return ret;
}
