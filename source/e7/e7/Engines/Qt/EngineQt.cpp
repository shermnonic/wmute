#include "EngineQt.h"
#include "Trackball.h"
#include <iostream>

//==============================================================================
//	OpenGL/Qt4 Helper functions (gl_...)
//==============================================================================

static void gl_multMatrix(const QMatrix4x4& m)
{
    // static to prevent glMultMatrixf to fail on certain drivers
    static GLfloat mat[16];
    const qreal *data = m.constData();
    for (int index = 0; index < 16; ++index)
        mat[index] = data[index];
    glMultMatrixf(mat);
}


//==============================================================================
//	EngineQt
//==============================================================================

EngineQt::EngineQt( QWidget* parent, const QGLWidget* shareWidget, Qt::WindowFlags f )
: CustomGLWidget( parent, shareWidget, f )
{
	// animation timer
	m_renderUpdateTimer = new QTimer( this );
	connect( m_renderUpdateTimer, SIGNAL(timeout()), this, SLOT(updateGL()) );	
}

EngineQt::~EngineQt()
{	
}

void EngineQt::setAutomaticRenderUpdate( bool enable )
{
	if( enable )
		m_renderUpdateTimer->start( 42 );
	else
		m_renderUpdateTimer->stop();
}

void EngineQt::initializeGL()
{
	// Since we create the GL context, we have to initialize GLEW
	glewExperimental = GL_TRUE;	
	GLenum glew_err = glewInit();
	if( glew_err != GLEW_OK )
	{
		std::cerr << "GLEW error:" << glewGetErrorString(glew_err) << std::endl;
		QMessageBox::warning( this, tr("sdmvis: Warning"),
			tr("Could not setup GLEW OpenGL extension manager!") );
	}
	std::cout << "Using GLEW " << glewGetString( GLEW_VERSION ) << std::endl;
	if( !glewIsSupported("GL_VERSION_1_3") )
	{
		std::cerr << "GLEW error:" << glewGetErrorString(glew_err) << std::endl;
		QMessageBox::warning( this, tr("sdmvis: Warning"),
			tr("OpenGL 1.3 not supported by current graphics hardware/driver!"));
	}

	// Some OpenGL default states
	glClearColor(0,0,0,1);
}

void EngineQt::reshape( int w, int h )
{
	float aspect = (float)w/h,
		  fov    = (float)getCameraFOV(),
		  znear  =   0.1f,
		  zfar   =  42.0f;

	glViewport( 0,0, w,h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( fov, aspect, znear, zfar );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();	
}

void EngineQt::render()
{
	// gray screen
	glClearColor( .5, .5, .5, 1 );
	glClear( GL_COLOR_BUFFER_BIT );
}

void EngineQt::resizeGL( int w, int h )
{
	reshape( w, h );
}

void EngineQt::paintGL()
{
	idle();
	loadCameraMatrix();
	render();
}


//==============================================================================
//  CustomGLWidget
//==============================================================================

CustomGLWidget::CustomGLWidget( QWidget* parent, const QGLWidget* shareWidget, 
	                            Qt::WindowFlags f )
: QGLWidget( parent, shareWidget, f ),
  m_trackball(NULL),
  m_zoom     (  5.f ),
  m_zoomExp  (  0.f ),
  m_cameraFOV( 30.f )
{
	m_trackball = new Trackball( 0.f, QVector3D(0,1,0), Trackball::Sphere );
}

CustomGLWidget::~CustomGLWidget()
{
	delete m_trackball;
}

QPointF CustomGLWidget::pixelPosToViewPos(const QPointF& p)
{
    return QPointF(2.0 * float(p.x()) / width() - 1.0,
                   1.0 - 2.0 * float(p.y()) / height());
}

void CustomGLWidget::invokeRenderUpdate()
{
	makeCurrent();
	update();
	doneCurrent();
}

void CustomGLWidget::loadCameraMatrix()
{
	// trackball rotation + zoom
	QMatrix4x4 Mcam;
	Mcam.rotate( m_trackball->rotation() );
	Mcam(2,3) -= m_zoom * exp(m_zoomExp / 1200.f);  // (row,column)

	// camera
	makeCurrent();
	glLoadIdentity();	
	gl_multMatrix( Mcam );
}

void CustomGLWidget::mouseMoveEvent( QMouseEvent* e )
{	
	QGLWidget::mouseMoveEvent( e );
	if( e->isAccepted() )
		return;

	if( e->buttons() & Qt::LeftButton )
	{
		QPointF viewPos  = pixelPosToViewPos(e->pos());
		QQuaternion quat; // = (*m_trackball).rotation().conjugate();

		m_trackball->move( viewPos, quat );
		e->accept();

		invokeRenderUpdate();
	}
}

void CustomGLWidget::mousePressEvent( QMouseEvent* e )
{		
	QGLWidget::mouseMoveEvent( e );
	if( e->isAccepted() )
		return;

	if( e->buttons() & Qt::LeftButton )
	{
		QPointF viewPos  = pixelPosToViewPos(e->pos());
		QQuaternion quat;// = (*m_trackball).rotation().conjugate();

		m_trackball->push( viewPos, quat );
		e->accept();
		
		invokeRenderUpdate();
	}
}

void CustomGLWidget::mouseReleaseEvent( QMouseEvent* e )
{	
	QGLWidget::mouseMoveEvent( e );
	if( e->isAccepted() )
		return;

	QPointF viewPos  = pixelPosToViewPos(e->pos());

	if( e->button() == Qt::LeftButton )
	{
		QQuaternion quat; // = (*m_trackball).rotation().conjugate();
		//m_trackball->release( viewPos, quat );
		e->accept();
		invokeRenderUpdate();			
	}
}

void CustomGLWidget::wheelEvent( QWheelEvent* e )
{	
	QGLWidget::wheelEvent( e );
	if( e->isAccepted() )
		return;

	m_zoomExp += e->delta();
	if( m_zoomExp < -8 * 120 ) m_zoomExp = -8 * 120;
	if( m_zoomExp > 10 * 120 ) m_zoomExp = 10 * 120;
	e->accept();
	invokeRenderUpdate();		
}
