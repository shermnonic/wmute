// MainWindow for projectme editor
// Max Hermann, Jul 2014
#include "glbase.h"
#include "MainWindow.h"
#include "ShaderModule.h"
#include "ParticleModule.h"
#include "RenderSetWidget.h"
#include "ModuleManagerWidget.h"
#include "ModuleFactory.h"
#include "ProjectMe.h"

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
	// Create hard-coded setup of 3 render areas with 3 ShaderModules
#if 1
    m_moduleManager.addModule( new ShaderModule );
    m_moduleManager.addModule( new ShaderModule );
    m_moduleManager.addModule( new ShaderModule );
#else
	m_moduleManager.addModule( new ParticleModule );
#endif
	RenderSet* set = m_renderSetManager.getActiveRenderSet();
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

	// Update UI
	m_sharedGLWidget->setModuleManager( &m_moduleManager );
	m_moduleWidget  ->setModuleManager( &m_moduleManager );
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

	m_moduleWidget->updateModuleTable();
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

	// --- dock widgets ---

	QDockWidget* dock = new QDockWidget(tr("Module Manager"),this);
	dock->setWidget( m_moduleWidget );
	addDockWidget( Qt::RightDockWidgetArea, dock );	
	
	// --- actions ---

	QAction
		*actOpen,
		*actSave,
		*actQuit,
		*actShowModuleManager,
		*actNewPreview,
		*actNewScreen,
		*actLoadShader,
		*actReloadShader;
	
	actOpen = new QAction( tr("&Open project..."), this );
	actOpen->setShortcut( tr("Ctrl+O") );

	actSave = new QAction( tr("&Save project..."), this );
	actSave->setShortcut( tr("Ctrl+S") );
	
	actQuit = new QAction( tr("&Quit"), this );
	actQuit->setStatusTip( tr("Quit application.") );
	actQuit->setShortcut( tr("Ctrl+Q") );

	actShowModuleManager = new QAction( tr("Show module manager"), this );

	actNewPreview = new QAction( tr("New preview"), this );
	actNewScreen  = new QAction( tr("New screen"), this );

	actLoadShader = new QAction( tr("Load shader..."), this );
	actReloadShader = new QAction( tr("&Reload shader"), this );
	actReloadShader->setShortcut( tr("Ctrl+R") );

	// --- build menu ---

	QMenu
		*menuFile,
		*menuWindows,
		*menuModules;

	menuFile = menuBar()->addMenu( tr("&File") );
	menuFile->addAction( actOpen );
	menuFile->addAction( actSave );
	menuFile->addSeparator();
	menuFile->addAction( actQuit );

	menuWindows = menuBar()->addMenu( tr("&Windows") );
	menuWindows->addAction( dock->toggleViewAction() );
	menuWindows->addSeparator();
	menuWindows->addAction( actNewPreview );
	menuWindows->addAction( actNewScreen );
	menuWindows->addSeparator();

	m_menuView = new QMenu( tr("Configure windows") );
	menuWindows->addMenu( m_menuView );	

	menuModules = menuBar()->addMenu( tr("&Modules") );
	menuModules->addAction( actLoadShader );
	menuModules->addAction( actReloadShader );
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

	// --- connections ---

	connect( actOpen, SIGNAL(triggered()), this, SLOT(open() ) );
	connect( actSave, SIGNAL(triggered()), this, SLOT(save() ) );
	connect( actQuit, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()) );

	connect( actNewPreview, SIGNAL(triggered()), this, SLOT(newPreview()) );
	connect( actNewScreen,  SIGNAL(triggered()), this, SLOT(newScreen ()) );

	connect( actLoadShader, SIGNAL(triggered()), this, SLOT(loadShader()) );
	connect( actReloadShader, SIGNAL(triggered()), this, SLOT(reloadShader()) );

	connect( newModuleMapper, SIGNAL(mapped(int)), this, SLOT(createModule(int)) );
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

#include "RenderSetWidget.h"
#include <QTextEdit>

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
}

void MainWindow::loadShader()
{
	QString filename = QFileDialog::getOpenFileName( this, tr("Load shader") );	
	if( filename.isEmpty() )
		return;

	m_shaderFilename = filename;

	reloadShader();
}

void MainWindow::reloadShader()
{
	// Get module index
	int idx = m_moduleWidget->getActiveModuleIndex();
	if( idx < 0 ) 
	{
		QMessageBox::warning( this, tr("Warning"), tr("No module selected!") );
		return;
	}
	if( idx >= m_moduleManager.modules().size() )
	{
		QMessageBox::warning( this, tr("Error"), tr("Selection out of range?!") );
		return;
	}

	// Manually reload shader from disk (only for a ShaderModule!)
	ShaderModule* sm = dynamic_cast<ShaderModule*>(	m_moduleManager.modules().at( idx ) );
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
		m_moduleManager.modules().at( idx )->touch();
	}
}
