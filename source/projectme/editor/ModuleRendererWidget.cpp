#include "ModuleRendererWidget.h"
#include "ModuleRenderer.h"
#include <QDebug>
#include <QTimer>

//-----------------------------------------------------------------------------
ModuleRendererWidget::ModuleRendererWidget( QWidget *parent, QGLWidget *share )
: QGLWidget( parent, share ),  
  m_moduleRenderer( 0 )
{
	setMinimumSize( 256, 180 );

	// Render update timer
	m_renderUpdateTimer = new QTimer( this );
	connect( m_renderUpdateTimer, SIGNAL(timeout()), this, SLOT(updateGL()) );
	m_renderUpdateTimer->start( 42 );

	// DEBUG
	qDebug() << "The newly created ModuleRendererWidget" << 
		(isSharing() ? "is sharing!" : "is *not* sharing?!");
}

//-----------------------------------------------------------------------------
void ModuleRendererWidget::setModuleRenderer( ModuleRenderer* mod )
{
	m_moduleRenderer = mod;
}

//-----------------------------------------------------------------------------
void ModuleRendererWidget::initializeGL()
{
	// The context should already be initialized correctly by the master
	// QGLWidget whose context this ModuleRendererWidget shares.
}

//-----------------------------------------------------------------------------
void ModuleRendererWidget::resizeGL( int w, int h )
{
	// Set some default projection matrix. 
	// Note that this may be overidden in the render() call!
	float aspect = (float)w/h,
		  fov    = (float)45.f,
		  znear  =   0.1f,
		  zfar   =  42.0f;

	glViewport( 0,0, w,h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( fov, aspect, znear, zfar );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}

//-----------------------------------------------------------------------------
void ModuleRendererWidget::paintGL()
{
	static GLfloat verts[2*4] =
		{ -1.f, -1.f,
		  -1.f,  1.f,
		   1.f,  1.f,
		   1.f, -1.f };
	static GLfloat texcoords[2*4] = 
		{  0.f,  0.f,
		   0.f,  1.f,
		   1.f,  1.f,
		   1.f,  0.f };
	
	glClearColor( 0,1,0,1 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	if( !m_moduleRenderer ) return;
	
	// Assume that module rendering has already been triggered and do
	// not call render() here!
	// 	m_moduleRenderer->render();
	int texid = m_moduleRenderer->target();
		
	// FIXME: Code duplication from RenderSet::beginRendering()!
	// Push the projection matrix and the entire GL state
	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	
	// Set 2D ortho projection
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( -1,1, -1,1, -10,10 );
	glMatrixMode( GL_MODELVIEW );			
	
	// Render full-sized quad with texture output from ModuleRenderer
	glColor4f( 1.f, 1.f, 1.f, 1.f );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, texid );
	
	glBegin( GL_QUADS );
	for( int i=0; i < 4; i++ )
	{
		glTexCoord2fv( &texcoords[2*i] );
		glVertex2fv( &verts[2*i] );
	}
	glEnd();
	
	glDisable( GL_TEXTURE_2D );
	
	// FIXME: Code duplication from RenderSet::endRendering()!
	// Pop the projection matrix and GL state back for rendering
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );	
	glPopAttrib();
}
