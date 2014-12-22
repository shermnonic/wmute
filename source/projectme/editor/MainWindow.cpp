// MainWindow for projectme editor
// Max Hermann, Jul 2014
#include "glbase.h"
#include "MainWindow.h"
#include "ShaderModule.h"
#include "ParticleModule.h"
#include "PotentialFromImageModule.h"
#include "RenderSetWidget.h"
#include "ModuleManagerWidget.h"
#include "ModuleRendererWidget.h"
#include "MapperWidget.h"
#include "ModuleFactory.h"
#include "ProjectMe.h"
#include "ShaderEditorWidget.h"

#include <QtGui> // FIXME: Include only required Qt classes
#include <QMdiArea>
#include <QGLWidget>
#include <QDockWidget>
#include <QTimer>

const QString APP_NAME        ( "projectme" );
const QString APP_ORGANIZATION( "www.386dx25.de" );
#define       APP_ICON        QIcon(QPixmap(":/projectme.png"))

//=============================================================================
//  SharedGLContextWidget
//=============================================================================
#include <QGLFormat>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

SharedGLContextWidget::SharedGLContextWidget( QWidget* parent )
: QGLWidget( parent ),
  m_man(0)
{
	// Set multisampling option for OpenGL.
	// Note that it still has to enabled in OpenGL via glEnable(GL_MULTISAMPLE)
    QGLFormat glf = QGLFormat::defaultFormat();
    glf.setSampleBuffers(true);
    glf.setSamples(4);
    QGLFormat::setDefaultFormat(glf);

	// Render update timer
	m_renderUpdateTimer = new QTimer( this );
	connect( m_renderUpdateTimer, SIGNAL(timeout()), this, SLOT(updateGL()) );
	m_renderUpdateTimer->start( 42 );
}

void SharedGLContextWidget::initializeGL()
{
	// At first creation of the (shared) GL context, we have to initialize GLEW		
	static bool glewInitialized = false;
	
	if( !glewInitialized )
	{
		glewExperimental = GL_TRUE;	
		GLenum glew_err = glewInit();
		if( glew_err != GLEW_OK )
		{
			cerr << "GLEW Error:\n" << glewGetErrorString(glew_err) << endl;
			QMessageBox::warning( this, tr("%1 Error").arg(APP_NAME),
				tr("Could not setup GLEW OpenGL extension manager!\n") );
		}
		cout << "Using GLEW " << glewGetString( GLEW_VERSION ) << endl;
		if( !glewIsSupported("GL_VERSION_1_3") )
		{
			cerr << "GLEW Error:\n" << glewGetErrorString(glew_err) << endl;
			QMessageBox::warning( this, tr("%1 Warning").arg(APP_NAME),
				tr("OpenGL 1.3 not supported by current graphics hardware/driver!") );
		}

		glewInitialized = true;

		printf("------------\nOpenGL info:\n");
		printf(" Vendor  : %s\n",glGetString( GL_VENDOR ));
		printf(" Renderer: %s\n",glGetString( GL_RENDERER ));
		printf(" Version : %s\n",glGetString( GL_VERSION ));
		printf(" GLSL    : %s\n",glGetString( GL_SHADING_LANGUAGE_VERSION ));
		//printf(" Extensions: %s\n",glGetString( GL_EXTENSIONS ));
		printf("------------\n");
		
	}

	// Enable multisampling
	// Note that multisampling is configured via QGLFormat, usually in main.cpp.
	glEnable( GL_MULTISAMPLE );

	// OpenGL default states
	glClearColor(0,0,0,1);	
}

void SharedGLContextWidget::paintGL()
{	
	// All modules will be updated here
	if( m_man )		
		m_man->render();
}


//=============================================================================
//  MainWindow
//=============================================================================
MainWindow::MainWindow()
{
	// Create UI (widgets, menu, actions, connections)
	createUI();	

	// Load application settings
	readSettings();

	// Setup render set
	createRenderSet();

	// Default startup
	newPreview();
	m_moduleWidget->show();

	statusBar()->showMessage( tr("Ready.") );
}

MainWindow::~MainWindow()
{
}

void MainWindow::createRenderSet()
{
	RenderSet* set = m_renderSetManager.getActiveRenderSet();

#if 0 // PREDEFINED mappers and areas
  #if 0
	// Create hard-coded setup of N render areas with N ShaderModules
	const int N = 1;
	for( int i=0; i < N; i++ )
	{
		m_moduleManager.addModule( new ShaderModule );
	}
  #else
	// Create a particle system and a shader module
	ShaderModule* sm = new ShaderModule;
	ParticleModule* pm = new ParticleModule;
	//pm->setForceTexture( sm->target() ); // texture not created yet!
	m_moduleManager.addModule( sm );
	m_moduleManager.addModule( pm );
  #endif	
    unsigned n = (unsigned)m_moduleManager.modules().size();
	if( set )
	{
		set->clear();
        for( unsigned i=0; i < n; i++ )
		{
			float w = 2.f/ (float)n; // width
			RenderArea ra( (float)i*w+.1f-1.f, -.9f, (float)(i+1)*w-.1f-1.f, .9f );
			set->addArea( ra, m_moduleManager.modules().at(i) );
		}
	}
#endif

	// Update UI
	m_sharedGLWidget->setModuleManager( &m_moduleManager );
	m_moduleWidget  ->setModuleManager( &m_moduleManager );
	m_mapperWidget  ->setRenderSet( set );
	m_mapperWidget  ->setModuleManager( &m_moduleManager );
}

void MainWindow::createModule( int typeId )
{
	static ModuleFactory::ModuleTypeList availableModules = 
		ModuleFactory::ref().getAvailableModules();

	// Sanity 
	if( typeId < 0 || typeId >= availableModules.size() )
	{
		QMessageBox::warning( this, tr("%1 Warning").arg(APP_NAME),
			tr("Unknown module type!") );
		return;
	}

	// Create new module
	ModuleBase* m = ModuleFactory::ref().createInstance( availableModules[typeId] );
	if( !m )
	{
		QMessageBox::warning( this, tr("%1 Warning").arg(APP_NAME),
			tr("Could not create %1!").arg(QString::fromStdString(availableModules[typeId])) );
		return;
	}

	// So far we only support ModuleRenderer types
	if( !dynamic_cast<ModuleRenderer*>( m ) )
	{
		QMessageBox::warning( this, tr("%1 Warning").arg(APP_NAME),
			tr("Not a renderer module!") );
		return;
	}

	// Add to module manager	
	m_moduleManager.addModule( dynamic_cast<ModuleRenderer*>(m) );

	// Custom init
	customModuleInit( m );

	updateTables();
}

void MainWindow::customModuleInit()
{
	customModuleInit( getActiveModule() );
}

void MainWindow::customModuleInit( ModuleBase* m )
{
	// Sanity
	if( !m ) return;

	// Input image for potential field
	if( dynamic_cast<PotentialFromImageModule*>(m) )
	{
		PotentialFromImageModule* pfim = dynamic_cast<PotentialFromImageModule*>(m);
		QString filename = QFileDialog::getOpenFileName( this, tr("Load image"),
			tr(""), tr("*.png;*.jpg") );

		if( !filename.isEmpty() )
			pfim->loadImage( filename.toStdString().c_str() );
	}

	// Velocity texture for particle system
	if( dynamic_cast<ParticleModule*>(m) )
	{
		ParticleModule* pm = dynamic_cast<ParticleModule*>(m);

		// Get list of modules
		QStringList sl;
		for( int i=0; i < m_moduleManager.modules().size(); i++ )
		{
			const ModuleRenderer* mr = m_moduleManager.modules()[i];
			sl << (mr ? QString::fromStdString(mr->getName()) : "<Invalid module>");
		}

		// Let the user select a module
		bool ok;
		QString sel = QInputDialog::getItem( this, tr("Select input module"), 
			tr("Select input module for particle velocities"), sl, 0, false, &ok );
		if( ok )
		{
			int idx = sl.indexOf(sel);			
			pm->setForceTexture( m_moduleManager.modules()[idx]->target() );
		}
	}
}

void MainWindow::newArea()
{
	m_renderSetManager.getActiveRenderSet()->addArea( RenderArea() );	
	m_mapperWidget->updateTable(); // was: updateTables()
}

void MainWindow::updateTables()
{
	m_moduleWidget->updateModuleTable();
	m_mapperWidget->updateTable();
}

void MainWindow::createUI()
{
	setWindowTitle( APP_NAME );
	setWindowIcon( APP_ICON );

	// --- MDI ---

	m_sharedGLWidget = new SharedGLContextWidget(this);
	m_sharedGLWidget->resize(1,1);

	m_mdiArea = new QMdiArea(this);
	m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget( m_mdiArea );

	// --- widgets ---

	m_moduleWidget = new ModuleManagerWidget();
	m_moduleWidget->setWindowTitle(tr("Module Manager"));

	m_mapperWidget = new MapperWidget();
	m_mapperWidget->setWindowTitle(tr("Area Mapper"));

	m_moduleRendererWidget = new ModuleRendererWidget( this, m_sharedGLWidget );
	m_moduleRendererWidget->setWindowTitle(tr("Module Renderer"));

	// --- dock widgets ---

	QDockWidget* dockModuleManager = new QDockWidget(tr("Module Manager"),this);
	dockModuleManager->setWidget( m_moduleWidget );

	QDockWidget* dockAreaMapper = new QDockWidget(tr("Area Mapper"),this);
	dockAreaMapper->setWidget( m_mapperWidget );

	QDockWidget* dockModuleRenderer = new QDockWidget(tr("Module Renderer"),this);
	dockModuleRenderer->setWidget( m_moduleRendererWidget );

	addDockWidget( Qt::RightDockWidgetArea, dockModuleRenderer );
	addDockWidget( Qt::RightDockWidgetArea, dockModuleManager );
	addDockWidget( Qt::RightDockWidgetArea, dockAreaMapper );
	
	// --- actions ---

	QAction
		*actOpen,
		*actSave,
		*actQuit,
		*actNewPreview,
		*actNewScreen,
		*actLoadShader,
		*actReloadShader,
		*actEditShader,
		*actModuleInit,
		*actNewArea;
	
	actOpen = new QAction( tr("&Open project..."), this );
	actOpen->setShortcut( tr("Ctrl+O") );

	actSave = new QAction( tr("&Save project..."), this );
	actSave->setShortcut( tr("Ctrl+S") );
	
	actQuit = new QAction( tr("&Quit"), this );
	actQuit->setStatusTip( tr("Quit application.") );
	actQuit->setShortcut( tr("Ctrl+Q") );

	actNewPreview = new QAction( tr("New preview"), this );
	actNewScreen  = new QAction( tr("New screen"), this );

	actLoadShader = new QAction( tr("Load shader..."), this );
	actReloadShader = new QAction( tr("&Reload shader"), this );
	actReloadShader->setShortcut( tr("Ctrl+R") );

	actEditShader = new QAction( tr("Edit shader..."), this );	

	actModuleInit = new QAction( tr("Custom module init"), this );
	actModuleInit->setShortcut( tr("Ctrl+I") );

	actNewArea  = new QAction( tr("New area"), this );

	// --- build menu ---

	QMenu
		*menuFile,
		*menuWindows,
		*menuModules,
		*menuAreas;

	menuFile = menuBar()->addMenu( tr("&File") );
	menuFile->addAction( actOpen );
	menuFile->addAction( actSave );
	menuFile->addSeparator();
	menuFile->addAction( actQuit );

	menuWindows = menuBar()->addMenu( tr("&Windows") );
	menuWindows->addAction( dockModuleRenderer->toggleViewAction() );
	menuWindows->addAction( dockModuleManager ->toggleViewAction() );
	menuWindows->addAction( dockAreaMapper    ->toggleViewAction() );
	menuWindows->addSeparator();
	menuWindows->addAction( actNewPreview );
	menuWindows->addAction( actNewScreen );
	menuWindows->addSeparator();

	m_menuView = new QMenu( tr("Configure windows") );
	menuWindows->addMenu( m_menuView );	

	menuModules = menuBar()->addMenu( tr("&Modules") );
	menuModules->addAction( actLoadShader );
	menuModules->addAction( actReloadShader );
	menuModules->addAction( actEditShader );
	menuModules->addSeparator();
	menuModules->addAction( actModuleInit );
	menuModules->addSeparator();
	
	// "New module" menu entries + connection to signal mapper
	QSignalMapper* newModuleMapper = new QSignalMapper(this);
	ModuleFactory::ModuleTypeList availableModules = 
		ModuleFactory::ref().getAvailableModules();
	for( int i=0; i < availableModules.size(); i++ )
	{	
		// Action
		QAction* act = new QAction(
			tr("New %1").arg(
				QString::fromStdString(availableModules[i])),
			this );
		// Menu entry
		menuModules->addAction( act );
		// Connection to signal mapper
		connect( act, SIGNAL(triggered()), newModuleMapper, SLOT(map()) );
		newModuleMapper->setMapping( act, i );
	}

	menuAreas = menuBar()->addMenu( tr("Areas") );
	menuAreas->addAction( actNewArea );


	// --- connections ---

	connect( actOpen, SIGNAL(triggered()), this, SLOT(open() ) );
	connect( actSave, SIGNAL(triggered()), this, SLOT(save() ) );
	connect( actQuit, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()) );

	connect( actNewPreview, SIGNAL(triggered()), this, SLOT(newPreview()) );
	connect( actNewScreen,  SIGNAL(triggered()), this, SLOT(newScreen ()) );

	connect( actLoadShader,   SIGNAL(triggered()), this, SLOT(loadShader())   );
	connect( actReloadShader, SIGNAL(triggered()), this, SLOT(reloadShader()) );
	connect( actEditShader,   SIGNAL(triggered()), this, SLOT(editShader())   );

	connect( newModuleMapper, SIGNAL(mapped(int)), this, SLOT(createModule(int)) );

	connect( m_moduleWidget, SIGNAL(moduleNameChanged(int)), m_mapperWidget, SLOT(updateTable()) );
	connect( m_moduleWidget, SIGNAL(moduleChanged(ModuleRenderer*)), m_moduleRendererWidget, SLOT(setModuleRenderer(ModuleRenderer*)) );

	connect( actNewArea, SIGNAL(triggered()), this, SLOT(newArea()) );

	connect( actModuleInit, SIGNAL(triggered()), this, SLOT(customModuleInit()) );
}

void MainWindow::closeEvent( QCloseEvent* event )
{	
	// Destroy OpenGL resources
	m_sharedGLWidget->makeCurrent(); // Get OpenGL context

	m_mdiArea->closeAllSubWindows();
	if( m_mdiArea->currentSubWindow() )
	{
		event->ignore();
	}
	else
	{
		m_sharedGLWidget->makeCurrent(); // Get OpenGL context
		m_moduleManager.clear();

		writeSettings();
		event->accept();
	}
	//QMainWindow::closeEvent( event );
}

void MainWindow::writeSettings()
{
	QSettings settings( APP_ORGANIZATION, APP_NAME );
	settings.setValue( "geometry"   , saveGeometry() );
	settings.setValue( "windowState", saveState()    );
	settings.setValue( "baseDir"    , m_baseDir      );
}

void MainWindow::readSettings()
{
	QSettings settings( APP_ORGANIZATION, APP_NAME );
	m_baseDir = settings.value( "baseDir", QString("../data/") ).toString();
	restoreGeometry( settings.value("geometry")   .toByteArray() );
	restoreState   ( settings.value("windowState").toByteArray() );
	// for dock widgets use restoreDockWidget( .. );
}

void MainWindow::open()
{
	QString filename = QFileDialog::getOpenFileName( this, tr("Open project"),
		m_baseDir, tr("ProjectMe project file (*.projectme)") );
	
	if( filename.isEmpty() )
		return;
	
	// update baseDir
	QFileInfo info( filename );
	m_baseDir = info.absolutePath();

#if 0
	// clear modules and areas
	m_moduleManager.clear();
	m_renderSetManager.clear();
#endif

	// TBD: More general load code!
	ProjectMe pm;
	pm.setModuleManager( &m_moduleManager );
	pm.setRenderSetManager( &m_renderSetManager );

	m_sharedGLWidget->makeCurrent(); // Some deserializers require GL context!

	if( pm.deserializeFromDisk( filename.toStdString() ) )
	{
		// success		
		statusBar()->showMessage( tr("Sucessfully loaded %1").arg( filename ) );
	}
	else
	{
		// failure
		statusBar()->showMessage( tr("Failed to load %1!").arg( filename ) );
	}

	updateTables();
}

void MainWindow::save()
{
	QString filename = QFileDialog::getSaveFileName( this, tr("Save project"),
		m_baseDir, tr("ProjectMe project file (*.projectme)") );

	if( filename.isEmpty() )
		return;

	// Update baseDir
	QFileInfo info( filename );
	m_baseDir = info.absolutePath();
	
	// TBD: More general save code!
	ProjectMe pm;
	pm.setModuleManager( &m_moduleManager );
	pm.setRenderSetManager( &m_renderSetManager );
	pm.serializeToDisk( filename.toStdString() );
}

void MainWindow::editShader()
{
	ModuleBase* m = getActiveModule();
	if( dynamic_cast<ShaderModule*>(m) )
	{
		// Get shader source
		ShaderModule* sm = dynamic_cast<ShaderModule*>(m);
		QString source = QString::fromStdString( sm->getShaderSource() );
		QString name = QString::fromStdString(sm->getName());

		// Create new editor widget with source as document text
		ShaderEditorWidget* w = new ShaderEditorWidget( m_mdiArea );
		QMdiSubWindow* sub = m_mdiArea->addSubWindow( w );
		w->setShaderModule( sm );
		
		//w->setWindowTitle(tr("Editor %1").arg( name ));
		w->show();
	}
}

void MainWindow::newPreview()
{
	static int windowCount = 1;
	if( m_mdiArea->subWindowList().count() > 9 )
		return;

	RenderSetWidget* w = new RenderSetWidget( m_mdiArea, m_sharedGLWidget );
	w->setRenderSet( m_renderSetManager.getActiveRenderSet() );
	QMdiSubWindow* sub = m_mdiArea->addSubWindow( w );
	w->setWindowTitle("Preview #"+QString::number(windowCount++));
	w->show();
	sub->resize( 640, 480 );

	m_previews.append( w );
	updateViewMenu();
}

void MainWindow::newScreen()
{
	static int screenCount = 1;
	
	RenderSetWidget* w = new RenderSetWidget( 0, m_sharedGLWidget );
	w->setRenderSet( m_renderSetManager.getActiveRenderSet() );
	w->setWindowTitle("Screen #"+QString::number(screenCount++));
	w->show();

	m_screens.append( w );
	updateViewMenu();
}

void MainWindow::updateViewMenu()
{
	m_menuView->clear();

	for( int i=0; i < m_screens.size(); i++ )	
	{
		QAction* a = m_screens.at(i)->toggleFullscreenAction();
		a->setText( tr("Toggle fullscreen for screen #%1").arg(i+1) );
		m_menuView->addAction( a );
	}

	m_mapperWidget->updateTable();
}

void MainWindow::loadShader()
{
	QString filename = QFileDialog::getOpenFileName( this, tr("Load shader") );	
	if( filename.isEmpty() )
		return;

	m_shaderFilename = filename;

	reloadShader();
}

ModuleBase* MainWindow::getActiveModule()
{
	// Get module index
	int idx = m_moduleWidget->getActiveModuleIndex();
	if( idx < 0 ) 
	{
		QMessageBox::warning( this, tr("Warning"), tr("No module selected!") );
		return NULL;
	}
	if( idx >= m_moduleManager.modules().size() )
	{
		QMessageBox::warning( this, tr("Error"), tr("Selection out of range?!") );
		return NULL;
	}

	return (ModuleBase*)m_moduleManager.modules().at( idx );
}

void MainWindow::reloadShader()
{
	ModuleBase* mod = getActiveModule();
	if( !mod ) return;

	// Manually reload shader from disk (only for a ShaderModule!)
	ShaderModule* sm = dynamic_cast<ShaderModule*>(	mod );
	if( sm )
	{
		if( m_shaderFilename.isEmpty() )
			return;

		m_sharedGLWidget->makeCurrent();
		if( !sm->loadShader( m_shaderFilename.toStdString().c_str() ) )
			QMessageBox::warning( this, tr("Error"), tr("Failed to load and/or compile shader!") );
	}
	else
	{
		// Just "touch" the module
		m_sharedGLWidget->makeCurrent();
		mod->touch();		
	}
}
