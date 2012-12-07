#ifndef ENGINEQT_H
#define ENGINEQT_H

#include "../Engine.h"
#include <GL/glew.h>
#include <QGLWidget>

// external classes
class Trackball;
// Qt forwards
class QMouseEvent;
class QWheelEvent;

//==============================================================================
//	CustomGLWidget
//==============================================================================
class CustomGLWidget : public QGLWidget
{
public:
	CustomGLWidget( QWidget* parent=0, const QGLWidget* shareWidget=0, 
		            Qt::WindowFlags f=0 );
	~CustomGLWidget();
public:
	void paint_() 
	{ 
		static bool initialized=false;
		if( !initialized )
		{
			initializeGL();
			initialized = true;
		}

		resizeGL( this->geometry().width(), this->geometry().height() );

		paintGL(); 
	}
protected:
	QPointF CustomGLWidget::pixelPosToViewPos(const QPointF& p);
	virtual void mousePressEvent  ( QMouseEvent* e );
	virtual void mouseReleaseEvent( QMouseEvent* e );
	virtual void mouseMoveEvent   ( QMouseEvent* e );
	virtual void wheelEvent       ( QWheelEvent* e );

	/// Custom render update function (for calls from outside of GL context)
	void invokeRenderUpdate();

	///@{ Trackball stuff
protected:
	void   loadCameraMatrix();
	double getCameraFOV() const { return m_cameraFOV; }

private:
	Trackball* m_trackball;
	double m_zoom;
	double m_zoomExp;
	double m_cameraFOV;	
	///@}

};

//==============================================================================
//	EngineQt
//==============================================================================
class EngineQt : public CustomGLWidget, public Engine 
{
	//Q_OBJECT
public:
	EngineQt( QWidget* parent=0, const QGLWidget* shareWidget=0, 
			  Qt::WindowFlags f=0 );
	virtual ~EngineQt();	

//------------------------------------------------------------------------------
///@{ Engine implementation
public:
	virtual void run( int argc, char* argv[] )
	{
		if( !init(argc,argv) )
			throw std::runtime_error("Initialization failed!");
	}
	
	virtual void set_window_title( const char* text ) 
	{
		setWindowTitle( QString(text) );
	}
	virtual void set_window_size ( int w, int h )     { /* do nothing */ }

	virtual int getMouseX()                           { return -1; }
	virtual int getMouseY()                           { return -1; }

	virtual void setFrequentUpdate( bool b )
	{
		setAutomaticRenderUpdate( false );
	}

protected:
	virtual void onKeyPressed( unsigned char key )    { /* do nothing */ }

	virtual void render ();
	virtual void idle   ()                            { /* do nothing */ }
	virtual void reshape( int w, int h );

	/// init() is called with valid OpenGL context
	virtual bool init( int argc, char* argv[] )       { return true; }
	virtual void destroy()                            { /* do nothing */ }

	/// Return milliseconds passed since last call to this function
	float get_time_delta() { assert(false); }
///@} // Engine implementation
//------------------------------------------------------------------------------	
	
//------------------------------------------------------------------------------
///@{ QGLWidget implementation
protected:	
	void initializeGL();
	void resizeGL( int w, int h );
	void paintGL();
	QTimer* m_renderUpdateTimer;
///@} // QGLWidget implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Custom functions:
public:
	void setAutomaticRenderUpdate( bool enable );
};

#endif ENGINE_H





