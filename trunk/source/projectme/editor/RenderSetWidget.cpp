#include "RenderSetWidget.h"
#include "glbase.h"
#include <iostream>
#include <QAction>
#include <QMenu>
#include <QTimer>
#include <QPoint>
#include <QMessageBox>
#include <QMouseEvent>
#include <QDebug>

//=============================================================================
//  RenderSetWidget
//=============================================================================

//-----------------------------------------------------------------------------
RenderSetWidget::RenderSetWidget( QWidget *parent, QGLWidget *share )
: QGLWidget( parent, share ),  
  m_set( 0 ),  
  m_state( DefaultState ),
  m_flags( RenderPreview ),
  m_fullscreen( false )
{
	// Actions
	m_actFullscreen = new QAction(tr("Toggle fullscreen"),this);
	m_actFullscreen->setCheckable( true );
	m_actFullscreen->setChecked( m_fullscreen );
	connect( m_actFullscreen, SIGNAL(toggled(bool)), this, SLOT(toggleFullscreen(bool)) );

	// Context menu
	setContextMenuPolicy( Qt::CustomContextMenu );
	connect( this, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(showContextMenu(const QPoint&)) );

	// Render update timer
	m_renderUpdateTimer = new QTimer( this );
	connect( m_renderUpdateTimer, SIGNAL(timeout()), this, SLOT(updateGL()) );
	m_renderUpdateTimer->start( 42 );

	// DEBUG
	qDebug() << "The newly created RenderSetWidget" << 
		(isSharing() ? "is sharing!" : "is *not* sharing?!");
}

//-----------------------------------------------------------------------------
void RenderSetWidget::initializeGL()
{
	// The context should already be initialized correctly by the master
	// QGLWidget whose context this RenderSetWidget shares.
}

//-----------------------------------------------------------------------------
void RenderSetWidget::resizeGL( int w, int h )
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
void RenderSetWidget::paintGL()
{	
	glClearColor( 0,0,0,1 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	if( m_set )
	{
		if( m_flags & RenderFinal )
		{
			if( m_flags & RenderDebug )
			{
				GLuint texid = 
					bindTexture( QPixmap(":/grid.png"), GL_TEXTURE_2D, GL_RGBA );
				m_set->render( texid );
			}
			else
				m_set->render();
		}

		if( m_flags & RenderPreview )
			m_set->drawOutline();
	}
}

//-----------------------------------------------------------------------------
void RenderSetWidget::toggleFullscreen( bool enabled )
{
#if 0
    setWindowFlags( Qt::Window | (b ? Qt::FramelessWindowHint : Qt::Window) );
    move( 1280, 0 ); // FIXME: Hardcoded screen position
    show();
#else
    setWindowState( windowState() ^ Qt::WindowFullScreen );
#endif	
	m_fullscreen = enabled;
}

//-----------------------------------------------------------------------------
QPointF RenderSetWidget::normalizedCoordinates( QPointF pos )
{
	return QPointF( 
		2.f * pos.x() / width () - 1.f,
		2.f * (height()-pos.y()) / height() - 1.f );  // invert y-axis
}

//-----------------------------------------------------------------------------
void RenderSetWidget::mousePressEvent( QMouseEvent* e )
{		
	QGLWidget::mouseMoveEvent( e );
	if( e->isAccepted() )
		return;

	// Handle area editing interaction
	if( m_set )
	{
		// Normalized coordinates in [-1,-1]-[1,1]
		QPointF pt = normalizedCoordinates( e->posF() );
		
		if( e->buttons() & Qt::LeftButton )
		{
			if( m_set->pickVertex( pt.x(), pt.y() ) >= 0 )
			{
				m_state = PickedVertexState;
				
				// Delta between mouse and vertex position for translation
				float x,y;
				m_set->getPickedVertexPosition( x, y );
				m_delta = pt - QPointF( x, y );
			}
			else
				m_state = DefaultState;			
			
			e->accept();
		}
	}
}

//-----------------------------------------------------------------------------
void RenderSetWidget::mouseMoveEvent( QMouseEvent* e )
{		
	QGLWidget::mouseMoveEvent( e );
	if( e->isAccepted() )
		return;

	// Handle area editing interaction
	if( m_set && m_state == PickedVertexState )
	{
		// Normalized coordinates in [-1,-1]-[1,1]
		QPointF pt = normalizedCoordinates(e->pos()) - m_delta;		
		
		if( e->buttons() & Qt::LeftButton )
		{			
			m_set->setPickedVertexPosition( pt.x(), pt.y() );
			e->accept();
		}
	}
}

//-----------------------------------------------------------------------------
void RenderSetWidget::mouseReleaseEvent( QMouseEvent* e )
{	
	QGLWidget::mouseMoveEvent( e );
	if( e->isAccepted() )
		return;	

	// Handle area editing interaction
	if( m_state == PickedVertexState )
	{
		if( e->button() == Qt::LeftButton )
		{
			e->accept();
		}
	}
}

//-----------------------------------------------------------------------------
void RenderSetWidget::showContextMenu( const QPoint& pt )
{
	if( !m_set ) return;

	QMenu menu;
	menu.addAction( toggleFullscreenAction() );
	menu.addSeparator();
	QAction* a1 = menu.addAction( tr("Preview blue&yellow") );
	QAction* a2 = menu.addAction( tr("Preview black&white") );
	a1->setCheckable( true );
	a2->setCheckable( true );

	switch( m_set->getAreaMode() )	{
	case RenderSet::AreaOutline   : a1->setChecked(true); break;
	case RenderSet::AreaBlackWhite: a2->setChecked(true); break;
	}

	menu.addSeparator();
	QAction* f1 = menu.addAction( tr("Render preview") );
	QAction* f2 = menu.addAction( tr("Render final") );	
	QAction* f3 = menu.addAction( tr("Render debug") );	
	f1->setCheckable( true );
	f2->setCheckable( true );
	f3->setCheckable( true );

	f1->setChecked( m_flags & RenderPreview );
	f2->setChecked( m_flags & RenderFinal   );
	f3->setChecked( m_flags & RenderDebug  );

	QAction* selectedItem = menu.exec( mapToGlobal(pt) );	
	if( selectedItem )
	{
		if( selectedItem==a1 ) m_set->setAreaMode( RenderSet::AreaOutline ); else
		if( selectedItem==a2 ) m_set->setAreaMode( RenderSet::AreaBlackWhite ); else
		if( selectedItem==f1 ) m_flags = RenderPreview; else
		if( selectedItem==f2 ) m_flags = RenderFinal; else
		if( selectedItem==f3 ) m_flags = RenderFinal | RenderDebug;
	}
}

