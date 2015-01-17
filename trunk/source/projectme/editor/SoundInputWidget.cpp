#include "SoundInputWidget.h"
#include "SoundInput.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QToolBar>
#include <QAction>
#include <QHBoxLayout>
#include <QDebug>
#include <QtGui>


SoundInputWidget
  ::SoundInputWidget( QWidget* parent )
	: QWidget(parent),
	  m_master(NULL)
{
	QAction 
		*actOpenFile  = new QAction(tr("Open file"),this),
		*actOpenInput = new QAction(tr("Open input"),this),
		*actPlayPause = new QAction(tr("Play/pause"),this),
		*actRestart   = new QAction(tr("Restart"),this);
	
	actPlayPause->setCheckable( true );
	actPlayPause->setChecked( true );
	
	QToolBar* toolbar = new QToolBar(tr("Audio"),this);
	toolbar->addAction( actOpenFile );
	toolbar->addAction( actOpenInput );
	toolbar->addAction( actPlayPause );
	toolbar->addAction( actRestart );
	
	QVBoxLayout* l = new QVBoxLayout();
	l->addWidget( toolbar );
	l->setContentsMargins(0,0,0,0);
	setLayout( l );
	
	connect( actOpenFile, SIGNAL(triggered()), this, SLOT(openSoundFile()) );
	connect( actOpenInput,SIGNAL(triggered()), this, SLOT(openSoundInput()) );
	connect( actPlayPause,SIGNAL(toggled(bool)), this, SLOT(play(bool)) );
	connect( actRestart,  SIGNAL(triggered()), this, SLOT(restart()) );
}

void SoundInputWidget
  ::setSoundInput( SoundInput* master )
{
	disconnect();
	m_master = master;
}

void SoundInputWidget
  ::openSoundFile()
{
	if( !m_master ) return;
	
	// Query filename
	QString filename = QFileDialog::getOpenFileName( this, tr("SoundInput"),
		"", tr("Audio files (*.mp3;*.wav;*.ogg)") );
	if( filename.isEmpty() ) // User cancelled
		return;
	
	// Load audio file
	if( !m_master->openFile( filename.toStdString().c_str() ) )
	{
		QMessageBox::warning( this, tr("SoundInput"),
			tr("Could not open audio file!") );
		return;
	}
	
	// Start playback
	m_master->startInput();
	
	qDebug() << "SoundInputWidget::openSoundFile() : "
		"Started playing audio file \"" << filename << "\"";
}

void SoundInputWidget
  ::openSoundInput()
{
	if( !m_master ) return;
	
	// Record input from any source (e.g. microphone)
	if( !m_master->openInput() )
	{
		QMessageBox::warning( this, tr("SoundInput"),
			tr("Could not open sound source!") );
		return;
	}	
	
	// Start playback (not required for recording?)
	m_master->startInput();
}

void SoundInputWidget
  ::restart()
{
	if( m_master )
		m_master->restart();
}

void SoundInputWidget
  ::play( bool toggle )
{
	if( m_master )
		m_master->playPause( toggle );
}
