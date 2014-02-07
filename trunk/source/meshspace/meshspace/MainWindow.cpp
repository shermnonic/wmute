// MainWindow for meshspace application
// Max Hermann, Jan 2014
#include "MainWindow.h"
#include "SceneViewer.h"

#include <QtGui>
#include <QStringList>
#include <QProgressDialog>

const QString APP_NAME        ( "meshspace" );
const QString APP_ORGANIZATION( "University Bonn Computer Graphics" );
#define       APP_ICON        QIcon(QPixmap(":/meshspace.png"))

MainWindow::MainWindow()
{
	setWindowTitle( APP_NAME );
	setWindowIcon( APP_ICON );

	// --- widgets ---

	m_viewer = new SceneViewer;
	
	// --- actions ---

	QAction
		*actOpenMesh,
		*actOpenAnimation,
		*actSaveMesh,
		*actQuit,
		*actShowSceneBrowser,
		*actShowSceneInspector;
	
	actOpenMesh = new QAction( tr("&Open meshes..."), this );
	actOpenMesh->setStatusTip( tr("Add one ore more 3D mesh (as separate mesh objects) to the current scene.") );
	actOpenMesh->setShortcut( tr("Ctrl+O") );

	actOpenAnimation = new QAction( tr("Open mesh &animation..."), this );
	actOpenAnimation->setStatusTip( tr("Open multiple meshes with same connectivity which are interpreted as mesh animation.") );
	actOpenAnimation->setShortcut( tr("Ctrl+A") );

	actSaveMesh = new QAction( tr("&Save mesh or mesh animation..."), this );
	actSaveMesh->setStatusTip( tr("&Save currently selected mesh object in binary MESHBUFFER format.") );
	actSaveMesh->setShortcut( tr("Ctrl+S") );
	
	actQuit = new QAction( tr("&Quit"), this );
	actQuit->setStatusTip( tr("Quit application.") );
	actQuit->setShortcut( tr("Ctrl+Q") );

	actShowSceneBrowser = new QAction( tr("Show scene &browser"), this );
	actShowSceneBrowser->setShortcut( tr("Ctrl+B") );

	actShowSceneInspector = new QAction( tr("Show scene &inspector"), this );
	actShowSceneInspector->setShortcut( tr("Ctrl+I") );

	// --- build menu ---

	QMenu
		*menuFile,
		*menuWindows;

	menuFile = menuBar()->addMenu( tr("&File") );
	menuFile->addAction( actOpenMesh );
	menuFile->addAction( actOpenAnimation );
	menuFile->addAction( actSaveMesh );
	menuFile->addSeparator();
	menuFile->addAction( actQuit );

	menuWindows = menuBar()->addMenu( tr("&Windows") );
	menuWindows->addAction( actShowSceneBrowser );
	menuWindows->addAction( actShowSceneInspector );

	// --- connections ---

	connect( actOpenMesh,      SIGNAL(triggered()), this, SLOT(openMesh()) );
	connect( actOpenAnimation, SIGNAL(triggered()), this, SLOT(openAnimation()) );
	connect( actSaveMesh,      SIGNAL(triggered()), this, SLOT(saveMesh()) );
	connect( actQuit,          SIGNAL(triggered()), this, SLOT(close()) );
	connect( actShowSceneBrowser,   SIGNAL(triggered()), m_viewer, SLOT(showBrowser()) );
	connect( actShowSceneInspector, SIGNAL(triggered()), m_viewer, SLOT(showInspector()) );
	
	// --- layout ---

	QVBoxLayout* l = new QVBoxLayout;
	l->addWidget( m_viewer );
	l->setContentsMargins(0,0,0,0);
	
	QWidget* centralWidget = new QWidget;
	centralWidget->setLayout( l );
	m_viewer     ->setContentsMargins(0,0,0,0);
	centralWidget->setContentsMargins(0,0,0,0);
	setCentralWidget( centralWidget );
	
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

void MainWindow::openMesh()
{
#if 0 // SINGLE SELECTION
	QString filename = QFileDialog::getOpenFileName( this, tr("Open mesh"),
		m_baseDir, tr("3D Meshes (*.obj *.ply *.off *.om *.stl)") );
	
	if( filename.isEmpty() )
		return;
	
	// update baseDir
	QFileInfo info( filename );
	m_baseDir = info.absolutePath();

	if( m_viewer->loadMesh( filename ) )
	{
		// success		
		statusBar()->showMessage( tr("Sucessfully loaded %1").arg( filename ) );
	}
	else
	{
		// failure
		statusBar()->showMessage( tr("Failed to load %1!").arg( filename ) );
	}
#else // MULTI SELECTION
	QStringList names = QFileDialog::getOpenFileNames( this, tr("Open mesh(es)"),
		m_baseDir, tr("3D Meshes (*.obj *.ply *.off *.om *.stl)") );

	if( names.isEmpty() )
		return;

	// Update baseDir
	QFileInfo info( names[0] );
	m_baseDir = info.absolutePath();

	// Progress dialog
	QProgressDialog progress(tr("Loading meshes..."), 
		tr("Abort at current file"), 0, names.size(), this );
	progress.setWindowModality( Qt::WindowModal );
	progress.setValue(0);
	progress.show();
	QApplication::processEvents();

	int i;
	for( i=0; i < names.size(); ++i )
	{
		progress.setValue( i );

		if( progress.wasCanceled() )
			break;

		if( !m_viewer->loadMesh( names[i] ) )
		{
			// Failed to load mesh
			QMessageBox::warning( this, tr("Warning"), 
				tr("Could not load %1!").arg(names[i]) );
		}
	}
	progress.setValue( names.size() );

	statusBar()->showMessage( tr("Succesfully loaded %1 meshes.").arg(i) );
#endif
}

void MainWindow::openAnimation()
{
	QStringList names = QFileDialog::getOpenFileNames( this, tr("Open mutliple meshes with same connectivity"),
		m_baseDir, tr("3D Meshes (*.obj *.ply *.off *.om *.stl);; MeshBuffer (*.mb *.meshbuffer)") );

	if( names.isEmpty() )
		return;

	// Update baseDir
	QFileInfo info( names[0] );
	m_baseDir = info.absolutePath();

	int i = m_viewer->loadMeshAnimation( names );
	statusBar()->showMessage( tr("Succesfully loaded %1 meshes.").arg(i) );
}

void MainWindow::saveMesh()
{
	QString filename = QFileDialog::getSaveFileName( this, tr("Save mesh buffer"),
		m_baseDir, tr("MeshBuffer (*.mb *.meshbuffer)") );

	if( filename.isEmpty() )
		return;

	// Update baseDir
	QFileInfo info( filename );
	m_baseDir = info.absolutePath();

	m_viewer->saveMeshBuffer( filename );
}
