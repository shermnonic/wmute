#include "ObjectBrowserWidget.h"
#include <QListView>
#include <QVBoxLayout>
#include <QAction>
#include <QToolBar>
#include <QDebug>

ObjectBrowserWidget::ObjectBrowserWidget( QWidget* parent )
: QWidget(parent)
{
	m_listView = new QListView();
	m_listView->setSelectionMode( QAbstractItemView::SingleSelection );
	
	QAction* m_actRemove = new QAction(tr("Remove"),this);
	QAction* m_actMoveUp = new QAction(tr("Up"),this);
	QAction* m_actMoveDown = new QAction(tr("Down"),this);
	
	QToolBar* m_toolbar = new QToolBar();
	m_toolbar->addAction( m_actRemove );
	// m_toolbar->addAction( m_actMoveUp );	
	// m_toolbar->addAction( m_actMoveDown );
	
	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget( m_listView );
	layout->addWidget( m_toolbar );
	
	this->setLayout( layout );	
	
	connect( m_actRemove, SIGNAL(triggered()), this, SLOT(removeSelectedObject()) );
}

void ObjectBrowserWidget::setModel( QAbstractItemModel* model )
{
	m_listView->setModel( model );

	connect( m_listView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
			 this, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)) );
}

int ObjectBrowserWidget::selectedObject() const
{	
	if( !m_listView->selectionModel() )
		return -1;
	if( m_listView->selectionModel()->selectedIndexes().empty() )
		return -1;
	return m_listView->selectionModel()->selectedIndexes().front().row();
}

void ObjectBrowserWidget::select( const QModelIndex& index )
{
	// Highlight selection
	m_listView->selectionModel()->select( index, QItemSelectionModel::Select );	
}

void ObjectBrowserWidget::removeSelectedObject()
{
	int idx = selectedObject();
	if( idx >= 0 )
		emit removeObject( idx );
	else
		qDebug() << "ObjectBrowserWidget::removeSelectedObject() : Invalid index!";
}
