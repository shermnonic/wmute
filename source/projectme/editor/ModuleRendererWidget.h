#ifndef MODULERENDERERWIDGET_H
#define MODULERENDERERWIDGET_H

#include "glbase.h"
#include <QGLWidget>

class ModuleRenderer;

/**
	\class ModuleRendererWidget
	
	Show render output of a single ModuleRenderer.
*/
class ModuleRendererWidget : public QGLWidget
{
	Q_OBJECT

public:
	ModuleRendererWidget( QWidget* parent, QGLWidget* share );

public slots:
	/// Set ModuleRenderer to show
	void setModuleRenderer( ModuleRenderer* mod );

protected:
	///@name QGLWidget implementation
	///@{ 
	void initializeGL();
	void resizeGL( int w, int h );
	void paintGL();
	QTimer* m_renderUpdateTimer;
	///@}

private:
	ModuleRenderer* m_moduleRenderer;
};

#endif // MODULERENDERERWIDGET_H
