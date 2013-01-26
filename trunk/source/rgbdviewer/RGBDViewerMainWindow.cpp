#include "RGBDViewerMainWindow.h"
#include "RendererWidget.h"
#include "ParameterWidget.h"
#include <QtGui>

//-----------------------------------------------------------------------------
//	C'tor
//-----------------------------------------------------------------------------
RGBDViewerMainWindow::RGBDViewerMainWindow()
{
	setWindowTitle( APP_NAME );
	setWindowIcon ( APP_ICON );	

	//------------------------------------------------------------------------
	//	Setup movie rendering
	//------------------------------------------------------------------------

	// Establish filter pipeline
	setupFilters();

	// Create render window
	m_rendererWidget = new RendererWidget();
	m_rendererWidget->setFilter( &m_filters );

	// Setup animation timers
	m_playingTime = new QTime();
	m_animationTimer = new QTimer();	
	connect( m_animationTimer, SIGNAL(timeout()), this, SLOT(updateAnimation()) );

	//------------------------------------------------------------------------
	//	Dock widget
	//------------------------------------------------------------------------
	QDockWidget* dock = new QDockWidget( tr("Filter Parameters") );
	dock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
	
	m_parameterWidget = new ParameterWidget();
	dock->setWidget( m_parameterWidget );

	addDockWidget( Qt::RightDockWidgetArea, dock );

	// Setup parameter widget
	m_parameterWidget->setParameters( m_filters.getFloatParameters() );

	//------------------------------------------------------------------------
	//	Actions
	//------------------------------------------------------------------------
	QAction
		*actOpen,
		*actSave,
		*actQuit;
	
	actOpen = new QAction( tr("&Open RGBD raw datasets..."), this );
	actOpen->setShortcut( tr("Ctrl+O") );
	actOpen->setIcon( QIcon(QPixmap(":data/icons/document-open.png")) );
	connect( actOpen, SIGNAL(triggered()), this, SLOT(openDataset()) );
	
	actSave = new QAction( tr("&Save RGBD raw dataset..."), this );
	actSave->setShortcut( tr("Ctrl+S") );
	actSave->setIcon( QIcon(QPixmap(":data/icons/document-save.png")) );
	connect( actSave, SIGNAL(triggered()), this, SLOT(saveDataset()) );
	
	actQuit = new QAction( tr("&Quit"), this );
	actQuit->setShortcut( tr("Ctrl+Q") );
	actQuit->setIcon( QIcon(QPixmap(":data/icons/system-log-out.png")) );
	actQuit->setStatusTip( tr("Quit application.") );
	connect( actQuit, SIGNAL(triggered()), this, SLOT(close()) );

	// Global actions
	
	m_actPlayPause = new QAction( tr("&Play"), this );
	m_actPlayPause->setShortcut( tr("Ctrl+P") );
	m_actPlayPause->setIcon( QIcon(QPixmap(":data/icons/media-playback-start.png")) );;
	connect( m_actPlayPause, SIGNAL(triggered()), this, SLOT(togglePlayPause()) );

	//------------------------------------------------------------------------
	//	Toolbar
	//------------------------------------------------------------------------	
	QToolBar *toolbar = this->addToolBar(tr("Default toolbar"));
	toolbar->setObjectName(tr("Default toolbar"));
	toolbar->addAction( actOpen );
	toolbar->addAction( actQuit );
	toolbar->addSeparator();
	toolbar->addAction( m_actPlayPause );
	
	//------------------------------------------------------------------------
	//	Menu
	//------------------------------------------------------------------------	
	QMenu 
		*menuFile,
		*menuPlayback,
		*menuView;

	menuFile = menuBar()->addMenu( tr("&File") );
	menuFile->addAction( actOpen );
	menuFile->addAction( actSave );
	menuFile->addSeparator();
	menuFile->addAction( actQuit );

	menuPlayback = menuBar()->addMenu( tr("Playback") );
	menuPlayback->addAction( m_actPlayPause );

	menuView = menuBar()->addMenu( tr("View") );
	menuView->addAction( toolbar->toggleViewAction() );
	menuView->addAction( dock->toggleViewAction() );

	//------------------------------------------------------------------------
	//	Finish up
	//------------------------------------------------------------------------
	setCentralWidget( m_rendererWidget );
	readSettings();
}

//-----------------------------------------------------------------------------
//	closeEvent()
//-----------------------------------------------------------------------------
void RGBDViewerMainWindow::closeEvent( QCloseEvent* event )
{
	writeSettings();
	QMainWindow::closeEvent( event );
}

//-----------------------------------------------------------------------------
//	writeSettings()
//-----------------------------------------------------------------------------
void RGBDViewerMainWindow::writeSettings()
{
	QSettings settings( APP_ORGANIZATION, APP_NAME );
	settings.setValue( "geometry"   , saveGeometry() );
	settings.setValue( "windowState", saveState()    );
	settings.setValue( "baseDir"    , m_baseDir      );
}

//-----------------------------------------------------------------------------
//	readSettings()
//-----------------------------------------------------------------------------
void RGBDViewerMainWindow::readSettings()
{
	QSettings settings( APP_ORGANIZATION, APP_NAME );
	m_baseDir = settings.value( "baseDir", QString("../data/") ).toString();
	restoreGeometry( settings.value("geometry")   .toByteArray() );
	restoreState   ( settings.value("windowState").toByteArray() );
	//restoreDockWidget( .. );
}

//-----------------------------------------------------------------------------
//	openDataset()
//-----------------------------------------------------------------------------
void RGBDViewerMainWindow::openDataset()
{
	QString dir = QFileDialog::getExistingDirectory( this,
		tr("Open directory with RGBD data"),
		m_baseDir );
	
	// User cancelled?
	if( dir.isEmpty() )
		return;

	// Load RGBD movie
	m_rendererWidget->setRGBDFrame( NULL );
	if( m_movie.loadMovie( dir, RGBDMovie::FormatRGBDDemo, this ) )
	{
		// Successfully loaded movie

		// Show first frame
		m_rendererWidget->setRGBDFrame( m_movie.getFrame(0.0) );

		// Update history
		m_baseDir = dir;
	}
	else
	{
		// Error loading movie
		QMessageBox::warning( this, tr("Error"),
			tr("Could not load RGBD movie from:\n %1").arg(dir) );
	}
}

//-----------------------------------------------------------------------------
//	saveDataset()
//-----------------------------------------------------------------------------
void RGBDViewerMainWindow::saveDataset()
{
}

//-----------------------------------------------------------------------------
//	Playback controls
//-----------------------------------------------------------------------------

void RGBDViewerMainWindow::togglePlayPause()
{
	static bool playing = false;
	if( !playing )
	{
		// Switch to play mode, show Pause icon+text
		m_actPlayPause->setIcon( QIcon(QPixmap(":data/icons/media-playback-pause.png")) );
		m_actPlayPause->setText( tr("&Pause") );
		playing = true;
	}
	else
	{
		// Switch to pause mode, show Play icon+text
		m_actPlayPause->setIcon( QIcon(QPixmap(":data/icons/media-playback-start.png")) );
		m_actPlayPause->setText( tr("&Play") );
		playing = false;
	}

	playMovie( playing );
}

void RGBDViewerMainWindow::playMovie( bool toggle )
{
	if( toggle )
	{
		m_playingTime->start();
		m_animationTimer->start( 16.6 ); // 60fps
	}
	else	
	{
		m_animationTimer->stop();
	}
}

void RGBDViewerMainWindow::updateAnimation()
{
	float t0, t1;
	m_movie.getTimeRange( t0, t1 );
	
	float elapsed = m_playingTime->elapsed() / 1000.f;
	if( elapsed > (t1-t0) )
	{
		// Passed end of movie, default behaviour now is to restart from front
		m_playingTime->start();
		return;
	}

	// Show frame for current timepoint
	m_rendererWidget->setRGBDFrame( m_movie.getFrame( t0 + elapsed ) );

	// Render update is currently performed automatically at 60fps
}

//-----------------------------------------------------------------------------
//	setupFilters()
//-----------------------------------------------------------------------------
void RGBDViewerMainWindow::setupFilters()
{
	// Setup default filter pipeline
	// Note: Filter object instances must remain valid throughout the whole 
	//       programs execution!
	static RGBDDepthScale      filterDepthScale;
	static RGBDDepthThreshold  filterDepthTreshold;
	m_filters.push_back( &filterDepthTreshold );
	m_filters.push_back( &filterDepthScale );
	m_filters.registerParameters();
}
