#ifndef RENDERSETWIDGET_H
#define RENDERSETWIDGET_H

#include "glbase.h"
#include "RenderSet.h"
#include "FrameCounter.h"
#include <QGLWidget>
#include <QPointF>
#include <QPoint>

//=============================================================================
//  RenderSetWidget
//=============================================================================

class QMouseEvent;
class QWheelEvent;
class RenderSet;
class QTimer;
class QAction;

/**
	\class RenderSetWidget

	- Responsible for visualization and editing of a RenderSet.
	- Does *not* own the RenderSet but only a pointer to one.
*/
class RenderSetWidget : public QGLWidget
{
	Q_OBJECT

public:
	enum InteractionState { 
		EditVertexState,
		PickedVertexState, 
        PaintMaskState
	};

	enum RenderFlags {
		RenderPreview = 1,
		RenderFinal   = 2,
		RenderGrid    = 4
	};
	
	/// C'tor
	RenderSetWidget( QWidget *parent, QGLWidget *share );

	/// Set RenderSet, pointer must stay valid while it is assigned here!
	void setRenderSet( RenderSet* set ) { m_set = set; }	

	///@name Fullscreen
	///@{
	QAction* toggleFullscreenAction() { return m_actFullscreen; }
public slots:
	void toggleFullscreen( bool enabled );
	bool isFullscreen() const { return m_fullscreen; }
	///@}

	void showContextMenu( const QPoint& );

protected:
	///@name QGLWidget implementation
	///@{ 
	void initializeGL();
	void resizeGL( int w, int h );
	void paintGL();
	QTimer* m_renderUpdateTimer;
	///@}

	///@name Mouse events
	///@{
	QPointF normalizedCoordinates( QPointF pos );
	virtual void mousePressEvent  ( QMouseEvent* e );
	virtual void mouseReleaseEvent( QMouseEvent* e );
	virtual void mouseMoveEvent   ( QMouseEvent* e );
    virtual void wheelEvent( QWheelEvent* e );
	///@}

private:
	RenderSet* m_set;	
	int        m_state;
	int        m_flags;
	QPointF    m_delta;
	bool       m_fullscreen;
	QAction*   m_actFullscreen;
	FrameCounter m_fps;
    int        m_maskRadius;
	QPointF    m_cursorPos;
};

#endif // RENDERSETWIDGET_H
