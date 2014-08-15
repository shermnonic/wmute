// MainWindow for projectme editor
// Max Hermann, Jul 2014
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "glbase.h"
#include <QGLWidget>

#include <QMainWindow>
#include "RenderSet.h"

class QMdiArea;
class QGLWidget;
class QMenu;
class QTimer;

class RenderSetWidget;
class ModuleManagerWidget;
class ShaderModule;

/** @addtogroup editor_grp Editor
  * @{ */

//=============================================================================
//  SharedGLContext
//=============================================================================
/**
	\class SharedGLContextWidget
	- Provide OpenGL context shared between previews and screens.
	- Setup GL extension manager GLEW in initializeGL().
	- Frequent update of RenderModule's of a ModuleManager in paintGL().
*/
class SharedGLContextWidget : public QGLWidget
{
	Q_OBJECT
public:
	SharedGLContextWidget( QWidget* parent );

	void setModuleManager( ModuleManager* man ) { m_man = man; }

protected:
	///@name QGLWidget implementation
	///@{ 
	void initializeGL();
	void resizeGL( int w, int h ) {}
	void paintGL();
	QTimer* m_renderUpdateTimer;
	///@}

private:
	ModuleManager* m_man;
};

//=============================================================================
//  MainWindow
//=============================================================================
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();

public slots:
	void open();
	void save();

private slots:
	void newPreview();
	void newScreen();
	void updateViewMenu();
	
	void loadShader();
	void reloadShader();

protected:
	void createUI();
	void createRenderSet();

	void closeEvent( QCloseEvent* event );

	///@{ Application settings
	void readSettings();
    void writeSettings();
	QString m_baseDir; ///< directory of last successfully opened file
	///@}
	
private:
	QMdiArea        *m_mdiArea;
	SharedGLContextWidget
	                *m_sharedGLWidget;

	RenderSetManager m_renderSetManager;
	ModuleManager    m_moduleManager;

	ShaderModule    *m_shaderModule;
	QString          m_shaderFilename;
	
	QMenu* m_menuView;

	QList<RenderSetWidget*> m_previews;
	QList<RenderSetWidget*> m_screens;

	ModuleManagerWidget* m_moduleWidget;
};

/** @} */ // end group

#endif // MAINWINDOW
