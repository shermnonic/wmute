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

#include "RenderSet.h"
#include "ModuleBase.h"
#include "ModuleManager.h"
#include "ProjectMe.h"


/// Helper function for NodeEditorWidget, create a module node
QNEBlock* createModuleNode( ModuleRenderer* mod, QGraphicsScene* s )
{
	QString name = QString::fromStdString( mod->getName() );
	QString type = QString::fromStdString( mod->getModuleType() );

    QNEBlock *b = new QNEBlock(0, s);
	b->setDeletable( false );
    b->addPort( name, 0, QNEPort::NamePort ); // port 0 - name
    b->addPort( type, 0, QNEPort::TypePort ); // port 1 - type

	RenderSet* rs = dynamic_cast<RenderSet*>(mod);
	if( rs )
	{ 
		// Not output ports for RenderSet node
	}
	else
		b->addOutputPort("target");

	for( int i=0; i < mod->numChannels(); i++ )
	{
		QNEPort* p;
		QString id = rs ? "area " : "channel ";
		p = b->addPort(id+QString::number(i),false);
		p->setMaxAllowedConnections( 1 ); // Inputs have to be unique (for now)
	}    

	return b;
}

int getChannel( QNEPort* p )
{
	return p->block()->inputPorts().indexOf(p) - 2;
}

QNEPort* getInputPort( QNEBlock* b, int channel )
{
	return b->inputPorts()[ channel + 2 ];
}


NodeEditorWidget::NodeEditorWidget( QWidget* parent )
: QWidget( parent ),
  m_projectMe( 0 )
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

	connect( m_nodesEditor, SIGNAL(connectionCreated(QNEConnection*)),
	         this, SLOT(onConnectionCreated(QNEConnection*)) );
	connect( m_nodesEditor, SIGNAL(connectionDeleted(QNEConnection*)),
	         this, SLOT(onConnectionDeleted(QNEConnection*)) );
	connect( s, SIGNAL(selectionChanged()),
	         this, SLOT(onSelectionChanged()) );
}

void NodeEditorWidget::setProjectMe( ProjectMe* pm )
{
	m_projectMe = pm;
	if( !pm )
	{
		// Clear node editor		
		ModuleBlockMap::iterator it = m_moduleBlockMap.begin();
		for( ; it != m_moduleBlockMap.end(); ++it )
		{
			// Delete node
			delete it.value();  it.value() = NULL;
		}
		m_moduleBlockMap.clear();

		m_nodesEditor->clear();
	}
	else
	{
		updateNodes();
	}
}

void NodeEditorWidget::updateConnections()
{
	if( !m_projectMe )
		return;

	m_nodesEditor->eraseConnections();

	typedef ProjectMe::Connections Connections;
	Connections& cons = m_projectMe->connections();
	Connections::iterator it = cons.begin();

	for( ; it != cons.end(); ++it )
	{
		ModuleManager::ModuleArray::iterator mit;
		ModuleBlockMap::iterator bit;

		QNEBlock 
			*block1 = findBlock( it->source().module ),
			*block2 = findBlock( it->destination().module );
		if( !block1 || !block2 )
		{
			continue;
		}

		QNEPort
			*port1 = block1->outputPorts()[0], // unique output "target"
			*port2 = getInputPort( block2, it->destination().channel );

		QNEConnection* con = new QNEConnection(0, m_graphicsView->scene());
		con->setPort1( port1 );
		con->setPort2( port2 );
		con->updatePosFromPorts();
		con->updatePath();
	}
}

void NodeEditorWidget::updateNodes( ModuleManager::ModuleArray& m )
{
	ModuleManager::ModuleArray::iterator mit;
	ModuleBlockMap::iterator bit;

	// Add new modules, update existing ones
	mit = m.begin();
	for( ; mit != m.end(); ++mit )
	{
		ModuleRenderer* mr = *mit;

		if( !mr ) continue;

		bit = m_moduleBlockMap.find( mr );
		if( bit == m_moduleBlockMap.end() )
		{
			// Add new node
			QNEBlock* b = createModuleNode( mr, m_graphicsView->scene() );

			// Position node
			ModuleRenderer::Position pos = mr->position();			
			b->setPos( b->mapFromScene( pos.x, pos.y ) ); // was: b->setPos( pos.x, pos.y );

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

			// WORKAROUND: Render set node may have increase its (input) channels
			while( mr->numChannels() > (b->inputPorts().size()-2) ) // Subtract name and type ports!
				b->addInputPort(QString("area ")+QString::number(b->inputPorts().size()-2));

			// Update ModuleRenderer data (just position for now)
			mr->setPosition( ModuleRenderer::Position(b->scenePos().x(),b->scenePos().y()) );
		}
	}

	// Remove nodes for missing modules
	bit = m_moduleBlockMap.begin();
	for( ; bit != m_moduleBlockMap.end(); ++bit )
	{
		ModuleRenderer* mr = bit.key();

		if( !mr ) continue;

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

void NodeEditorWidget::updateNodes()
{
	if( !m_projectMe ) return;

	// Treat classical modules
	ModuleManager::ModuleArray ma = m_projectMe->moduleManager().modules();

	// WORKAROUND: Treat render set as additional module
	ModuleRenderer* rs = static_cast<ModuleRenderer*>(
		m_projectMe->renderSetManager().getActiveRenderSet() );
	ma.push_back( rs );
	
	updateNodes( ma );
}

ModuleRenderer* NodeEditorWidget::findModule( QNEPort* p )
{
	// Find matching modules in our map
	return findModule( p->block() );
}

ModuleRenderer* NodeEditorWidget::findModule( QNEBlock* b )
{
	// Find matching modules in our map
	ModuleRenderer *m = NULL;
	ModuleBlockMap::iterator it = m_moduleBlockMap.begin();
	for( ; it != m_moduleBlockMap.end(); ++it )
		if( !m && it.value() == b ) m = it.key();
	return m;
}

QNEBlock* NodeEditorWidget::findBlock( ModuleRenderer* mr )
{
#if 1
	if( m_moduleBlockMap.count( mr ) )
		return m_moduleBlockMap[mr];
	return NULL;
#else
	ModuleBlockMap::iterator it = m_moduleBlockMap.begin();
	for( ; it != m_moduleBlockMap.end(); ++it )
		if( it.key()==mr ) return it.value();
	return NULL;
#endif
}

void NodeEditorWidget::onConnectionCreated( QNEConnection* con )
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
	int ch = p2->block()->inputPorts().indexOf( p2 ) - 2;
	if( m_projectMe )
		m_projectMe->addConnection( m1, m2, ch );
	else
		m2->setChannel( ch, m1->target() );

	emit connectionChanged();
}

void NodeEditorWidget::onConnectionDeleted( QNEConnection* con )
{
	// See onConnectionChanged() for comments
	QNEPort  *p1 = con->port1(),
	         *p2 = con->port2();	
	
	ModuleRenderer *m1 = findModule(p1),
	               *m2 = findModule(p2);

	int ch = p2->block()->inputPorts().indexOf( p2 ) - 2;
	if( m_projectMe )
		m_projectMe->delConnection( m1, m2, ch );
	else
		m2->setChannel( ch, -1 ); // Invalidate connection via -1

	emit connectionChanged();
}

void NodeEditorWidget::onSelectionChanged()
{
	QList<QGraphicsItem*> sel = m_graphicsView->scene()->selectedItems();	

	if( sel.empty() ) // Maybe we want to add a selectionCleared() signal later?
		return;

	// Does not support multi-selections!
	if( sel.size() > 1 )
		return;

	// Find selected Block and corresponding ModuleRenderer*
	QNEBlock* b = dynamic_cast<QNEBlock*>( sel[0] );
	if( b )
	{
		ModuleRenderer* m = findModule( b );
		if( m )
		{
			emit selectionChanged( m );
		}
	}
}
