#include "PropertyTreeView.h"
#include <QMouseEvent>

PropertyTreeView::PropertyTreeView( QWidget* parent )
	: QTreeView(parent),
	  m_oneClickEditing(true)
{
	setOneClickEditing( m_oneClickEditing );
}

void PropertyTreeView::
  setOneClickEditing( bool enable )
{
	if( enable )
	{
		// Enable workaround to accomplish one-click editing
		setEditTriggers( QAbstractItemView::SelectedClicked );
	}
	else
	{
		// Disable one-click editing
		setEditTriggers( QAbstractItemView::DoubleClicked );
	}
	m_oneClickEditing = enable;
}

void PropertyTreeView::
  mousePressEvent( QMouseEvent* e )
{
	if( m_oneClickEditing )
	{
		// One-click editing workaround
		if( e->button() == Qt::LeftButton ) 
		{
			QModelIndex index = indexAt( e->pos() );
			if( index.column() == 1 )  // column you want to use for one click
				edit(index);
		}
	}
	QTreeView::mousePressEvent( e );
}
