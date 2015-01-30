#include "ModuleParameterWidget.h"
#include "ModuleBase.h"
#include "Parameter.h"
#include "QAutoGUI.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>

ModuleParameterWidget
  ::ModuleParameterWidget( QWidget* parent )
  : QWidget(parent)
{
	m_parameters = new QAutoGUI::Parameters(this);
	m_options    = new QAutoGUI::Parameters(this);
	m_paramWidget = new QAutoGUI::ParametersWidget();
	m_optsWidget  = new QAutoGUI::ParametersWidget();

	QPushButton* optsApplyBut = new QPushButton(tr("Apply Options"));
	
	QVBoxLayout* paramLayout = new QVBoxLayout;
	paramLayout->addWidget( m_paramWidget );
	paramLayout->setContentsMargins( 0,0,0,0 );

	QVBoxLayout* optsLayout = new QVBoxLayout;
	optsLayout->addWidget( m_optsWidget );
	optsLayout->addWidget( optsApplyBut );
	optsLayout->setContentsMargins( 0,0,0,0 );

	QGroupBox* paramGroup = new QGroupBox(tr("Live parameters"));
	QGroupBox* optsGroup  = new QGroupBox(tr("Setup options"));	
	paramGroup->setLayout( paramLayout );
	optsGroup->setLayout( optsLayout );

	QVBoxLayout* l = new QVBoxLayout();
	l->addWidget( paramGroup );
	l->addWidget( optsGroup );
	l->addStretch( 1 );
	l->setContentsMargins( 0,0,0,0 );
	setLayout( l );

	connect( optsApplyBut, SIGNAL(clicked()), this, SLOT(onApplyOptions()) );
}

void ModuleParameterWidget
  ::onApplyOptions()
{
	if( m_master )
	{
		m_master->applyOptions();
	}
}

void ModuleParameterWidget
  ::setupParameters( ParameterList& plist, QAutoGUI::Parameters* params, QAutoGUI::ParametersWidget* widget )
{
	assert( params );
	assert( widget );

	using QAutoGUI::DoubleLineEdit;
	using QAutoGUI::DoubleSpinEdit;
	using QAutoGUI::BooleanCheckbox;
	using QAutoGUI::IntegerSpinEdit;
	using QAutoGUI::BooleanIntCheckbox;
	using QAutoGUI::EnumCombobox;

	QAutoGUI::Parameters& p = *params;
	
	// Create parameters
	ParameterList::iterator it = plist.begin();
	for( ; it != plist.end(); ++it )
	{
		DoubleParameter* pdouble = dynamic_cast<DoubleParameter*>(*it);
		IntParameter*    pint    = dynamic_cast<IntParameter*   >(*it);
		BoolParameter*   pbool   = dynamic_cast<BoolParameter*  >(*it);
		EnumParameter*   penum   = dynamic_cast<EnumParameter*  >(*it);
		StringParameter* pstring = dynamic_cast<StringParameter*>(*it);	

		QString name = QString::fromStdString( (*it)->key() );
				
		if( pdouble )
		{			
			DoubleSpinEdit& dse = DoubleSpinEdit::New( pdouble->valueRef(), name );				
			if( pdouble->limits().active ) 
			{
				double 
					min_ = pdouble->limits().min_,
					max_ = pdouble->limits().max_;

				dse.setRange( min_, max_ );
				dse.setSingleStep( abs(max_-min_)/100.0 );
			}
			else
				dse.setSingleStep( 0.1 ); // Default single step
			p << dse.setDecimals( 2 ); // FIXME: Hardcoded #decimals
					//.connect_to( this, SLOT(parameterChanged()) );
			
			// Alternatively we could use a line edit:
			//p << DoubleLineEdit::New( pdouble->valueRef(), name );
		}
		else
		if( pbool ) // handle bool specialization before int
		{
			p << BooleanIntCheckbox::New( pbool->valueRef(), name );
					//.connect_to( this, SLOT(parameterChanged()) );
		}
		else
		if( penum ) // handle enum specialization before int
		{
			// Enum names
			QStringList items;
			for( int i=0; i < penum->enumNames().size(); i++ )
				items.append( QString::fromStdString( penum->enumNames()[i] ) );

			// WORKAROUND: Somehow the value is set to zero when creating a 
			//             EnumCombobox. Therefore we store the value in 
			//             advance, reset it afterwards and call update_gui().
			int val = penum->value();
			EnumCombobox& ecb = EnumCombobox::New( penum->valueRef(), name )
				.setItems( items );
			p << ecb;
			penum->setValue( val );
			ecb.update_gui();
		}
		else
		if( pint ) // handle int after its specializations bool and enum
		{
			IntegerSpinEdit& ise = IntegerSpinEdit::New( pint->valueRef(), name );
			if( pint->limits().active )
				ise.setRange( pint->limits().min_, pint->limits().max_ );
			p << ise;				
					//.connect_to( this, SLOT(parameterChanged()) );
		}
		else
		if( pstring )
		{
			qDebug() << "ModuleParameterWidget::setModule : "
				"String parameters not supported yet!";
		}
	}
	
	// Update gui
	widget->load_parameters( p );
}

void ModuleParameterWidget
  ::clear()
{
	m_paramWidget->clear();
	m_optsWidget->clear();
	m_parameters->clear();
	m_options->clear();
}

void ModuleParameterWidget
  ::setModule( ModuleBase* master )
{
	clear();

	m_master = master;
	if( !master ) 
	{
		return;
	}

	setupParameters( master->parameters(), m_parameters, m_paramWidget );
	setupParameters( master->options(), m_options, m_optsWidget );
}
