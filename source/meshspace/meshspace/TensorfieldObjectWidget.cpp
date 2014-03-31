#include "TensorfieldObjectWidget.h"
#include "TensorfieldObject.h"
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QComboBox>

using scene::TensorfieldObject;

TensorfieldObjectWidget::TensorfieldObjectWidget( QWidget* parent )
: QWidget( parent )
{
	m_dsbGlyphScale = new QDoubleSpinBox;
	m_dsbGlyphScale->setRange( 0.01, 1000. );
	m_dsbGlyphScale->setSingleStep( 0.5 );
	QLabel* lblGlyphScale = new QLabel(tr("Glyph scale"));
	lblGlyphScale->setBuddy( m_dsbGlyphScale );
	
	m_dsbGlyphSharpness = new QDoubleSpinBox;
	m_dsbGlyphSharpness->setRange( 0.1, 6.0 );
	m_dsbGlyphSharpness->setSingleStep( .5 );
	QLabel* lblGlyphSharpness = new QLabel(tr("Glyph sharpness"));
	lblGlyphSharpness->setBuddy( m_dsbGlyphSharpness );

	m_spbGlyphResolution = new QSpinBox;
	m_spbGlyphResolution->setRange( 8, 42 );
	m_spbGlyphResolution->setSingleStep( 8 );
	QLabel* lblGlyphResolution = new QLabel(tr("Glyph resolution"));
	lblGlyphResolution->setBuddy( m_spbGlyphResolution );

	m_chkGlyphSqrtEV = new QCheckBox;
	QLabel* lblGlyphSqrtEV = new QLabel(tr("Scale eigenvalues by sqrt"));
	lblGlyphSqrtEV->setBuddy( m_chkGlyphSqrtEV );

	m_cmbColorMode = new QComboBox;
	m_cmbColorMode->insertItems( 0, QStringList() << "FA" << "Cluster" );
	QLabel* lblColorMode = new QLabel(tr("Color mode"));
	lblColorMode->setBuddy( m_cmbColorMode );

	m_butLoadTensors = new QPushButton(tr("Load tensor field"));
	m_butSaveTensors = new QPushButton(tr("Save tensor field"));
	
	QGridLayout* l1 = new QGridLayout;
	int row=0;
	l1->addWidget( lblGlyphScale       , row,0 );
	l1->addWidget( m_dsbGlyphScale     , row,1 ); row++;
	l1->addWidget( lblGlyphSharpness   , row,0 );
	l1->addWidget( m_dsbGlyphSharpness , row,1 ); row++;
	l1->addWidget( lblGlyphResolution  , row,0 );
	l1->addWidget( m_spbGlyphResolution, row,1 ); row++;
	l1->addWidget( lblGlyphSqrtEV      , row,0 );
	l1->addWidget( m_chkGlyphSqrtEV    , row,1 ); row++;
	l1->addWidget( lblColorMode        , row,0 );
	l1->addWidget( m_cmbColorMode      , row,1 ); row++;	
	l1->addWidget( m_butLoadTensors    , row,0, 1,2 ); row++;
	l1->addWidget( m_butSaveTensors    , row,0, 1,2 ); row++;
	
	setLayout( l1 );

	connect( m_butLoadTensors, SIGNAL(clicked()), this, SLOT(loadTensors()) );
	connect( m_butSaveTensors, SIGNAL(clicked()), this, SLOT(saveTensors()) );
}

void TensorfieldObjectWidget::setMaster( TensorfieldObject* master )
{
	m_master = master;

	// Disconnect everything
	disconnect( m_dsbGlyphScale     , SIGNAL(valueChanged(double)), this, SLOT(updateMaster()) );
	disconnect( m_dsbGlyphSharpness , SIGNAL(valueChanged(double)), this, SLOT(updateMaster()) );
	disconnect( m_spbGlyphResolution, SIGNAL(valueChanged(int   )), this, SLOT(updateMaster()) );
	disconnect( m_chkGlyphSqrtEV    , SIGNAL(stateChanged(int   )), this, SLOT(updateMaster()) );
	disconnect( m_cmbColorMode      , SIGNAL(currentIndexChanged(int)), this, SLOT(updateMaster()) );

	// Enabled/disable GUI
	bool enabled = master!=NULL;
	this->setEnabled( enabled );
	
	if( enabled )
	{
		// Set GUI values
		m_dsbGlyphScale     ->setValue( master->getGlyphScale() );
		m_dsbGlyphSharpness ->setValue( master->getGlyphSharpness() );
		m_spbGlyphResolution->setValue( master->getGlyphResolution() );
		m_chkGlyphSqrtEV  ->setChecked( master->getGlyphSqrtEV() );
		m_cmbColorMode      ->setCurrentIndex( master->getColorMode() );
		
		// Reconnect everything
		connect( m_dsbGlyphScale     , SIGNAL(valueChanged(double)), this, SLOT(updateMaster()) );
		connect( m_dsbGlyphSharpness , SIGNAL(valueChanged(double)), this, SLOT(updateMaster()) );
		connect( m_spbGlyphResolution, SIGNAL(valueChanged(int   )), this, SLOT(updateMaster()) );
		connect( m_chkGlyphSqrtEV    , SIGNAL(stateChanged(int   )), this, SLOT(updateMaster()) );
		connect( m_cmbColorMode      , SIGNAL(currentIndexChanged(int)), this, SLOT(updateMaster()) );
	}
}

void TensorfieldObjectWidget::updateMaster()
{
	if( !m_master ) return; // Sanity		

	m_master->setGlyphScale     ( m_dsbGlyphScale     ->value() );
	m_master->setGlyphSharpness ( m_dsbGlyphSharpness ->value() );
	m_master->setGlyphResolution( m_spbGlyphResolution->value() );
	m_master->setGlyphSqrtEV    ( m_chkGlyphSqrtEV->isChecked() );
	m_master->setColorMode      ( m_cmbColorMode->currentIndex() );
	m_master->updateTensorfield();

	emit redrawRequired();
}

void TensorfieldObjectWidget::loadTensors()
{
	if( !m_master ) return;

	// Ask for filename
	QString filename = 
		QFileDialog::getOpenFileName( this, tr("Load tensor field"), "",
		tr("Tensor field matrix (*.tensorfield)") );
	if( filename.isEmpty() ) // User cancelled ?
		return;

	if( !m_master->loadTensorfield( filename.toStdString() ) )
	{
		QMessageBox::warning( this, tr("Error loading tensor field"),
			tr("Could not load a valid tensor field from \"%1\"!")
			.arg(filename) );
		return;
	}
}

void TensorfieldObjectWidget::saveTensors()
{
	if( !m_master ) return;

	// Ask for filename
	QString filename =
		QFileDialog::getSaveFileName( this, tr("Save tensor field"), "",
		tr("Tensor field matrix (*.tensorfield)") );
	if( filename.isEmpty() ) // User cancelled ?
		return;

	m_master->saveTensorfield( filename.toStdString() );
}

