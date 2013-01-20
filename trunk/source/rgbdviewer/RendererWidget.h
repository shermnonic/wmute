#ifndef RENDERERWIDGET_H
#define RENDERERWIDGET_H

#include <GL/glew.h>  // include glew before QGLWidget
#include <GL/glu.h>
#include <QGLWidget>
#include "glutils/Trackball2.h"

class RGBDFrame;

/// Render a single \a RGBDFrame
class RendererWidget : public QGLWidget
{
	Q_OBJECT
	
public:
	enum InteractionMode { ModeTrackball };

	RendererWidget( QWidget* parent=0, const QGLWidget* shareWidget=0, 
	                Qt::WindowFlags f=0 );

	void setRGBDFrame( RGBDFrame* frame ) { m_frame = frame; }

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
	Trackball2      m_trackball;
	InteractionMode m_mode;
	RGBDFrame*      m_frame;
};

#endif // RENDERERWIDGET_H
