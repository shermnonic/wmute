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
		b->addInputPort(QString("channel ")+QString::number(i));
    
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

void NodeEditorWidget::onConnectionChanged( QNEConnection* con )
{
	// Ports and corresponding blocks are given
	QNEPort  *p1 = con->port1(),
	         *p2 = con->port2();
	
	QNEBlock *b1 = p1->block(),
	         *b2 = p2->block();

	// Find matching modules in our map
	ModuleRenderer *m1 = NULL,
	               *m2 = NULL;
	ModuleBlockMap::iterator it = m_moduleBlockMap.begin();
	for( ; it != m_moduleBlockMap.end(); ++it )
	{
		if( !m1 && it.value() == b1 ) m1 = it.key();
		if( !m2 && it.value() == b2 ) m2 = it.key();
	}

	// Sanity
	if( !m1 || !m2 )
	{
		qDebug() << "NodeEditorWidget::connectionChanged() : "
			<< "Mismatch in module to node mapping!";
		return;
	}

	// Hardcoded channel index mapping (only for input/destination port)
	// Port p1 must be the single output port ("target") of a ModuleRenderer
	int ch = b2->ports().indexOf( p2 ) - 2;
	m2->setChannel( ch, m1->target() );
}
