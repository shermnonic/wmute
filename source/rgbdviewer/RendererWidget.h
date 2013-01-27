#ifndef RENDERERWIDGET_H
#define RENDERERWIDGET_H

#include <GL/glew.h>  // include glew before QGLWidget
#include <GL/glu.h>
#include <QGLWidget>
#include "glutils/Trackball2.h"

class RGBDFrame;
class RGBDLocalFilter;

/// Render a single \a RGBDFrame
class RendererWidget : public QGLWidget
{
	Q_OBJECT
	
public:
	enum InteractionMode { ModeTrackball };

	RendererWidget( QWidget* parent=0, const QGLWidget* shareWidget=0, 
	                Qt::WindowFlags f=0 );

	void setRGBDFrame( RGBDFrame* frame ) { m_frame = frame; }

	void setFilter( RGBDLocalFilter* filter ) { m_filter = filter; }
	
	/// Choose one of \a RGBDFrame::RenderModes
	void setRenderMode( int mode ) { m_renderMode = mode; }
public slots:
	// provided for convenience
	void setRenderModeSurface();
	void setRenderModePoints();

	void toggleBlackBackground( bool );

protected:
	void invokeRenderUpdate() { };

	///@{ QGLWidget implementation
	void initializeGL();
	void resizeGL( int w, int h );
	void paintGL();
	QTimer* m_renderUpdateTimer;
	///@}

	///@{ Mouse events
	virtual void mousePressEvent  ( QMouseEvent* e );
	virtual void mouseReleaseEvent( QMouseEvent* e );
	virtual void mouseMoveEvent   ( QMouseEvent* e );
	virtual void wheelEvent       ( QWheelEvent* e );
	///@}

private:
	Trackball2       m_trackball;
	InteractionMode  m_mode;
	RGBDFrame*       m_frame;
	RGBDLocalFilter* m_filter;
	int              m_renderMode; // one of RGBDFrame::RenderModes
};

#endif // RENDERERWIDGET_H
