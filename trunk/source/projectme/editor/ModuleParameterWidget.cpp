#include "ModuleParameterWidget.h"
#include "RenderSet.h" // for ModuleBase
#include "Parameter.h"
#include "QAutoGUI.h"

#include <QDebug>
#include <QVBoxLayout>

ModuleParameterWidget
  ::ModuleParameterWidget( QWidget* parent )
  : QWidget(parent)
{
	m_parameters = new QAutoGUI::Parameters(this);
	m_widget = new QAutoGUI::ParametersWidget();
	QVBoxLayout* l = new QVBoxLayout();
	l->addWidget( m_widget );
	l->setContentsMargins( 0,0,0,0 );
	setLayout( l );
}

void ModuleParameterWidget
  ::setModule( ModuleBase* master )
{
	using QAutoGUI::DoubleLineEdit;
	using QAutoGUI::DoubleSpinEdit;
	using QAutoGUI::BooleanCheckbox;
	using QAutoGUI::IntegerSpinEdit;
	using QAutoGUI::BooleanIntCheckbox;

	m_widget->clear();
	m_parameters->clear();
	m_master = master;	
	QAutoGUI::Parameters& p = *m_parameters;
	if( !master ) 
	{
		return;
	}
	
	// Create parameters
	ParameterList& plist = master->parameters();	
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
			qDebug() << "ModuleParameterWidget::setModule : "
				"Enum parameters not supported yet!";
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
	m_widget->load_parameters( p );
}
