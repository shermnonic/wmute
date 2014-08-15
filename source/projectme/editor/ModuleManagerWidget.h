#ifndef MODULEMANAGERWIDGET_H
#define MODULEMANAGERWIDGET_H

#include <QWidget>

// Qt forwards
class QStandardItemModel;
class QTableView;
class QItemSelection;
// Custom forwards
class ModuleManager;

/**
	\class ModuleManagerWidget
	
	User interface for \a ModuleManager.
*/
class ModuleManagerWidget : public QWidget
{
Q_OBJECT
	
public:
	ModuleManagerWidget( QWidget* parent=0 );

	void setModuleManager( ModuleManager* mm );
	ModuleManager* getModuleManager() { return m_master; }

	int getActiveModuleIndex() { return m_activeRow; }
	
public slots:
	void updateModuleTable();

protected slots:
	void selectionChanged(const QItemSelection&,const QItemSelection&);

private:
	ModuleManager*         m_master;
	QStandardItemModel*    m_model;
	QTableView*            m_tableView;
	int m_activeRow;
};

#endif // MODULEMANAGERWIDGET_H
