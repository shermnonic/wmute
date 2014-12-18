#include "ModuleManagerWidget.h"
#include "RenderSet.h" // for ModuleManager
#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTableView>
#include <QItemSelection>
#include <QVBoxLayout>
#include <QStringList>

//----------------------------------------------------------------------------
ModuleManagerWidget::ModuleManagerWidget( QWidget* parent )
: QWidget(parent),
  m_master(0),
  m_activeRow(-1)
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
	// Update
	m_activeRow = -1; // Reset active module
	m_master = mm;	
	updateModuleTable();
}

//----------------------------------------------------------------------------
void ModuleManagerWidget::onItemChanged( QStandardItem* item )
{
	// Debug
	QModelIndex mi = m_model->indexFromItem( item );
	qDebug() << "ModuleManagerWidget::onItemChanged() : " 
		<< "(" << mi.row() << "," << mi.column() << ") "
		<< item->text();

	// Changes can only be applied if master is connected
	if( !m_master )
		return;	

	// Changed name in column 1
	if( mi.column() == 1 )
	{
		int idx = mi.row(); // Rows and indices match 1-1
		if( idx >= 0 && idx < m_master->modules().size() )
		{
			m_master->modules().at(idx)->setName( item->text().toStdString() );
			
			// Emit signals
			emit moduleNameChanged( idx );
		}
		else
		{
			qDebug() << "ModuleManagerWidget::onItemChanged() : "
				<< "Unexpected module index " << idx << "!";
		}
	}
}

//----------------------------------------------------------------------------
void ModuleManagerWidget::updateModuleTable()
{
	// Disconnect
	disconnect( m_tableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(onSelectionChanged(const QItemSelection&,const QItemSelection&)) );
	disconnect( m_model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(onItemChanged(QStandardItem*)) );

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

	// Re-connect
	if( m_master )
	{
		connect( m_tableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(onSelectionChanged(const QItemSelection&,const QItemSelection&)) );
		connect( m_model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(onItemChanged(QStandardItem*)) );
	}
}

//----------------------------------------------------------------------------
void ModuleManagerWidget::onSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected )
{
	if( !m_master || selected.isEmpty() || selected.first().indexes().isEmpty() )
		return;

	QModelIndex mi = selected.first().indexes().first();
	
	const ModuleManager::ModuleArray& modules = m_master->modules();
	if( mi.row() >=0 && mi.row() < modules.size() )
	{
		m_activeRow = mi.row();
		emit moduleChanged( m_master->modules()[m_activeRow] );
	}
	else
		emit moduleChanged( NULL );
}
