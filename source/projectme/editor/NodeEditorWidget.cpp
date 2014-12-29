#include "NodeEditorWidget.h"
#include "qneblock.h"
#include "qnodeseditor.h"
#include "qneconnection.h"
#include "qneport.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QDebug>

#include <algorithm> // for find()

#include "RenderSet.h" // for RenderSet, ModuleManager, ModuleBase


/// Helper function for NodeEditorWidget, create a module node
QNEBlock* createModuleNode( ModuleRenderer* mod, QGraphicsScene* s )
{
	QString name = QString::fromStdString( mod->getName() );
	QString type = QString::fromStdString( mod->getModuleType() );

    QNEBlock *b = new QNEBlock(0, s);
	b->setDeletable( false );
    b->addPort( name, 0, QNEPort::NamePort ); // port 0 - name
    b->addPort( type, 0, QNEPort::TypePort ); // port 1 - type

	for( int i=0; i < mod->numChannels(); i++ )
	{
		QNEPort* p;
		p = b->addPort(QString("channel ")+QString::number(i),false);
		p->setMaxAllowedConnections( 1 ); // Inputs have to be unique (for now)
	}
    
	b->addOutputPort("target");

	return b;
}


NodeEditorWidget::NodeEditorWidget( QWidget* parent )
: QWidget( parent ),
  m_moduleManager( 0 ),
  m_renderSet( 0 )
{
	m_graphicsView = new QGraphicsView();
	
	QGraphicsScene *s = new QGraphicsScene();	
	m_graphicsView->setScene( s );
	m_graphicsView->setRenderHint( QPainter::Antialiasing );
	//m_graphicsView->setDragMode( QGraphicsView::ScrollHandDrag );
	
	m_nodesEditor = new QNodesEditor();
	m_nodesEditor->install( s );
	
	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget( m_graphicsView );
	layout->setContentsMargins( 0,0,0,0 );
	setLayout( layout );

	connect( m_nodesEditor, SIGNAL(connectionChanged(QNEConnection*)),
	         this, SLOT(onConnectionChanged(QNEConnection*)) );
	connect( m_nodesEditor, SIGNAL(connectionDeleted(QNEConnection*)),
	         this, SLOT(onConnectionDeleted(QNEConnection*)) );
}

void NodeEditorWidget::setModuleManager( ModuleManager* mm )
{
	m_moduleManager = mm;
	if( !mm )
	{
		// Clear node editor		
		ModuleBlockMap::iterator it = m_moduleBlockMap.begin();
		for( ; it != m_moduleBlockMap.end(); ++it )
		{
			// Delete node
			delete it.value();  it.value() = NULL;
		}
		m_moduleBlockMap.clear();
	}
	else
	{
		updateNodes();
	}
}

void NodeEditorWidget::setRenderSet( RenderSet* rs )
{
	m_renderSet = rs;
}

void NodeEditorWidget::updateNodes()
{
	if( !m_moduleManager ) return;

	ModuleManager::ModuleArray::iterator mit;
	ModuleBlockMap::iterator bit;

	ModuleManager::ModuleArray& m = m_moduleManager->modules();

	// Add new modules, update existing ones
	mit = m.begin();
	for( ; mit != m.end(); ++mit )
	{
		ModuleRenderer* mr = *mit;
		bit = m_moduleBlockMap.find( mr );
		if( bit == m_moduleBlockMap.end() )
		{
			// Add new node
			QNEBlock* b = createModuleNode( mr, m_graphicsView->scene() );

			// Insert new entry into QMap
			m_moduleBlockMap.insert( mr, b );
		}
		else
		{
			// Update existing node (no connections but only name/type yet)
			QNEBlock* b = bit.value();
			QString name = QString::fromStdString( mr->getName() );
			QString type = QString::fromStdString( mr->getModuleType() ); 
			b->ports()[0]->setName( name ); // port 0 - name
			b->ports()[1]->setName( type ); // port 1 - type
		}
	}

	// Remove nodes for missing modules
	bit = m_moduleBlockMap.begin();
	for( ; bit != m_moduleBlockMap.end(); ++bit )
	{
		ModuleRenderer* mr = bit.key();
		mit = std::find( m.begin(), m.end(), mr );
		if( mit == m.end() )
		{
			// Module is missing, delete node
			delete bit.value(); bit.value() = NULL;

			// Erase this entry from QMap
			bit = m_moduleBlockMap.erase( bit );
		}
	}	
}

ModuleRenderer* NodeEditorWidget::findModule( QNEPort* p )
{
	// Find matching modules in our map
	QNEBlock *b = p->block();
	ModuleRenderer *m = NULL;
	ModuleBlockMap::iterator it = m_moduleBlockMap.begin();
	for( ; it != m_moduleBlockMap.end(); ++it )
		if( !m && it.value() == b ) m = it.key();
	return m;
}

void NodeEditorWidget::onConnectionChanged( QNEConnection* con )
{
	// Ports (with corresponding blocks) are given
	QNEPort  *p1 = con->port1(),
	         *p2 = con->port2();	
	
	// Find matching modules in our map
	ModuleRenderer *m1 = findModule(p1),
	               *m2 = findModule(p2);
	// Sanity
	if( !m1 || !m2 )
	{
		qDebug() << "NodeEditorWidget::connectionChanged() : "
			<< "Mismatch in module to node mapping!";
		return;
	}

	// Hardcoded channel index mapping (only for input/destination port)
	// Port p1 must be the single output port ("target") of a ModuleRenderer
	int ch = p2->block()->ports().indexOf( p2 ) - 2;
	m2->setChannel( ch, m1->target() );
}

void NodeEditorWidget::onConnectionDeleted( QNEConnection* con )
{
	// See onConnectionChanged() for comments
	QNEPort  *p1 = con->port1(),
	         *p2 = con->port2();	
	
	ModuleRenderer *m1 = findModule(p1),
	               *m2 = findModule(p2);

	int ch = p2->block()->ports().indexOf( p2 ) - 2;
	m2->setChannel( ch, -1 ); // Invalidate connection via -1
}
