#ifndef VIEWERWIDGET_H
#define VIEWERWIDGET_H

#include "base/ViewerInterface.h"
#include <QGLWidget>

class QTimer;

class ViewerWidget : public QGLWidget, public ViewerInterface
{
public:
	ViewerWidget( QWidget* parent=0, const QGLWidget* shareWidget=0, 
	              Qt::WindowFlags f=0 );

	///@{ ViewerInterface implementation
	void setRenderer( AbstractRenderer* scene );
	void setInteractor( AbstractInteractor* interactor );
	///@}

protected:
	///@{ QGLWidget implementation
	void initializeGL();
	void resizeGL( int w, int h );
	void paintGL();
	///@}

	///@{ Mouse events
	virtual void mousePressEvent  ( QMouseEvent* e );
	virtual void mouseReleaseEvent( QMouseEvent* e );
	virtual void mouseMoveEvent   ( QMouseEvent* e );
	virtual void wheelEvent       ( QWheelEvent* e );
	///@}
	
private:
	AbstractRenderer*   m_renderer;
	AbstractInteractor* m_interactor;
	QTimer* m_renderUpdateTimer;
};

#endif // VIEWERWIDGET_H
