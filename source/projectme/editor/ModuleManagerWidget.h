#ifndef MODULEMANAGERWIDGET_H
#define MODULEMANAGERWIDGET_H

#include <QWidget>

// Qt forwards
class QStandardItemModel;
class QTableView;
class QItemSelection;
class QStandardItem;
// Custom forwards
class ModuleManager;

/**
	\class ModuleManagerWidget
	
	User interface for \a ModuleManager.
*/
class ModuleManagerWidget : public QWidget
{
Q_OBJECT

signals:
	void moduleNameChanged( int idx );
	
public:
	ModuleManagerWidget( QWidget* parent=0 );

	void setModuleManager( ModuleManager* mm );
	ModuleManager* getModuleManager() { return m_master; }

	int getActiveModuleIndex() { return m_activeRow; }
	
public slots:
	void updateModuleTable();

protected slots:
	void onSelectionChanged(const QItemSelection&,const QItemSelection&);
	void onItemChanged( QStandardItem* );

private:
	ModuleManager*         m_master;
	QStandardItemModel*    m_model;
	QTableView*            m_tableView;
	int m_activeRow;
};

#endif // MODULEMANAGERWIDGET_H
