#ifndef MAPPERWIDGET_H
#define MAPPERWIDGET_H

#include <QWidget>

// Qt forwards
class QStandardItemModel;
class QTableView;
class QItemSelection;
// Custom forwards
class RenderSet;

/**
	\class MapperWidget
	
	User interface to map modules to render areas.
*/
class MapperWidget : public QWidget
{
Q_OBJECT
	
public:
	MapperWidget( QWidget* parent=0 );

	void setRenderSet( RenderSet* rs );
	RenderSet* getRenderSet() { return m_renderSet; }

	int getActiveIndex() { return m_activeRow; }
	
public slots:
	void updateTable();

protected slots:
	void selectionChanged(const QItemSelection&,const QItemSelection&);

private:
	RenderSet*             m_renderSet;
	QStandardItemModel*    m_model;
	QTableView*            m_tableView;
	int m_activeRow;
};

#endif // MODULEMANAGERWIDGET_H
