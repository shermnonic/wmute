// MainWindow for projectme editor
// Max Hermann, Jul 2014
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "glbase.h"
#include <QGLWidget>

#include <QMainWindow>
#include "RenderSet.h"
#include "ModuleRenderer.h"
#include "ProjectMe.h"
#ifndef PROJECTME_BASS_DISABLED
#include "SoundInput.h"
#endif

class QMdiArea;
class QGLWidget;
class QMenu;
class QTimer;

class RenderSetWidget;
class ModuleManagerWidget;
class ModuleRendererWidget;
class ModuleParameterWidget;
class MapperWidget;
class NodeEditorWidget;
class ModuleBase;
class SoundInputWidget;
class SoundInput;

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

	void setUpdateEnabled( bool b );

protected:
	///@name QGLWidget implementation
	///@{ 
	void initializeGL();
    void resizeGL( int /*w*/, int /*h*/ ) {}
	void paintGL();
	QTimer* m_renderUpdateTimer;
	///@}

private:
	ModuleManager* m_man;
};

//=============================================================================
//  MainWindow
//=============================================================================
/**
	\class MainWindow

	ProjectMe application main window, owns a ProjectMe instance.
*/
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();

public slots:
	void open();
	void save();
	void clear();
	
	void openStyleSheet();

private slots:
	void destroy();

	void newPreview();
	void newScreen();
	void updateViewMenu();

	void updateTables();	
	void updateProject(); // Also calls updateTables()

	void loadShader();
	void reloadShader();
	void editShader();

	void createModule( int typeId );
	void customModuleInit();
	void customModuleInit( ModuleBase* m );
	void newArea();

	void forceRender();

protected:
	void createUI();

	void initialize();

	void closeEvent( QCloseEvent* event );

	///@{ Application settings
	void readSettings();
    void writeSettings();
	QString m_baseDir; ///< directory of last successfully opened file
	///@}
	
	ModuleBase* getActiveModule();

private:
	QMdiArea        *m_mdiArea;
	SharedGLContextWidget
	                *m_sharedGLWidget;

	QString          m_shaderFilename;
	
	QMenu* m_menuView;

	QList<RenderSetWidget*> m_previews;
	QList<RenderSetWidget*> m_screens;

	ProjectMe m_projectMe;

	ModuleManagerWidget*  m_moduleWidget;
	MapperWidget*         m_mapperWidget;
	ModuleRendererWidget* m_moduleRendererWidget;
	ModuleParameterWidget*m_moduleParameterWidget;
	NodeEditorWidget*     m_nodeEditorWidget;

#ifndef PROJECTME_BASS_DISABLED
	SoundInput m_soundInput;
	SoundInputWidget*     m_soundInputWidget;
#endif
};

/** @} */ // end group

#endif // MAINWINDOW
