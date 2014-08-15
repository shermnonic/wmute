#include "ModuleManagerWidget.h"
#include "RenderSet.h" // for ModuleManager
#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTableView>
#include <QVBoxLayout>
#include <QStringList>

//----------------------------------------------------------------------------
ModuleManagerWidget::ModuleManagerWidget( QWidget* parent )
: QWidget(parent),
  m_master(0)
{
	// Setup model and table view
	m_model = new QStandardItemModel;
	m_tableView = new QTableView;
	m_tableView->setModel( m_model );
	
	// Single row selection
	m_tableView->setSelectionBehavior( QAbstractItemView::SelectRows );
	m_tableView->setSelectionMode( QAbstractItemView::SingleSelection );	
	
	// Layout
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget( m_tableView );	
	layout->setContentsMargins( 0,0,0,0 );
	setLayout( layout );	
}

//----------------------------------------------------------------------------
void ModuleManagerWidget::setModuleManager( ModuleManager* mm )
{
	m_master = mm;
	updateModuleTable();
}

//----------------------------------------------------------------------------
void ModuleManagerWidget::updateModuleTable()
{
	// Clear table
	m_model->clear();
	m_model->setHorizontalHeaderLabels( 
		QStringList() << tr("Type") << tr("Name") );
	
	// Fill table with module info
	if( m_master )
	{
		const ModuleManager::ModuleArray& modules = m_master->modules();
		m_model->setRowCount( (int)modules.size() );
		
		for( int row=0; row < modules.size(); row++ )
		{
			QString type(tr("(Invalid module pointer)")), name;
			if( modules.at(row) )
			{
				type = QString::fromStdString( modules.at(row)->getModuleType() );
				name = QString::fromStdString( modules.at(row)->getName() );
			}
			
			QStandardItem 
				*itemType = new QStandardItem( type ),
				*itemName = new QStandardItem( name );
			itemType->setEditable( false );
			itemName->setEditable( true );
			
			m_model->setItem( row, 0, itemType );
			m_model->setItem( row, 1, itemName );
		}
	}
	
	// Resize table view
	m_tableView->resizeColumnsToContents();
	m_tableView->resizeRowsToContents();
}
