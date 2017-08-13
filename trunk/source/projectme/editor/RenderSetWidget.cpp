#include "RenderSetWidget.h"
#include "glbase.h"
#include <iostream>
#include <QAction>
#include <QMenu>
#include <QTimer>
#include <QPoint>
#include <QMessageBox>
#include <QMouseEvent>
#include <QFileDialog>
#include <QDebug>

//=============================================================================
//  RenderSetWidget
//=============================================================================

//-----------------------------------------------------------------------------
RenderSetWidget::RenderSetWidget( QWidget *parent, QGLWidget *share )
: QGLWidget( parent, share ),  
  m_set( 0 ),  
  m_state( EditVertexState ),
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
	m_renderUpdateTimer->start( 16 ); // 16ms=60fps, 42ms=24fps

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
			if( m_flags & RenderGrid )
			{
				GLuint texid = 
					bindTexture( QPixmap(":/grid.png"), GL_TEXTURE_2D, GL_RGBA );
				m_set->render( texid );
			}
			else
				m_set->render();
		}

		if( m_flags & RenderPreview )
		{
			m_set->drawMask();
			m_set->drawOutline();
		}
	}
	
	float fps = m_fps.measure();
	setWindowTitle(tr("Renderer - %1 FPS").arg((unsigned)fps));
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

	if( !m_set ) 
		return;

	// Normalized coordinates in [-1,-1]-[1,1]
	QPointF pt = normalizedCoordinates( e->posF() );
	
	if( m_state == EditVertexState )
	{
		// Handle area editing interaction
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
				m_state = EditVertexState;

			e->accept();
		}
	}
	else if( m_state == PaintMaskState )
	{
		// Handle mask painting
		if( e->buttons() & Qt::LeftButton )
		{			
			m_set->paintMask( pt.x(), pt.y(), 23, !(e->modifiers() & Qt::ShiftModifier) );
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

	if( !m_set )
		return;

	// Normalized coordinates in [-1,-1]-[1,1]
	QPointF pt = normalizedCoordinates(e->pos());
	
	if( m_state == PickedVertexState )
	{	
		// Handle area editing interaction
		pt -= m_delta;
		if( e->buttons() & Qt::LeftButton )
		{
			m_set->setPickedVertexPosition( pt.x(), pt.y() );
			e->accept();
		}
	}
	else if( m_state == PaintMaskState )
	{
		// Handle mask painting
		if( e->buttons() & Qt::LeftButton )
		{			
			m_set->paintMask( pt.x(), pt.y(), 23, !(e->modifiers() & Qt::ShiftModifier) );
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
			m_state = EditVertexState;
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

#if 0
	QAction* a1 = menu.addAction( tr("Preview blue&yellow") );
	QAction* a2 = menu.addAction( tr("Preview black&white") );
	a1->setCheckable( true );
	a2->setCheckable( true );

	switch( m_set->getAreaMode() )	{
	case RenderSet::AreaOutline   : a1->setChecked(true); break;
	case RenderSet::AreaBlackWhite: a2->setChecked(true); break;
	}

	menu.addSeparator();
#endif

	QAction* m1 = menu.addAction( tr("Mask edit") );
	QAction* m2 = menu.addAction( tr("Mask clear white") );
	QAction* m3 = menu.addAction( tr("Mask clear black") );
    QAction* m4 = menu.addAction( tr("Load mask") );
    QAction* m5 = menu.addAction( tr("Save mask") );
    m1->setCheckable( true );
	m1->setChecked( m_state == PaintMaskState );

	menu.addSeparator();


	QAction* f1 = menu.addAction( tr("Render preview") );
	QAction* f2 = menu.addAction( tr("Render final") );	
	QAction* f3 = menu.addAction( tr("Render debug") );	
	f1->setCheckable( true );
	f2->setCheckable( true );
	f3->setCheckable( true );

	f1->setChecked( m_flags & RenderPreview );
	f2->setChecked( m_flags & RenderFinal   );
	f3->setChecked( m_flags & RenderGrid    );

	QAction* selectedItem = menu.exec( mapToGlobal(pt) );	
	if( selectedItem )
	{
#if 0
		if( selectedItem==a1 ) m_set->setAreaMode( RenderSet::AreaOutline ); else
		if( selectedItem==a2 ) m_set->setAreaMode( RenderSet::AreaBlackWhite ); else
#endif
		if( selectedItem==f1 ) m_flags = RenderPreview; else
		if( selectedItem==f2 ) m_flags = RenderFinal; else
		if( selectedItem==f3 ) m_flags = RenderFinal | RenderGrid;

		if( selectedItem==m1 ) m_state = m1->isChecked() ? PaintMaskState : EditVertexState;
		if( selectedItem==m2 ) m_set->clearMask( true );
		if( selectedItem==m3 ) m_set->clearMask( false );
        if( selectedItem==m4 )
        {
            QString filename = QFileDialog::getOpenFileName( this,
                tr("Open mask image"), QString(), tr("Images (*.png *.jpg)") );

            if( !filename.isEmpty() )
            {
                if( !m_set->loadMask( filename.toStdString().c_str() ) )
                    qDebug() << "Could not load " << filename << "!";
            }
        }
        if( selectedItem==m5 )
        {
            QString filename = QFileDialog::getSaveFileName( this,
                tr("Save mask image"), QString(), tr("Images (*.png *.jpg)") );

            if( !filename.isEmpty() )
            {
                if( !m_set->saveMask( filename.toStdString().c_str() ) )
                    qDebug() << "Could not save " << filename << "!";
            }
        }
    }
}

