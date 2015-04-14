// MainWindow for projectme editor
// Max Hermann, Jul 2014
#include "glbase.h"
#include "MainWindow.h"
#include "ShaderModule.h"
#include "ParticleModule.h"
#include "ImageModule.h"
#include "PotentialFromImageModule.h"
#include "RenderSetWidget.h"
#include "ModuleManagerWidget.h"
#include "ModuleRendererWidget.h"
#include "ModuleParameterWidget.h"
#include "MapperWidget.h"
#include "ModuleFactory.h"
#include "ProjectMe.h"
#include "ShaderEditorWidget.h"
#include "NodeEditorWidget.h"
#ifndef PROJECTME_BASS_DISABLED
#include "SoundInputWidget.h"
#include "SoundModule.h"
#endif

#include <QtGui> // FIXME: Include only required Qt classes
#include <QMdiArea>
#include <QGLWidget>
#include <QDockWidget>
#include <QTimer>
#include <QApplication>

const QString APP_NAME        ( "projectme" );
const QString APP_ORGANIZATION( "www.386dx25.de" );
#define       APP_ICON        QIcon(QPixmap(":/data/icon.png"))

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
	// Set OpenGL profile.
	// Set multisampling option for OpenGL.
	// Note that it still has to enabled in OpenGL via glEnable(GL_MULTISAMPLE)
    QGLFormat glf = QGLFormat::defaultFormat();
	glf.setProfile( QGLFormat::CompatibilityProfile );
    glf.setSampleBuffers(true);
    glf.setSamples(4);
    QGLFormat::setDefaultFormat(glf);

	// Print some infos on first instantiation
	static bool firstCall = true;
	if( firstCall )
	{
		firstCall = false;

		// Profile
		QGLFormat::OpenGLContextProfile profile = glf.profile();
		QString sProfile("Unknown");
		switch( profile )
		{
		case QGLFormat::CoreProfile:           sProfile=QString("Core");          break;
		case QGLFormat::CompatibilityProfile:  sProfile=QString("Compatibility"); break;
		case QGLFormat::NoProfile:             sProfile=QString("No");            break;
		}

		// Version
		QGLFormat::OpenGLVersionFlags flags = glf.openGLVersionFlags();
		QString version("Unknown");
		if( flags & 0x00000000 ) version  = QString("OpenGL Version None");
		if( flags & 0x00000001 ) version  = QString("OpenGL Version 1.1");
		if( flags & 0x00000002 ) version  = QString("OpenGL Version 1.2");
		if( flags & 0x00000004 ) version  = QString("OpenGL Version 1.3");
		if( flags & 0x00000008 ) version  = QString("OpenGL Version 1.4");
		if( flags & 0x00000010 ) version  = QString("OpenGL Version 1.5");
		if( flags & 0x00000020 ) version  = QString("OpenGL Version 2.0");
		if( flags & 0x00000040 ) version  = QString("OpenGL Version 2.1");
		if( flags & 0x00001000 ) version  = QString("OpenGL Version 3.0");
		if( flags & 0x00002000 ) version  = QString("OpenGL Version 3.1");
		if( flags & 0x00004000 ) version  = QString("OpenGL Version 3.2");
		if( flags & 0x00008000 ) version  = QString("OpenGL Version 3.3");
		if( flags & 0x00010000 ) version  = QString("OpenGL Version 4.0");
		//if( flags & 0x00000100 ) version  = QString("OpenGL ES CommonLite Version 1 0"); else
		//if( flags & 0x00000080 ) version  = QString("OpenGL ES Common Version 1 0"); else
		//if( flags & 0x00000400 ) version  = QString("OpenGL ES CommonLite Version 1 1"); else
		//if( flags & 0x00000200 ) version  = QString("OpenGL ES Common Version 1 1"); else
		//if( flags & 0x00000800 ) version  = QString("OpenGL ES Version 2 0"); else

		qDebug() << "Using" << qPrintable(version) << qPrintable(sProfile) << "Profile";
	}

	// Render update timer
	m_renderUpdateTimer = new QTimer( this );
	connect( m_renderUpdateTimer, SIGNAL(timeout()), this, SLOT(updateGL()) );
	m_renderUpdateTimer->start( 42 ); // 16ms=60fps, 42ms=24fps
		// This is the "heart-beat" of the module system. No render module will
		// be updated faster than this rate. 
}

void SharedGLContextWidget::setRenderUpdateEnabled( bool b )
{
	if( !b )
		m_renderUpdateTimer->stop();
	else
		if( !m_renderUpdateTimer->isActive() )
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

		cout << "------------\nOpenGL info:\n";
		cout << " Vendor  : " << glGetString( GL_VENDOR ) << "\n";
		cout << " Renderer: " << glGetString( GL_RENDERER ) << "\n";;
		cout << " Version : " << glGetString( GL_VERSION ) << "\n";;
		cout << " GLSL    : " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << "\n";;
		//printf(" Extensions: %s\n",glGetString( GL_EXTENSIONS ));
		cout << "------------\n";
		
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
		m_man->update();
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

	// Initialize engine
	initialize();

	// Default startup
	newPreview();
	m_moduleWidget->show();

	statusBar()->showMessage( tr("Ready.") );
}

MainWindow::~MainWindow()
{
}

void MainWindow::initialize()
{
	// Setup ProjectMe
	RenderSet* set = m_projectMe.renderSetManager().getActiveRenderSet();
	if( set )
		set->setProjectMe( &m_projectMe );
	ModuleManager* mm = &m_projectMe.moduleManager();

#ifndef PROJECTME_BASS_DISABLED
	// Setup SoundInput
	if( m_soundInput.isInitialized() )
	{
		// BASS init succeeded
		m_soundInputWidget->setSoundInput( &m_soundInput );
	}
	else
	{
		// BASS init failed
		QMessageBox::warning( this, tr("ProjectMe"),
			tr("Could not setup audio device!") );
	}
#endif

	// Update UI
	m_sharedGLWidget->setModuleManager( mm );
	m_moduleWidget  ->setModuleManager( mm );
	m_mapperWidget  ->setRenderSet( set );
	m_mapperWidget  ->setModuleManager( mm );

	m_nodeEditorWidget->setProjectMe( &m_projectMe );
	m_nodeEditorWidget->updateNodes();
}

void MainWindow::createModule( int typeId )
{
	static ModuleFactory::ModuleTypeList availableModules = 
		ModuleFactory::ref().getAvailableModules();

	// Sanity 
    if( typeId < 0 || typeId >= (int)availableModules.size() )
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
	m_projectMe.moduleManager().addModule( dynamic_cast<ModuleRenderer*>(m) );

	// Custom init
	customModuleInit( m );

	// Update gui
	updateTables();
	m_nodeEditorWidget->setProjectMe( &m_projectMe );	
}

void MainWindow::customModuleInit()
{
	customModuleInit( getActiveModule() );
}

void MainWindow::customModuleInit( ModuleBase* m )
{
	// Sanity
	if( !m ) return;

#ifndef PROJECTME_BASS_DISABLED
	// Link SoundModule to global SoundInput instance
	if( dynamic_cast<SoundModule*>(m) )
	{
		SoundModule* sm = dynamic_cast<SoundModule*>(m);
		sm->setSoundInput( &m_soundInput );
	}
#endif

	// Input image for potential field
	if( dynamic_cast<PotentialFromImageModule*>(m) )
	{
		PotentialFromImageModule* pfim = dynamic_cast<PotentialFromImageModule*>(m);
		QString filename = QFileDialog::getOpenFileName( this, tr("Load image"),
            tr(""), tr("Images (*.png *.jpg)") );

		if( !filename.isEmpty() )
			pfim->loadImage( filename.toStdString().c_str() );
	}

	if( dynamic_cast<ImageModule*>(m) )
	{
		ImageModule* im = dynamic_cast<ImageModule*>(m);

		QString filename = QFileDialog::getOpenFileName( this, tr("Load image"),
            tr(""), tr("Images (*.png *.jpg)") );

		if( !filename.isEmpty() )
			im->loadImage( filename.toStdString().c_str() );
	}
}

void MainWindow::newArea()
{
	QStringList types;
	types << tr("Default area") 
		<< tr("UV split vertical left") << tr("UV split vertical right") 
		<< tr("UV split horizontal top") << tr("UV split horizontal bottom");
	bool ok;
	QString type = QInputDialog::getItem( this, tr("projectme - Create area"),
		tr("Select area type"), types, 0, false, &ok );
	if( ok && !type.isEmpty() )
	{
		RenderArea area;

		// Set UV split
		area.setUVSplit( types.indexOf(type) );

		m_projectMe.renderSetManager().getActiveRenderSet()->addArea( area );

		m_mapperWidget->updateTable(); // was: updateTables()
		m_nodeEditorWidget->updateNodes();
	}
}

void MainWindow::updateTables()
{
	m_moduleWidget->updateModuleTable();
	m_mapperWidget->updateTable();
}

QDockWidget* createDock( QWidget* parent, QWidget* w )
{
	QString title = w->windowTitle();
	QDockWidget* dock = new QDockWidget( title, parent );
	dock->setWidget( w );
	dock->setWindowTitle( title );
	dock->setObjectName( QString("Dock ")+title );
	return dock;
}

#include "QDebugStream.h"

QDockWidget* createLogDock( QWidget* parent )
{
	QTextEdit* te = new QTextEdit();
	te->setTextInteractionFlags( Qt::TextBrowserInteraction );
	te->setWindowTitle(parent->tr("Log"));
    te->setLineWrapMode( QTextEdit::NoWrap );
    te->setFontFamily( "Courier" );
    te->setFontPointSize( 9 );

	// TODO: Somehow consider also stream output before QTextEdit was created.
	//       E.g. at program start one could redirect cout/cerr to another 
	//       streambuf which is then read in when creating the QDebugStream
	//       here.
	QDebugStream 
		*qout = new QDebugStream( std::cout, te ),
		*qerr = new QDebugStream( std::cerr, te );

	// TODO: Also capture Qt outputs like qDebug() via qInstallMsgHandler().	

	return createDock( parent, te );
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

	m_moduleParameterWidget = new ModuleParameterWidget( this );
	m_moduleParameterWidget->setWindowTitle(tr("Module Parameters"));

	m_nodeEditorWidget = new NodeEditorWidget( this );
	m_nodeEditorWidget->setWindowTitle(tr("Node Editor"));

	// --- dock widgets ---

	QDockWidget
		*dockModuleManager    = createDock( this, m_moduleWidget ),
		*dockAreaMapper       = createDock( this, m_mapperWidget ),
		*dockModuleRenderer   = createDock( this, m_moduleRendererWidget ),
		*dockModuleParameters = createDock( this, m_moduleParameterWidget ),
		*dockNodeEditor       = createDock( this, m_nodeEditorWidget ),
		*dockLog              = createLogDock( this );

	addDockWidget( Qt::RightDockWidgetArea, dockModuleRenderer );
	addDockWidget( Qt::RightDockWidgetArea, dockModuleManager );
	addDockWidget( Qt::LeftDockWidgetArea, dockModuleParameters );
	addDockWidget( Qt::RightDockWidgetArea, dockAreaMapper );
	addDockWidget( Qt::BottomDockWidgetArea, dockNodeEditor );
	addDockWidget( Qt::BottomDockWidgetArea, dockLog );

	QList<QDockWidget*> docks;
	docks << dockModuleManager << dockAreaMapper << dockModuleRenderer
		  << dockModuleParameters << dockNodeEditor << dockLog;

#ifndef PROJECTME_BASS_DISABLED
    m_soundInputWidget = new SoundInputWidget( this );
    m_soundInputWidget->setWindowTitle(tr("Audio Input"));

	QDockWidget *dockSoundInput = createDock( this, m_soundInputWidget );
	addDockWidget( Qt::TopDockWidgetArea, dockSoundInput );
	docks << dockSoundInput;
#endif

	
	// --- actions ---

	QAction
		*actOpen,
		*actSave,
		*actClear,
		*actQuit,
		*actNewPreview,
		*actNewScreen,
		*actLoadShader,
		*actReloadShader,
		*actEditShader,
		*actModuleInit,
		*actNewArea,
		*actOpenStyleSheet;
	
	actOpen = new QAction( tr("&Open project..."), this );
	actOpen->setShortcut( tr("Ctrl+O") );

	actSave = new QAction( tr("&Save project..."), this );
	actSave->setShortcut( tr("Ctrl+S") );

	actClear = new QAction( tr("&New project"), this );
	
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

	actOpenStyleSheet = new QAction( tr("Open style sheet..."), this );

	// --- build menu ---

	QMenu
		*menuFile,
		*menuWindows,
		*menuModules,
		*menuAreas;

	menuFile = menuBar()->addMenu( tr("&File") );
	menuFile->addAction( actClear );
	menuFile->addAction( actOpen );
	menuFile->addAction( actSave );
	menuFile->addSeparator();
	menuFile->addAction( actOpenStyleSheet );
	menuFile->addSeparator();
	menuFile->addAction( actQuit );

	menuWindows = menuBar()->addMenu( tr("&Windows") );
	for( int i=0; i < docks.size(); i++ )
		menuWindows->addAction( docks[i]->toggleViewAction() );
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
    for( unsigned i=0; i < availableModules.size(); i++ )
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

	connect( actClear,SIGNAL(triggered()), this, SLOT(clear()) );
	connect( actOpen, SIGNAL(triggered()), this, SLOT(open() ) );
	connect( actSave, SIGNAL(triggered()), this, SLOT(save() ) );
	connect( actQuit, SIGNAL(triggered()), this, SLOT(close()) );

	connect( actOpenStyleSheet, SIGNAL(triggered()), this, SLOT(openStyleSheet()) );

	connect( actNewPreview, SIGNAL(triggered()), this, SLOT(newPreview()) );
	connect( actNewScreen,  SIGNAL(triggered()), this, SLOT(newScreen ()) );

	connect( actLoadShader,   SIGNAL(triggered()), this, SLOT(loadShader())   );
	connect( actReloadShader, SIGNAL(triggered()), this, SLOT(reloadShader()) );
	connect( actEditShader,   SIGNAL(triggered()), this, SLOT(editShader())   );

	connect( newModuleMapper, SIGNAL(mapped(int)), this, SLOT(createModule(int)) );

	connect( m_moduleWidget, SIGNAL(moduleNameChanged(int)), m_mapperWidget, SLOT(updateTable()) );
	connect( m_moduleWidget, SIGNAL(moduleNameChanged(int)), m_nodeEditorWidget, SLOT(updateNodes()) );
	connect( m_moduleWidget, SIGNAL(moduleChanged(ModuleRenderer*)), m_moduleRendererWidget, SLOT(setModuleRenderer(ModuleRenderer*)) );
	connect( m_moduleWidget, SIGNAL(moduleChanged(ModuleBase*)), m_moduleParameterWidget, SLOT(setModule(ModuleBase*)) );

	connect( actNewArea, SIGNAL(triggered()), this, SLOT(newArea()) );

	connect( actModuleInit, SIGNAL(triggered()), this, SLOT(customModuleInit()) );

	connect( m_nodeEditorWidget, SIGNAL(connectionChanged()), this, SLOT(updateTables()) );
	connect( m_nodeEditorWidget, SIGNAL(selectionChanged(ModuleRenderer*)), m_moduleWidget, SLOT(setActiveModule(ModuleRenderer*)) );
}

void MainWindow::destroy()
{
	static bool destroyed = false;
	if( destroyed ) return;

	// Avoid any GL render calls
	disconnect( m_moduleWidget );
	m_sharedGLWidget->setRenderUpdateEnabled( false ); // Stop timer update
	m_sharedGLWidget->setUpdatesEnabled( false ); // Prohibit updateGL() calls
    //qApp->processEvents();

	// Destroy OpenGL resources
	m_sharedGLWidget->makeCurrent(); // Get OpenGL context

	// Close screens
	for( int i=0; i < m_screens.size(); i++ )
	{
		if( m_screens[i] )
			m_screens[i]->close();
	}
	m_screens.clear();

	// Close MDI windows
	m_mdiArea->closeAllSubWindows();
	//for( unsigned i=0; i < m_mdiArea->subWindowList().size(); i++ )
		//m_mdiArea->subWindowList()[i]->close();


	m_sharedGLWidget->makeCurrent(); // Get OpenGL context
	m_projectMe.clear();

	destroyed = true;
}

void MainWindow::closeEvent( QCloseEvent* event )
{
	destroy();
	writeSettings();
	QMainWindow::closeEvent( event );
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

void MainWindow::openStyleSheet()
{
	QString filename = QFileDialog::getOpenFileName( this, tr("Open stylesheet"),
		m_baseDir, tr("Qt style sheet (*.css;*.qss)") );
	
	if( filename.isEmpty() )
		return;

	QFile f( filename );	
	if( f.open(QIODevice::ReadOnly | QIODevice::Text) )
	{		
		qDebug() << "Setting style sheet" << filename;
		QString style = QTextStream(&f).readAll();
		qApp->setStyleSheet( style );
	}
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

	m_sharedGLWidget->makeCurrent(); // Some deserializers require GL context!

	if( m_projectMe.deserializeFromDisk( filename.toStdString() ) )
	{
		// success		
		statusBar()->showMessage( tr("Sucessfully loaded %1").arg( filename ) );
	}
	else
	{
		// failure
		statusBar()->showMessage( tr("Failed to load %1!").arg( filename ) );
	}

	updateProject();
}

void MainWindow::clear()
{
	// TODO: Show "Are you sure?" dialog

	m_nodeEditorWidget->setProjectMe( NULL );
	m_projectMe.clear();
	updateProject();
}

void MainWindow::updateProject()
{
	// Update GUI.
	updateTables();	
	m_nodeEditorWidget->setProjectMe( &m_projectMe );
	m_nodeEditorWidget->updateNodes();
	m_nodeEditorWidget->updateConnections();

	// Trigger render to force GL initialization of all modules. This results
	// in valid texture ids for all channels.
	forceRender();	
	m_projectMe.touchConnections();
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
	
	// Update node editor infos (e.g. set position attribute of ModuleRenderer nodes)
	m_nodeEditorWidget->updateNodes();

	m_projectMe.serializeToDisk( filename.toStdString() );
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

		connect( w, SIGNAL(shaderUpdated(ModuleBase*)), m_moduleParameterWidget, SLOT(setModule(ModuleBase*)) );
	}
}

void MainWindow::newPreview()
{
	static int windowCount = 1;
	if( m_mdiArea->subWindowList().count() > 9 )
		return;

	RenderSetWidget* w = new RenderSetWidget( m_mdiArea, m_sharedGLWidget );
	w->setRenderSet( m_projectMe.renderSetManager().getActiveRenderSet() );
	QMdiSubWindow* sub = m_mdiArea->addSubWindow( w );
	sub->setAttribute( Qt::WA_DeleteOnClose );
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
	w->setAttribute( Qt::WA_DeleteOnClose );
	w->setRenderSet( m_projectMe.renderSetManager().getActiveRenderSet() );
	w->setWindowTitle("Screen #"+QString::number(screenCount++));
	w->show();

	m_screens.append( w );
	updateViewMenu();
}

void MainWindow::forceRender()
{
	m_sharedGLWidget->updateGL();
	m_sharedGLWidget->updateGL();
	m_sharedGLWidget->updateGL();
	QApplication::processEvents();
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
    if( idx >= (int)m_projectMe.moduleManager().modules().size() )
	{
		QMessageBox::warning( this, tr("Error"), tr("Selection out of range?!") );
		return NULL;
	}

	return (ModuleBase*)m_projectMe.moduleManager().modules().at( idx );
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
