#include "RendererWidget.h"
#include <QtGui>
#include <iostream>
#include "RGBDFrame.h"

//-----------------------------------------------------------------------------
//	C'tor
//-----------------------------------------------------------------------------
RendererWidget::RendererWidget( QWidget* parent, const QGLWidget* shareWidget, 
                                Qt::WindowFlags f )
	: QGLWidget( parent, shareWidget, f ),
	  m_mode( ModeTrackball ),
	  m_frame( NULL ),
	  m_filter( NULL )
{
	// Render update timer
	m_renderUpdateTimer = new QTimer( this );
	connect( m_renderUpdateTimer, SIGNAL(timeout()), this, SLOT(updateGL()) );
	m_renderUpdateTimer->start( 42 );
}

//-----------------------------------------------------------------------------
//	initializeGL()
//-----------------------------------------------------------------------------
void RendererWidget::initializeGL()
{
	// Since we create the GL context, we have to initialize GLEW		
	glewExperimental = GL_TRUE;	
	GLenum glew_err = glewInit();
	if( glew_err != GLEW_OK )
	{
		std::cerr << "GLEW Error:\n" << glewGetErrorString(glew_err) << std::endl;
		QMessageBox::warning( this, tr("Error"),
			tr("Could not setup GLEW OpenGL extension manager!\n") );
	}
	std::cout << "Using GLEW " << glewGetString( GLEW_VERSION ) << std::endl;
	if( !glewIsSupported("GL_VERSION_1_3") )
	{
		std::cerr << "GLEW Error:\n" << glewGetErrorString(glew_err) << std::endl;
		QMessageBox::warning( this, tr("sdmvis: Warning"),
			tr("OpenGL 1.3 not supported by current graphics hardware/driver!") );
	}
	
	// OpenGL default states
	glClearColor(0,0,1,1);
}

//-----------------------------------------------------------------------------
//	resizeGL()
//-----------------------------------------------------------------------------
void RendererWidget::resizeGL( int w, int h )
{
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

	m_trackball.setViewSize( w, h );
}

//-----------------------------------------------------------------------------
//	paintGL()
//-----------------------------------------------------------------------------
void RendererWidget::paintGL()
{
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	// Camera
	glLoadIdentity( );
	glm::mat4 modelview = m_trackball.getCameraMatrix();
	glMultMatrixf( &modelview[0][0] );

	// RGBD frame visualization
	if( m_frame ) 
		m_frame->render( m_filter );
	
	// Overlay additional information
	if( true )
	{
		// setup font
		static QFont labelFont( "Helvetica", 12 );
		const int lineHeight = 16;
		labelFont.setStyleStrategy( QFont::PreferAntialias );
		glColor3f( .9f, .9f, .9f );

		QString infoPath, infoTimecode;
		if( m_frame ) 
		{
			infoPath = m_frame->getPath();
			infoTimecode = QString::number( (double)m_frame->getTimecode(), 'f', 2 );
		}

		QString infoMode;
		switch( m_trackball.getMode() ) 
		{
		case Trackball2::Rotate    : infoMode = "Rotate"; break;
		case Trackball2::Translate : infoMode = "Pan";    break;
		case Trackball2::Zoom      : infoMode = "Zoom";   break;
		}

		// info text
		renderText( 10,   lineHeight, tr("RGBD path: %1").arg(infoPath), labelFont );
		renderText( 10, 2*lineHeight, tr("Mode: %1").arg(infoMode), labelFont );
		renderText( 10, 3*lineHeight, tr("Time: %1").arg(infoTimecode), labelFont );
	}
}

//-----------------------------------------------------------------------------
//	mouseMoveEvent()
//-----------------------------------------------------------------------------
void RendererWidget::mouseMoveEvent( QMouseEvent* e )
{	
	QGLWidget::mouseMoveEvent( e );
	if( e->isAccepted() )
		return;

	if( m_mode == ModeTrackball )
	{
		m_trackball.update( e->pos().x(), e->pos().y() );
		e->accept();
		invokeRenderUpdate();
	}
	else
	{
	}
}

//-----------------------------------------------------------------------------
//	mousePressEvent()
//-----------------------------------------------------------------------------
void RendererWidget::mousePressEvent( QMouseEvent* e )
{		
	QGLWidget::mouseMoveEvent( e );
	if( e->isAccepted() )
		return;

	if( m_mode == ModeTrackball )
	{
		if( e->buttons() & Qt::LeftButton )
		{
			m_trackball.start( e->pos().x(), e->pos().y(), Trackball2::Rotate );
			e->accept();
			invokeRenderUpdate();
		}
		else
		if( e->buttons() & Qt::RightButton )
		{
			m_trackball.start( e->pos().x(), e->pos().y(), Trackball2::Translate );
			e->accept();
			invokeRenderUpdate();
		}
		else
		if( e->buttons() & Qt::MiddleButton )
		{
			m_trackball.start( e->pos().x(), e->pos().y(), Trackball2::Zoom );
			e->accept();
			invokeRenderUpdate();
		}
	}
	else
	{}
}

//-----------------------------------------------------------------------------
//	mouseReleaseEvent()
//-----------------------------------------------------------------------------
void RendererWidget::mouseReleaseEvent( QMouseEvent* e )
{	
	QGLWidget::mouseMoveEvent( e );
	if( e->isAccepted() )
		return;	

	if( m_mode == ModeTrackball )
	{
		if( e->button() == Qt::LeftButton )
		{
			m_trackball.stop();
			e->accept();
			invokeRenderUpdate();			
		}
	}
	else
	{}
}

//-----------------------------------------------------------------------------
//	wheelEvent()
//-----------------------------------------------------------------------------
void RendererWidget::wheelEvent( QWheelEvent* e )
{	
	QGLWidget::wheelEvent( e );
	if( e->isAccepted() )
		return;

	if( m_mode == ModeTrackball )
	{
		e->accept();
		invokeRenderUpdate();		
	}
	else
	{}
}
