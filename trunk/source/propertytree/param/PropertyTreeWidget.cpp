#include "PropertyTreeWidget.h"
#include "PropertyTreeDelegate.h"
#include "PropertyTreeView.h"
#include "ParameterTypes.h"

#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QVBoxLayout>
#include <QStringList>
#include <QDebug>

#include <cassert>

PropertyTreeWidget
  ::PropertyTreeWidget( QWidget* parent )
  : QWidget(parent)
{
	m_model = new QStandardItemModel;
	setParameters( NULL );

	m_view = new PropertyTreeView;
	m_view->setModel( m_model );	
	m_view->setOneClickEditing( false );

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget( m_view );
	setLayout( layout );

	connect( m_model, SIGNAL(itemChanged(QStandardItem*)), 
	         this, SLOT(onItemChanged(QStandardItem*)) );
}

void PropertyTreeWidget
  ::onItemChanged( QStandardItem* item )
{
	// Sanity
	if( !m_params )
	{
		qDebug() << "PropertyTreeWidget::onItemChanged : "
			"Missing parameter list!";
		return;
	}

	// Assume 1-1 index to item correspondence
	int idx = item->index().row();

	// Sanity
	if( idx<0 || idx>=m_params->size() )
	{
		qDebug() << "PropertyTreeWidget::onItemChanged : "
			"Mismatching index!";
		return;
	}

	// Emit parameterChanged() signal
	ParameterBase* p = m_params->at(idx);
	assert( p );
	emit parameterChanged( p );
}

void PropertyTreeWidget
  ::setParameters( ParameterList* params )
{
	// Reset model
	m_model->clear();
	m_model->setHorizontalHeaderLabels( 
		QStringList() << "Property" << "Value" << "Type" );
	
	m_params = params;

	// If called with NULL, simply return
	if( !params )
		return;

	// Fill model with parameters
	ParameterList::iterator it = params->begin();
	for( int row=0; it != params->end(); ++it, row++ )
	{		
		QStandardItem *itemName, *itemValue, *itemType;
		
		QString key  = QString::fromStdString( (*it)->key() );
		QString type = QString::fromStdString( (*it)->type() );		

		itemName  = new QStandardItem( tr("%1").arg( key  ));
		itemValue = getParameterValueItem( (*it) );
		itemType  = new QStandardItem( tr("<%1>").arg( type ));

		itemName->setEditable( false );
		itemType->setEditable( false );

		itemValue->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );

		m_model->setItem( row, 0, itemName );
		m_model->setItem( row, 1, itemValue );
		m_model->setItem( row, 2, itemType );
	}

	// FIXME: Will the delegate instance created here automatically be deleted?
	PropertyTreeDelegate* valueDelegate = new PropertyTreeDelegate;	
	valueDelegate->setParameters( params );
	m_view->setItemDelegateForColumn( 1, valueDelegate );

	// Customize view
	m_view->hideColumn( 2 );
	m_view->setAlternatingRowColors( true );

	// Auto resize columns
	for( int i=0; i < 4; i++ )
		m_view->resizeColumnToContents( i );
	
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

	// EnumParameter must be checked before its parent class IntParameter
	EnumParameter* p_enum = dynamic_cast<EnumParameter*>( p );
	if( p_enum )
	{
		item = new QStandardItem( 
			QString::fromStdString( p_enum->enumNames().at(p_enum->value()) ));
		return item;
	}

	// Int also covers its subclasses (not handled above), i.e. Bool for now
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
