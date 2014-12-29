#ifndef NODEEDITORWIDGET_H
#define NODEEDITORWIDGET_H

#include <QWidget>
#include <QMap>

class QGraphicsView;

class QNodesEditor;
class QNEPort;
class QNEBlock;
class QNEConnection;

class ModuleManager;
class RenderSet;
class ModuleRenderer;


class NodeEditorWidget : public QWidget
{
	Q_OBJECT
	
public:
	NodeEditorWidget( QWidget* parent=0 );

	void setModuleManager( ModuleManager* mm );
	void setRenderSet( RenderSet* rs );	

public slots:
	void updateNodes();

protected slots:
	void onConnectionChanged( QNEConnection* );
	void onConnectionDeleted( QNEConnection* );

protected:
	ModuleRenderer* findModule( QNEPort* p );

private:
	QGraphicsView* m_graphicsView;
	QNodesEditor*  m_nodesEditor;
	ModuleManager* m_moduleManager;
	RenderSet*     m_renderSet;

	typedef QMap<ModuleRenderer*,QNEBlock*> ModuleBlockMap;
	ModuleBlockMap m_moduleBlockMap;
};

#endif // NODEEDITORWIDGET_H
