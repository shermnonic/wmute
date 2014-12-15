#include "MapperWidget.h"
#include "RenderSet.h"
#include "ComboBoxDelegate.h"
#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTableView>
#include <QItemSelection>
#include <QVBoxLayout>
#include <QStringList>
#include <cassert>

//----------------------------------------------------------------------------
MapperWidget::MapperWidget( QWidget* parent )
: QWidget(parent),
  m_renderSet(0),
  m_moduleManager(0),
  m_activeRow(-1)
{
	// Setup model and table view
	m_model = new QStandardItemModel;
	m_tableView = new QTableView;
	m_tableView->setModel( m_model );
	
	// Single row selection
	m_tableView->setSelectionBehavior( QAbstractItemView::SelectRows );
	m_tableView->setSelectionMode( QAbstractItemView::SingleSelection );	

	// Delegate 
	m_delegate = new ComboBoxDelegate(this);
	
	// Layout
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget( m_tableView );	
	layout->setContentsMargins( 0,0,0,0 );
	setLayout( layout );	
}

//----------------------------------------------------------------------------
void MapperWidget::setRenderSet( RenderSet* rs )
{
	// Disconnect
	disconnect( m_tableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(selectionChanged(const QItemSelection&,const QItemSelection&)) );
	disconnect( m_model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(onItemChanged(QStandardItem*)) );
	
	// Update
	m_activeRow = -1; // Reset active module
	m_renderSet = rs;
	updateTable();

	// Re-connect
	if( m_renderSet )
	{
		connect( m_tableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(selectionChanged(const QItemSelection&,const QItemSelection&)) );
		connect( m_model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(onItemChanged(QStandardItem*)) );
	}
}

//----------------------------------------------------------------------------
void MapperWidget::onItemChanged( QStandardItem* item )
{
	// Debug
	QModelIndex mi = m_model->indexFromItem( item );
	qDebug() << "MapperWidget::onItemChanged() : " 
		<< "(" << mi.row() << "," << mi.column() << ") "
		<< item->text();

	// Changes can only be applied if a master RenderSet is connected
	if( !m_renderSet )
		return;	

	QString text = item->text();

	// Rows and indices match 1-1
	int idx = mi.row();
	if( idx < 0 || idx >= m_renderSet->areas().size() )
	{
		qDebug() << "MapperWidget::onItemChanged() : "
			<< "Unexpected render set area index " << idx << "!";
	}

	// Changed name in column 0
	if( mi.column() == 0 )
	{
		m_renderSet->areas().at(idx).setName( text.toStdString() );
			
		// Emit signal
		emit areaNameChanged( idx );
	}
	else
	// Changed mapping
	if( mi.column() == 1 )
	{
		// Sanity
		if( !m_moduleManager )
		{
			qDebug() << "MapperWidget::onItemChanged() : "
				<< "ModuleManager not set but required to change mapping!";
			return;
		}

		// Get pointer to newly mapped module renderer
		int moduleIdx = m_delegate->getIndex( text );
		assert( moduleIdx >= 0 && moduleIdx < m_moduleManager->modules().size() );
		ModuleRenderer* mr = m_moduleManager->modules()[moduleIdx];

		// Set mapping
		m_renderSet->mapper().at(idx) = mr;
	}
}

//----------------------------------------------------------------------------
void MapperWidget::setModuleManager( ModuleManager* mm )
{
	// Disconnect
	disconnect( m_tableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(onSelectionChanged(const QItemSelection&,const QItemSelection&)) );
	
	// Update
	m_moduleManager = mm;
	updateTable();

	// Re-connect
	if( m_renderSet )
	{
		connect( m_tableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(onSelectionChanged(const QItemSelection&,const QItemSelection&)) );
	}
}

//----------------------------------------------------------------------------
void MapperWidget::updateTable()
{
	// Set module list
	if( m_moduleManager )
	{
		// Fill combo box with module type names
		QStringList modules;
		for( int i=0; i < m_moduleManager->modules().size(); i++ )
		{
			modules.insert( i,
				QString::fromStdString(
					m_moduleManager->modules()[i]->getName() ));
		}

		// Enable combo box
		m_delegate->setItems( modules );
		m_tableView->setItemDelegateForColumn( 1, m_delegate );
	}
	else
	{
		// Disable combo box
		m_tableView->setItemDelegateForColumn( 1, NULL );
	}

	// Clear table
	m_model->clear();
	m_model->setHorizontalHeaderLabels( 
		QStringList() << tr("RenderArea") << tr("Module") );
	
	// Fill table with module info
	if( m_renderSet )
	{
		const RenderAreaCollection&  areas = m_renderSet->areas();
		
		m_model->setRowCount( (int)areas.size() );
		
		for( int row=0; row < areas.size(); row++ )
		{			
			RenderArea area = m_renderSet->areas()[row];
			const ModuleRenderer* mod = m_renderSet->mapper()[row];
			
			QStandardItem 
				*itemArea = new QStandardItem( QString::fromStdString(area.getName()) ),
				*itemMod  = new QStandardItem( mod ? QString::fromStdString(mod->getName()) : tr("(Invalid module pointer)") );
			itemArea->setEditable( true );
			itemMod ->setEditable( m_moduleManager ? true : false );
			
			m_model->setItem( row, 0, itemArea );
			m_model->setItem( row, 1, itemMod );
		}
	}
	
	// Resize table view
	m_tableView->resizeColumnsToContents();
	m_tableView->resizeRowsToContents();

#if 0
	// Make the combo boxes always displayed.
	// (See: http://programmingexamples.net/wiki/Qt/Delegates/ComboBoxDelegate)
	for( int i = 0; i < m_model->rowCount(); ++i )
	{
		m_tableView->openPersistentEditor( m_model->index(i, 1) );
	}
#endif
}

//----------------------------------------------------------------------------
void MapperWidget::onSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected )
{
	if( !m_renderSet || selected.isEmpty() || selected.first().indexes().isEmpty() )
		return;

	QModelIndex mi = selected.first().indexes().first();
	
	if( mi.row() >=0 && mi.row() < m_renderSet->areas().size() )
	{
		m_activeRow = mi.row();
	}
}
