#ifndef MAPPERWIDGET_H
#define MAPPERWIDGET_H

#include <QWidget>

// Qt forwards
class QStandardItemModel;
class QTableView;
class QItemSelection;
class QStandardItem;
// Custom forwards
class ComboBoxDelegate;
class RenderSet;
class ModuleManager;

/**
	\class MapperWidget
	
	User interface to map modules to render areas.
*/
class MapperWidget : public QWidget
{
Q_OBJECT

signals:
	void areaNameChanged( int idx );
	
public:
	MapperWidget( QWidget* parent=0 );

	void setRenderSet( RenderSet* rs );
	RenderSet* getRenderSet() { return m_renderSet; }

	void setModuleManager( ModuleManager* mm );

	int getActiveIndex() { return m_activeRow; }
	
public slots:
	void updateTable();

protected slots:
	void onSelectionChanged( const QItemSelection&,const QItemSelection& );
	void onItemChanged( QStandardItem* item );

private:
	RenderSet*             m_renderSet;
	ModuleManager*         m_moduleManager;
	QStandardItemModel*    m_model;
	QTableView*            m_tableView;
	ComboBoxDelegate*      m_delegate;
	int m_activeRow;
};

#endif // MODULEMANAGERWIDGET_H
