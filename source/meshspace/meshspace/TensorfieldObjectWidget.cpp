#include "TensorfieldObjectWidget.h"
#include "TensorfieldObject.h"
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>

using scene::TensorfieldObject;

TensorfieldObjectWidget::TensorfieldObjectWidget( QWidget* parent )
: QWidget( parent )
{
	m_dsbGlyphScale = new QDoubleSpinBox;
	m_dsbGlyphScale->setRange( 0.01, 10. );
	QLabel* lblGlyphScale = new QLabel(tr("Glyph scale"));
	lblGlyphScale->setBuddy( m_dsbGlyphScale );
	
	m_dsbGlyphSharpness = new QDoubleSpinBox;
	m_dsbGlyphSharpness->setRange( 0.1, 6.0 );
	QLabel* lblGlyphSharpness = new QLabel(tr("Glyph sharpness"));
	lblGlyphSharpness->setBuddy( m_dsbGlyphSharpness );

	m_spbGlyphResolution = new QSpinBox;
	m_spbGlyphResolution->setRange( 6, 42 );
	QLabel* lblGlyphResolution = new QLabel(tr("Glyph resolution"));
	lblGlyphResolution->setBuddy( m_spbGlyphResolution );
	
	QGridLayout* l1 = new QGridLayout;
	int row=0;
	l1->addWidget( lblGlyphScale  , row,0 );
	l1->addWidget( m_dsbGlyphScale, row,1 ); row++;
	l1->addWidget( lblGlyphSharpness  , row,0 );
	l1->addWidget( m_dsbGlyphSharpness, row,1 ); row++;
	l1->addWidget( lblGlyphResolution  , row,0 );
	l1->addWidget( m_spbGlyphResolution, row,1 ); row++;
	
	setLayout( l1 );
}

void TensorfieldObjectWidget::setMaster( TensorfieldObject* master )
{
	m_master = master;

	// Disconnect everything
	disconnect( m_dsbGlyphScale     , SIGNAL(valueChanged(double)), this, SLOT(updateMaster()) );
	disconnect( m_dsbGlyphSharpness , SIGNAL(valueChanged(double)), this, SLOT(updateMaster()) );
	disconnect( m_spbGlyphResolution, SIGNAL(valueChanged(int   )), this, SLOT(updateMaster()) );

	// Enabled/disable GUI
	bool enabled = master!=NULL;
	this->setEnabled( enabled );
	
	if( enabled )
	{
		// Set GUI values
		m_dsbGlyphScale     ->setValue( master->getGlyphScale() );
		m_dsbGlyphSharpness ->setValue( master->getGlyphSharpness() );
		m_spbGlyphResolution->setValue( master->getGlyphResolution() );
		
		// Reconnect everything
		connect( m_dsbGlyphScale     , SIGNAL(valueChanged(double)), this, SLOT(updateMaster()) );
		connect( m_dsbGlyphSharpness , SIGNAL(valueChanged(double)), this, SLOT(updateMaster()) );
		connect( m_spbGlyphResolution, SIGNAL(valueChanged(int   )), this, SLOT(updateMaster()) );
	}
}

void TensorfieldObjectWidget::updateMaster()
{
	if( !m_master ) return; // Sanity		

	m_master->setGlyphScale     ( m_dsbGlyphScale     ->value() );
	m_master->setGlyphSharpness ( m_dsbGlyphSharpness ->value() );
	m_master->setGlyphResolution( m_spbGlyphResolution->value() );
	m_master->updateTensorfield();
}
