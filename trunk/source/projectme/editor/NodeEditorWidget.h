#ifndef NODEEDITORWIDGET_H
#define NODEEDITORWIDGET_H

#include <QWidget>
#include <QMap>

class QGraphicsView;

class QNodesEditor;
class QNEPort;
class QNEBlock;
class QNEConnection;

#include "ModuleManager.h"
class ProjectMe;
class ModuleRenderer;

/**
	\class NodeEditorWidget

	Simple node editor to connect ModuleRenderer instances.

	Based on QNodesEditor from Stanislaw Adaszewski.
*/
class NodeEditorWidget : public QWidget
{
	Q_OBJECT

signals:
	void connectionChanged();
	void selectionChanged( ModuleRenderer* );
	
public:
	NodeEditorWidget( QWidget* parent=0 );

	void setProjectMe( ProjectMe* pm );

public slots:
	void updateNodes();
	void updateConnections();

protected slots:
	void onConnectionCreated( QNEConnection* );
	void onConnectionDeleted( QNEConnection* );
	void onSelectionChanged();

protected:
	ModuleRenderer* findModule( QNEBlock* b );
	ModuleRenderer* findModule( QNEPort* p );
	QNEBlock* findBlock( ModuleRenderer* mr );
	void updateNodes( ModuleManager::ModuleArray& ma );

private:
	QGraphicsView* m_graphicsView;
	QNodesEditor*  m_nodesEditor;
	ProjectMe*     m_projectMe;

	typedef QMap<ModuleRenderer*,QNEBlock*> ModuleBlockMap;
	ModuleBlockMap m_moduleBlockMap;
};

#endif // NODEEDITORWIDGET_H
