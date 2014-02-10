// SceneViewer - GUI for scene::Scene
// Max Hermann, Jan 2014
#include "SceneViewer.h"
#include "ObjectPropertiesWidget.h"

#include <qfileinfo.h>
#include <QListView>
#include <QDebug>
#include <QProgressDialog>
#include <QApplication>
#include <QMessageBox>
#include <QMouseEvent>
#include <QAction>


SceneViewer::SceneViewer( QWidget* parent )
  : QGLViewer(parent),
    m_currentObject(-1),
	m_selectionMode(SelectNone),
	m_selectFrontFaces(true),
	m_curShader(ShaderMesh)
{
	// --- Widgets ---
	m_listView = new QListView();
	m_listView->setModel( &m_model );
	m_listView->setSelectionMode( QAbstractItemView::SingleSelection );
	m_propertiesWidget = new ObjectPropertiesWidget();
	
	connect( m_listView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), 
		     this, SLOT(selectModelItem(const QModelIndex&)) );
	connect( m_propertiesWidget, SIGNAL(sceneObjectFrameChanged()), this, SLOT(updateScene()) );

	// --- Actions ---
	// Shortcut description is also added to QGLViewer help.

	QAction* actSelectNone = new QAction(tr("Select none"),this);
	actSelectNone->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_A );
	QGLViewer::setKeyDescription( Qt::CTRL + Qt::SHIFT + Qt::Key_A, "Select none (deselects all vertices)" );

	QAction* actReloadShaders = new QAction(tr("Reload shaders"),this);
	actReloadShaders->setShortcut( Qt::CTRL + Qt::Key_R );
	QGLViewer::setKeyDescription( Qt::CTRL + Qt::Key_R, "Reload shaders" );

	QAction* actSelectFrontFaces = new QAction(tr("Select vertices only on front faces"),this);
	actSelectFrontFaces->setCheckable( true );
	actSelectFrontFaces->setChecked( m_selectFrontFaces );

	connect( actSelectNone, SIGNAL(triggered()), this, SLOT(selectNone()) );
	connect( actReloadShaders, SIGNAL(triggered()), this, SLOT(reloadShaders()) );
	connect( actSelectFrontFaces, SIGNAL(toggled(bool)), this, SLOT(selectFrontFaces(bool)) );

	m_actions.push_back( actSelectNone );
	m_actions.push_back( actSelectFrontFaces );
	m_actions.push_back( actReloadShaders );
}

void SceneViewer::showInspector()
{
	m_propertiesWidget->setWindowTitle(tr("Inspector"));
	m_propertiesWidget->show();
}

void SceneViewer::showBrowser()
{		
	m_listView->show();	
}

QString SceneViewer::helpString() const
{
	return QString(
		"<h2>meshspace</h2>"
		"Max Hermann (<a href='mailto:hermann@cs.uni-bonn.de'>hermann@cs.uni-bonn.de</a>)<br>"
		"University of Bonn, Computer Graphics Group<br>"
		"Jan. 2014");
}

//----------------------------------------------------------------------------
// Scene Model
//----------------------------------------------------------------------------

void SceneViewer::selectModelItem( const QModelIndex& current, const QModelIndex& previous )
{
	selectModelItem( current );
}

void SceneViewer::selectModelItem( const QModelIndex& index )
{
	qDebug() << "Index row = " << index.row() << ", current index row = " << m_listView->currentIndex().row();
	m_currentObject = index.row();//m_listView->currentIndex().row(); // was: index.row()
	m_propertiesWidget->setSceneObject( m_scene.objects().at( m_currentObject ).get() );
}

void SceneViewer::updateModel()
{
	// Disconnect from change signal as long as the model and scene object
	// list are out of sync during this reset of the model.
	m_model.disconnect( this );	// same as: disconnect( &m_model, 0, this, 0 );

	// Rebuild model from current scene
	m_model.clear();
	m_model.setHorizontalHeaderItem( 0, new QStandardItem("Name") );

	for( unsigned i=0; i < m_scene.objects().size(); i++ )
	{
		QString name = QString::fromStdString( m_scene.objects().at(i)->getName() );
		bool visible = m_scene.objects().at(i)->isVisible();

		QStandardItem* item = new QStandardItem( name );
		item->setCheckable( true );
		item->setCheckState( visible ? Qt::Checked : Qt::Unchecked );

		m_model.setItem( i, item );
	}

	// Auto-select the last added object
	QModelIndex selection = m_model.item( m_model.rowCount()-1 )->index();
	// Highlight selection
	m_listView->selectionModel()->select( selection, QItemSelectionModel::Select );
	// Set as current scene object
	selectModelItem( selection );

	connect( &m_model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(onModelItemChanged(QStandardItem*)) );
}

void SceneViewer::onModelItemChanged( QStandardItem* )
{
	// Sanity check
	if( m_scene.objects().size() != m_model.rowCount() )
	{
		qDebug() << "Warning: Mismatch between scene objects and list widget!";
		return;
	}

	// Simply update the whole scene
	for( unsigned i=0; i < m_scene.objects().size(); i++ )
	{
		m_scene.objects().at(i)->setVisible(
			m_model.item(i)->checkState() == Qt::Checked );
	}

	// Force redraw
	updateGL();
}

scene::MeshObject* SceneViewer::currentMeshObject()
{
	if( m_currentObject < 0 ) return NULL;
	return dynamic_cast<scene::MeshObject*>( m_scene.objects().at(m_currentObject).get() );
}

//----------------------------------------------------------------------------
// Mesh IO
//----------------------------------------------------------------------------

void SceneViewer::saveMeshBuffer( QString filename )
{
	if( m_currentObject < 0 || m_currentObject >= m_scene.objects().size() )		
	{
		QMessageBox::warning( this, tr("Error"), tr("Invalid selection!") );
		return;
	}

	scene::Object* so = m_scene.objects().at( m_currentObject ).get();
	if( dynamic_cast<scene::MeshObject*>(so) )
	{
		scene::MeshObject* mo = dynamic_cast<scene::MeshObject*>(so);
		mo->meshBuffer().write( filename.toStdString().c_str() );
	}
	else
	{
		QMessageBox::warning( this, tr("Error"), tr("Selected object is not a mesh!") );
		return;
	}
}

meshtools::Mesh* SceneViewer::loadMeshInternal( QString filename )
{
	using meshtools::Mesh;
	Mesh* mesh = new Mesh();
	if( !meshtools::loadMesh( *mesh, filename.toStdString().c_str() ) )
	{
		// Failure
		delete mesh;
		return NULL;
	}

	qDebug() << "Loaded mesh " << filename;
	qDebug() << "#Vertices = " << mesh->n_vertices();
	qDebug() << "#Faces    = " << mesh->n_faces();

	return mesh;
}

bool SceneViewer::loadMesh( QString filename )
{
	using meshtools::Mesh;
	Mesh* mesh = loadMeshInternal( filename );
	
	if( !mesh )
	{
		// Failure
		return false;
	}
	
	// Add mesh to scene
	QFileInfo info( filename );	
	addMesh( mesh, info.baseName() );

	// Success
	return true;
}

int SceneViewer::loadMeshAnimation( QStringList filenames )
{
	using meshtools::Mesh;
	using scene::MeshObject;

	// Check if a MESHBUFFER file is given
	if( filenames.size() == 1 )
	{
		if( filenames[0].endsWith(".meshbuffer",Qt::CaseInsensitive) ||
			filenames[0].endsWith(".mb",Qt::CaseInsensitive) )
		{
			MeshBuffer mb;
			if( !mb.read(filenames[0].toStdString().c_str()) )
			{
				qDebug() << "Could not load MESHBUFFER mesh animation from "
					<< filenames[0] << "!";
				return -1;
			}

			QFileInfo info( filenames[0] );			
			addMeshBuffer( mb, info.baseName() + QString(" (meshbuffer)") );;
		}
	}

	// Progress dialog
	QProgressDialog progress(tr("Loading mesh animation..."), 
		tr("Abort at current file"), 0, filenames.size(), this );
	progress.setWindowModality( Qt::WindowModal );
	progress.setValue(0);
	progress.show();

	MeshObject* obj = NULL;

	int n=0;
	for( int i=0; i < filenames.size(); i++ )
	{
		progress.setValue( i );
		QApplication::processEvents();
		if( progress.wasCanceled() )
			break;

		Mesh* mesh = loadMeshInternal( filenames[i] );
		if( mesh )
		{
			if( n==0 )
			{
				// First mesh added creates the scene object
				QFileInfo info( filenames[i] );	
				obj = addMesh( mesh, info.baseName() + QString(" (animation)") );
			}
			else
			{
				// Append mesh as frame to animation
				assert( obj );
				assert( obj->numFrames() >= 1 );
				obj->addFrame( mesh );

				// Remark:
				// The mesh is not stored internally and we are still 
				// responsible to free its memory!
				delete mesh; mesh = NULL;
			}

			n++;
		}
		else
		{
			// Failed to load mesh
			qDebug() << tr("Could not load %1!").arg(filenames[i]);
		}
	}
	progress.setValue( filenames.size() );

	return n;
}

//----------------------------------------------------------------------------
// Scene Management
//----------------------------------------------------------------------------

void SceneViewer::addMeshBuffer( const MeshBuffer& mb, QString name )
{
	scene::MeshObject* so = newMeshObject( name );
	so->setMeshBuffer( mb );
	addMeshObject( so );
}

void SceneViewer::addMeshObject( scene::MeshObject* so )
{
	// Add mesh to scene
	m_scene.addSceneObject( so );

	// Update scene graph model
	updateModel();

	// Update camera to show the whole scene
	scene::BoundingBox bbox = m_scene.getBoundingBox();
	bbox.print();
	setSceneBoundingBox( qglviewer::Vec(bbox.min[0],bbox.min[1],bbox.min[2]),
	                     qglviewer::Vec(bbox.max[0],bbox.max[1],bbox.max[2]) );
	camera()->showEntireScene();

	// Force redraw
	updateGL();
}

scene::MeshObject* SceneViewer::addMesh( meshtools::Mesh* mesh_, QString name )
{
	// Take ownership of given Mesh pointer
	using meshtools::Mesh;
	boost::shared_ptr<Mesh> mesh( mesh_ );

	// Create scene object
	scene::MeshObject* so = newMeshObject( name );
	so->setMesh( mesh );

	// Add mesh to scene
	addMeshObject( so );

	return so;
}

scene::MeshObject* SceneViewer::newMeshObject( QString name )
{
	static const double colors[] = {
		1.0000,    1.0000,    1.0000,
		0.3137,    0.6784,    0.9020,
		0.6235,    0.3020,    0.6392,
		0.8902,    0.0902,    0.3725,
		0.9608,    0.5294,    0.1333,
		1.0000,    0.8353,    0.2392,
		1.0000,    0.9608,    0.8314
	};
	static const unsigned numColors = sizeof(colors) / (3*sizeof(double));
	static int lastColor = 0;
	int curColor = (lastColor++) % numColors;

	// Create scene object
	scene::MeshObject* so = new scene::MeshObject();
	so->setName( name.toStdString() );	
	so->setColor( scene::Color(colors[3*curColor], colors[3*curColor+1], colors[3*curColor+2]) );	

	return so;
}

//----------------------------------------------------------------------------
// Rendering
//----------------------------------------------------------------------------

void SceneViewer::reloadShaders()
{
	m_phongShader.init();
	m_meshShader.init();
	updateGL();
}

void SceneViewer::init()
{
	// Restore previous viewer state.
	restoreStateFromFile();

	// Initialize GLEW
	glewExperimental = GL_TRUE; // needed for GL3 on my GLEW 1.5.4
	GLenum glew_err = glewInit();
	if( glew_err != GLEW_OK )
	{
		qDebug() << "GLEW error:" << QString::fromLocal8Bit( (const char*)glewGetErrorString(glew_err) );
		throw; // InitException( "Error on initializing GLEW library!" );
	}
	qDebug() << "Using GLEW " << QString::fromLocal8Bit( (const char*)glewGetString( GLEW_VERSION ) );

	// Set OpenGL states

	// Two-sided rendering and lighting (front- and backfaces)
	glDisable( GL_CULL_FACE );
	glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, 1 );

	// Selection related 
	//setMouseTracking(true);
	this->setSelectRegionWidth( 21 );
	this->setSelectRegionHeight( 21 );	

	m_phongShader.init();
	m_phongShader.setDefaultLighting();
	m_meshShader.init();
	m_meshShader.setDefaultLighting();
}

void SceneViewer::updateScene()
{
	// In redrawing the scene all scene objects will again be evaluated
	// and changes will be reflected in the rendering
	updateGL();
}

void SceneViewer::bindShader()
{
	switch( m_curShader )
	{
	case ShaderPhong: m_phongShader.bind(); break;
	case ShaderMesh : m_meshShader.bind(); break;
	default:
	case ShaderNone : break;
	}
}

void SceneViewer::releaseShader()
{
	glUseProgram( 0 );
}

void SceneViewer::draw()
{
	glPushAttrib( GL_ALL_ATTRIB_BITS );

	// Draw scene objects	
	bindShader();
	m_scene.render();
	releaseShader();

	glDisable( GL_LIGHTING );
	glDisable( GL_DEPTH_TEST );
	glPointSize( 5.f );
	glColor3f( 1,0,0 );

	// Draw selected vertices of current mesh object
	if( !m_selection.empty() && currentMeshObject() )
	{
		currentMeshObject()->renderPoints( m_selection );
	}

	glColor3f( 1,1,0 );
	glPointSize( 10.f );
	glBegin( GL_POINTS );
	glVertex3f( m_selectedPoint.x, m_selectedPoint.y, m_selectedPoint.z );
	glEnd();

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_LIGHTING );

	// Draw brush shape
	if( m_selectionMode != SelectNone )
		drawSelectionRectangle();

	glPopAttrib();
}

//----------------------------------------------------------------------------
// Selection
//----------------------------------------------------------------------------

// We currently select only vertices on front faces. More convenient would be
// to perform selection only on closest surface to viewer. Sadly, deciding
// which points are on closest surface based on depth value is not working yet.
//#define SCENEVIEWER_SELECTION_DEPTH_DISAMBIGUATION

void SceneViewer::selectNone()
{
	// De-select vertices
	if( currentMeshObject() ) 
		currentMeshObject()->meshBuffer().setCBufferSelection( m_selection, false );
	// Clear selection
	m_selection.clear();
	updateGL();
}

void SceneViewer::selectFrontFaces( bool enable )
{
	m_selectFrontFaces = enable;
}

void SceneViewer::drawWithNames()
{
	using namespace scene;

	// Draw each object with a different name (for object selection)
	//m_scene.render( Object::RenderSurface | Object::RenderNames );

	// Draw each vertex of current mesh with a different name (for vertex selection)
	if( currentMeshObject() )
	{
		currentMeshObject()->render( Object::RenderPoints | Object::RenderNames );
	}
}

void SceneViewer::endSelection( const QPoint& point )
{
	// See: http://www.libqglviewer.com/examples/multiSelect.html

	// Flush GL buffers
	glFlush();

	// Get the number of objects that were seen through the pick matrix frustum. Reset GL_RENDER mode.
	GLint nbHits = glRenderMode(GL_RENDER);

	// Find 3D intersection with closest surface point
	bool found;
	qglviewer::Vec selectedPoint;
	m_selectedPoint = QGLViewer::camera()->pointUnderPixel( point, found );

	// Intersection ray
	qglviewer::Vec ray_origin, ray_dir;
	QGLViewer::camera()->convertClickToLine( point, ray_origin, ray_dir );
	ray_dir.normalize();

#ifdef SCENEVIEWER_SELECTION_DEPTH_DISAMBIGUATION
	// Find depth value of closest surface (assuming render mode is surface)
	unsigned depth, depth2;
	const unsigned zrange = std::numeric_limits<unsigned>::max(); // (unsigned)0x7fffffff
	
	// Variant 2: Project intersection point again
	qglviewer::Vec 
		windowPoint = QGLViewer::camera()->projectedCoordinatesOf( m_selectedPoint );
	depth2 = m_selectedPoint.z * zrange;

	// Variant 1: Read depth manually from depth buffer
	updateGL();
	glReadPixels( point.x(), point.y(), 1,1, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, (void*)&depth );
#endif
	
	// Get selection
	std::vector<unsigned> selected;
	if (nbHits > 0)
	{
		// Interpret results : each object created 4 values in the selectBuffer().
		// (selectBuffer())[4*i+3] is the id pushed on the stack.
		for( int i=0; i<nbHits; ++i )
		{
			unsigned id = (selectBuffer())[4*i+3];
			
			bool discard = false;

			// Only consider points on front-facing surfaces
			if( m_selectFrontFaces )
			{
				// Check if hit point is front-facing
				double sign=42.;
				if( currentMeshObject() )
				{
					scene::MeshObject* mobj = currentMeshObject();
					sign = mobj->projectVertexNormal( id, ray_origin.x, ray_origin.y, ray_origin.z );
				}

				// Discard back-facing points
				if( sign < 0.1 )
					discard = true;
			}

		  #ifdef SCENEVIEWER_SELECTION_DEPTH_DISAMBIGUATION
			// Only consider points on closest surface
			if( false )			
			{
				// Debug hit record
				GLuint hitrec[4];			
				memcpy( (void*)&hitrec[0], (void*)&((selectBuffer())[4*i]), sizeof(GLuint)*4 );

				// Window depth of closest hit
				unsigned min_depth = (selectBuffer())[4*i+1];
				double thresh  = abs((double)depth - (double)min_depth) / (double)zrange;
				double thresh2 = abs((double)min_depth - (double)depth2) / (double)zrange;

				// Skip points far away from closest surface
				//if( fabs(dist) > 0.001f )
				//	discard = true;

				qDebug() << id << ": min-depth = " << (unsigned)(selectBuffer())[4*i+1] << ", t1 = " << thresh << ", t2 = " << thresh2 << ", sign = " << sign;
			}
		  #endif

			if( discard )
				continue;

			selected.push_back( id );
		}
	}

	// Apply selection	
	std::vector<unsigned> removed;
	switch( m_selectionMode )
	{
	case SelectAdd    : 
		m_selection.insert( m_selection.end(), selected.begin(), selected.end() );
		if( currentMeshObject() ) 
			currentMeshObject()->meshBuffer().setCBufferSelection( selected );
		break;

	case SelectRemove : 
		//qDebug() << "Selection remove not implemented yet!"; 
		for( unsigned i=0; i < selected.size(); i++ )
		{
			std::vector<unsigned>::iterator it =
			  std::find( m_selection.begin(), m_selection.end(), selected[i] );
			if( it != m_selection.end() )
			{
				m_selection.erase( it );
				removed.push_back( selected[i] );
			}
		}
		if( currentMeshObject() ) 
			currentMeshObject()->meshBuffer().setCBufferSelection( removed, false );
		break;
	default: break;
	}			
}

//----------------------------------------------------------------------------
// Custom mouse events
//----------------------------------------------------------------------------

void SceneViewer::mouseMoveEvent( QMouseEvent* e )
{
	if( m_selectionMode != SelectNone )
	{
		int w = selectRegionWidth(),
		    h = selectRegionHeight();
		m_brushRectangle.setCoords( e->x()-w/2, e->y()-h/2, e->x()+w/2, e->y()+h/2 );
		m_brushRectangle = m_brushRectangle.normalized();

		// Update selection while mouse is moving
		QGLViewer::select( m_brushRectangle.center() );	
		updateGL();
	}

	QGLViewer::mouseMoveEvent(e);
}

void SceneViewer::mousePressEvent( QMouseEvent* e )
{
	// Enter selection mode
	if ((e->button() == Qt::LeftButton) && (e->modifiers() == Qt::ShiftModifier))
		m_selectionMode = SelectAdd;
	else
	if ((e->button() == Qt::LeftButton) && (e->modifiers() == Qt::AltModifier))
		m_selectionMode = SelectRemove;

	QGLViewer::mousePressEvent(e);
}

void SceneViewer::mouseReleaseEvent( QMouseEvent* e )
{
	if( m_selectionMode != SelectNone )
	{
		//// Update selection on mouse key release
		//QGLViewer::select( m_brushRectangle.center() );	
		//updateGL();

		// Leave selection mode on mouse key release
		m_selectionMode = SelectNone;
	}

	QGLViewer::mouseReleaseEvent(e);
}


//----------------------------------------------------------------------------
// Helpers
//----------------------------------------------------------------------------

void SceneViewer::drawSelectionRectangle() const
{
	QRect rect = m_brushRectangle;

	startScreenCoordinatesSystem();
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);

	/*
	glColor4f(0.0, 0.0, 0.3f, 0.3f);
	glBegin(GL_QUADS);
	glVertex2i(rect.left(),  rect.top());
	glVertex2i(rect.right(), rect.top());
	glVertex2i(rect.right(), rect.bottom());
	glVertex2i(rect.left(),  rect.bottom());
	glEnd();
	*/

	glLineWidth(2.0);
	//glColor4f(0.4f, 0.4f, 0.5f, 0.5f);
	glColor3f( 1,1,0 );
	glBegin(GL_LINE_LOOP);
	glVertex2i(rect.left(),  rect.top());
	glVertex2i(rect.right(), rect.top());
	glVertex2i(rect.right(), rect.bottom());
	glVertex2i(rect.left(),  rect.bottom());
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	stopScreenCoordinatesSystem();
}