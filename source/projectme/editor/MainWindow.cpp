// MainWindow for projectme editor
// Max Hermann, Jul 2014
#include "MainWindow.h"
#include "glbase.h"

#include <QtGui>
#include <QMdiArea>
#include <QGLWidget>
#include <QDockWidget>

const QString APP_NAME        ( "projectme" );
const QString APP_ORGANIZATION( "www.386dx25.de" );
#define       APP_ICON        QIcon(QPixmap(":/projectme.png"))

MainWindow::MainWindow()
{
	setWindowTitle( APP_NAME );
	setWindowIcon( APP_ICON );

	// --- MDI ---

	m_sharedGLWidget = new QGLWidget(this);
	m_sharedGLWidget->resize(1,1);

	m_mdiArea = new QMdiArea(this);
	m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget( m_mdiArea );

	// --- widgets ---

	// --- dock widgets ---
	
	// --- actions ---

	QAction
		*actOpen,
		*actSave,
		*actQuit,
		*actNewPreview,
		*actNewScreen;
	
	actOpen = new QAction( tr("&Open project..."), this );
	actOpen->setShortcut( tr("Ctrl+O") );

	actSave = new QAction( tr("&Save project..."), this );
	actSave->setShortcut( tr("Ctrl+S") );
	
	actQuit = new QAction( tr("&Quit"), this );
	actQuit->setStatusTip( tr("Quit application.") );
	actQuit->setShortcut( tr("Ctrl+Q") );

	actNewPreview = new QAction( tr("New preview"), this );
	actNewScreen  = new QAction( tr("New screen"), this );

	// --- build menu ---

	QMenu
		*menuFile,
		*menuWindows;

	menuFile = menuBar()->addMenu( tr("&File") );
	menuFile->addAction( actOpen );
	menuFile->addAction( actSave );
	menuFile->addSeparator();
	menuFile->addAction( actQuit );

	menuWindows = menuBar()->addMenu( tr("&Windows") );
	menuWindows->addAction( actNewPreview );
	menuWindows->addAction( actNewScreen );
	menuWindows->addSeparator();

	m_menuView = new QMenu( tr("Configure windows") );
	menuWindows->addMenu( m_menuView );	

	// --- connections ---

	connect( actOpen, SIGNAL(triggered()), this, SLOT(open() ) );
	connect( actSave, SIGNAL(triggered()), this, SLOT(save() ) );
	connect( actQuit, SIGNAL(triggered()), this, SLOT(close()) );	

	connect( actNewPreview, SIGNAL(triggered()), this, SLOT(newPreview()) );
	connect( actNewScreen,  SIGNAL(triggered()), this, SLOT(newScreen ()) );
	
	// --- finish up ---

	// load application settings
	readSettings();

	statusBar()->showMessage( tr("Ready.") );
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent( QCloseEvent* event )
{	
	m_mdiArea->closeAllSubWindows();
	if( m_mdiArea->currentSubWindow() )
	{
		event->ignore();
	}
	else
	{
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

	// TBD: Load code!
	if( true )
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
	
	// TBD: Save code!
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
	m_mdiArea->addSubWindow( w );
	w->setWindowTitle("Preview #"+QString::number(windowCount++));
	w->show();

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
	  #if 1
		QAction* a = m_screens.at(i)->toggleFullscreenAction();
	  #else
		QAction* a = new QAction( tr("Toggle fullscreen for screen #%1").arg(i), this );
		a->setCheckable( true );
		a->setChecked( m_screens.at(i)->isFullscreen() );
		connect( a, SIGNAL(toggled(bool)), m_screens.at(i), SLOT(toggleFullscreen(bool)) );
	  #endif

		m_menuView->addAction( a );
	}
}
