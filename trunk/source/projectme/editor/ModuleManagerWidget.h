#ifndef MODULEMANAGERWIDGET_H
#define MODULEMANAGERWIDGET_H

#include <QWidget>

// Qt forwards
class QStandardItemModel;
class QTableView;
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
	
public slots:
	void updateModuleTable();

private:
	ModuleManager*         m_master;
	QStandardItemModel*    m_model;
	QTableView*            m_tableView;
};

#endif // MODULEMANAGERWIDGET_H
