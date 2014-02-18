#include "MultiSliderWidget.h"
#include <QSignalMapper>
#include <QSlider>
#include <QVBoxLayout>

MultiSliderWidget::MultiSliderWidget( QWidget* parent)
: QWidget(parent)
{
	m_sigmap = new QSignalMapper( this );

	m_layout = new QVBoxLayout();
	setLayout( m_layout );

	createSliders( 100 );
	setNumberOfSliders( 0 );
}

void MultiSliderWidget::setNumberOfSliders( int num )
{
	m_numActiveSliders = std::min( num, m_sliders.size() );

	for( int i=0; i < m_sliders.size(); i++ )
	{
		bool active = i < m_numActiveSliders;
		m_sliders.at(i)->setVisible( active );
		m_sliders.at(i)->setEnabled( active );
	}
}

void MultiSliderWidget::createSliders( int num )
{
	// Remove previous sliders from layout and signal map
	for( int i=0; i < m_sliders.size(); i++ )
	{
		QSlider* slider = m_sliders.at(i);
		m_layout->removeWidget( slider );
		m_sigmap->removeMappings( slider );
		delete slider;
	}

	// Destroy previous sliders
	m_sliders.clear();

	// Create new sliders
	for( int i=0; i < num; i++ )
	{
		QSlider* slider = new QSlider( Qt::Horizontal );
		connect( slider, SIGNAL(valueChanged(int)), m_sigmap, SLOT(map()) );
		m_sigmap->setMapping( slider, i );
		
		m_sliders.push_back( slider );
	}	

	// Add sliders to layout
	for( int i=0; i < num; i++ )
		m_layout->addWidget( m_sliders.at(i) );

	connect( m_sigmap, SIGNAL(mapped(int)), this, SLOT(onValueChange(int)) );
}

void MultiSliderWidget::onValueChange( int slider_idx )
{
	int value = m_sliders.at(slider_idx)->value();
	emit( valueChanged( slider_idx, value ) );	
}

void MultiSliderWidget::setValue( int slider_idx, int value )
{
	if( slider_idx>=0 && slider_idx<m_sliders.size() )
	{
		m_sliders.at(slider_idx)->setValue( value );
	}
}

void MultiSliderWidget::setValues( int same_value_for_all )
{
	for( int i=0; i < m_sliders.size(); i++ )
		setValue( i, same_value_for_all );
}

void MultiSliderWidget::setValues( QVector<int> values )
{
	for( int i=0; i < values.size(); i++ )
		setValue( i, values.at(i) );
}

void MultiSliderWidget::setRange( int slider_idx, int min_range, int max_range )
{
	if( slider_idx>=0 && slider_idx<m_sliders.size() )
	{		
		m_sliders.at(slider_idx)->setRange( min_range, max_range );
	}
}

void MultiSliderWidget::setRange( int min_range, int max_range )
{
	for( int i=0; i < m_sliders.size(); i++ )
		setRange( i, min_range, max_range );
}

void MultiSliderWidget::getValuesAsDouble( std::vector<double>& values, double shift, double scale ) const
{
	values.clear();
	values.resize( m_sliders.size() );
	for( int i=0; i < m_sliders.size(); i++ )
	{
		int value = m_sliders.at(i)->value();
		values[i] = (value + shift) * scale;
	}
}
