#include "MapperWidget.h"
#include "RenderSet.h"
#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTableView>
#include <QItemSelection>
#include <QVBoxLayout>
#include <QStringList>

//----------------------------------------------------------------------------
MapperWidget::MapperWidget( QWidget* parent )
: QWidget(parent),
  m_renderSet(0),
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
void MapperWidget::setRenderSet( RenderSet* rs )
{
	// Disconnect
	disconnect( m_tableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(selectionChanged(const QItemSelection&,const QItemSelection&)) );
	
	// Update
	m_activeRow = -1; // Reset active module
	m_renderSet = rs;	
	updateTable();

	// Re-connect
	if( m_renderSet )
	{
		connect( m_tableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(selectionChanged(const QItemSelection&,const QItemSelection&)) );
	}
}

//----------------------------------------------------------------------------
void MapperWidget::updateTable()
{
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
			itemArea->setEditable( false );
			itemMod ->setEditable( false );
			
			m_model->setItem( row, 0, itemArea );
			m_model->setItem( row, 1, itemMod );
		}
	}
	
	// Resize table view
	m_tableView->resizeColumnsToContents();
	m_tableView->resizeRowsToContents();
}

//----------------------------------------------------------------------------
void MapperWidget::selectionChanged( const QItemSelection& selected, const QItemSelection& deselected )
{
	if( !m_renderSet || selected.isEmpty() || selected.first().indexes().isEmpty() )
		return;

	QModelIndex mi = selected.first().indexes().first();
	
	if( mi.row() >=0 && mi.row() < m_renderSet->areas().size() )
	{
		m_activeRow = mi.row();
	}
}
