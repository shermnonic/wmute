#include <Engines/GLUT/EngineGLUT.h>
#include <iostream>
#include <exception>
#include <GL/glew.h>
#include <GL/GLTexture.h>
#include <VolumeRendering/VolumeData.h>
#include <VolumeRendering/VolumeUtils.h>  // load_volume(), create_volume_tex()
#include <VolumeRendering/VolumeRendererRaycast.h>

#ifdef WIN32
#pragma warning(disable: 4244)
#pragma warning(disable: 4305)
#endif

using namespace std;

class Raycaster : public EngineGLUT
{
public:
	Raycaster(): m_vol(NULL) {}
	void render();
	bool init( int argc, char* argv[] );
	void destroy();
	void reshape( int w, int h );

private:
	VolumeDataHeader* m_vol;
	GL::GLTexture     m_vtex;
	VolumeRendererRaycast m_vren;
};

//-----------------------------------------------------------------------------
bool Raycaster::init( int argc, char* argv[] )
{
	int verbosity = 2;

	if( argc != 2 )
	{
		cout << "Usage: " << argv[0] << " <volume.mhd>" << endl;
		return false;
	}
	string filename( argv[1] );

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

	// --- Setup raycaster ---

	m_vren.setVolume( &m_vtex ); // FIXME: make setVolume() call implicit to init()
	m_vren.init();
	m_vren.setAspect(/*m_vol->spacingX()* */1,
	                 /*m_vol->spacingY()* */m_vol->resY()/(float)m_vol->resX(),
	                 /*m_vol->spacingZ()* */m_vol->resZ()/(float)m_vol->resX() );
	m_vren.setZNear( 0.1 );
	m_vren.setRenderMode( RaycastShader::RenderIsosurface ); // RenderMIP
	m_vren.setIsovalue( 0.15 );

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
	float aspect = (float)w/(float)h;

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45, aspect, 0.1, 142 );
	glMatrixMode( GL_MODELVIEW );
}

//-----------------------------------------------------------------------------
void Raycaster::render()
{
	glClearColor( 0,0,0,1 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	m_vren.render();
}

//-----------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	Raycaster app;
	try
	{
		app.set_window_title("Raycaster");
		app.run( argc, argv );
	}
	catch( exception& e )
	{
		cerr << "Exception caught:" << endl << e.what() << endl;
	}	
	return 0; // never reached, put cleanup code into EngineGLUT::destroy()
}
