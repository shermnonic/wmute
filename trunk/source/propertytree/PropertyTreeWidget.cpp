#include "PropertyTreeWidget.h"
#include "ParameterTypes.h"

#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QVBoxLayout>
#include <QStringList>

PropertyTreeWidget
  ::PropertyTreeWidget( QWidget* parent )
  : QWidget(parent)
{
	m_model = new QStandardItemModel;
	setParameters( NULL );

	QTreeView* view = new QTreeView;
	view->setModel( m_model );

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget( view );
	setLayout( layout );
}

void PropertyTreeWidget
  ::setParameters( ParameterList* params )
{
	m_model->clear();
	m_model->setHorizontalHeaderLabels( 
		QStringList() << "Property" << "Value" << "Type" );

	m_params = params;

	if( !params )
		return;

	ParameterList::iterator it = params->begin();

	for( int row=0; it != params->end(); ++it, row++ )
	{		
		QStandardItem *itemName, *itemValue, *itemType;
		
		QString key  = QString::fromStdString( (*it)->key() );
		QString type = QString::fromStdString( (*it)->type() );		

		itemName  = new QStandardItem( tr("%1").arg( key  ));
		itemValue = getParameterValueItem( (*it) );
		itemType  = new QStandardItem( tr("<%1>").arg( type ));

		m_model->setItem( row, 0, itemName );
		m_model->setItem( row, 1, itemValue );
		m_model->setItem( row, 2, itemType );
	}

	update();
}

QStandardItem* PropertyTreeWidget::
	getParameterValueItem( ParameterBase* p )
{
	QStandardItem* item;

	DoubleParameter* p_double = dynamic_cast<DoubleParameter*>( p );
	if( p_double )
	{
		item = new QStandardItem( tr("%1").arg(p_double->value()) );
		return item;
	}

	IntParameter* p_int = dynamic_cast<IntParameter*>( p );
	if( p_int )
	{
		item = new QStandardItem( tr("%1").arg(p_int->value()) );
		return item;
	}

	StringParameter* p_string = dynamic_cast<StringParameter*>( p );
	if( p_string )
	{
		item = new QStandardItem( tr("%1").arg( QString::fromStdString(p_string->value())) );
		return item;
	}

	item = new QStandardItem( tr("(unsupported type)") );
	return item;
}
