// SceneViewer - GUI for scene::Scene
// Max Hermann, Jan 2014
#include "SceneViewer.h"
#include "ObjectPropertiesWidget.h"
#include "MeshObject.h"
#include "PCAObject.h"
#include "TensorfieldObject.h"

#include <qfileinfo.h>
#include <QListView>
#include <QDebug>
#include <QProgressDialog>
#include <QApplication>
#include <QMessageBox>
#include <QMouseEvent>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

#include <fstream>

#include <glutils/GLError.h>

SceneViewer::SceneViewer( QWidget* parent )
  : QGLViewer(parent),
    m_currentObject(-1),
	m_selectionMode(SelectNone),
	m_selectFrontFaces(true)
{
	// --- Widgets ---
	m_listView = new QListView();
	m_listView->setModel( &m_model );
	m_listView->setSelectionMode( QAbstractItemView::SingleSelection );
	m_propertiesWidget = new ObjectPropertiesWidget();
	m_propertiesWidget->setWindowTitle(tr("Inspector"));
	
	connect( m_listView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), 
		     this, SLOT(selectModelItem(const QModelIndex&)) );
	connect( m_propertiesWidget, SIGNAL(redrawRequired()), this, SLOT(updateScene()) );

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

	QAction* actComputeDistance = new QAction(tr("Compute closest point distance"),this);
	QAction* actComputePCA = new QAction(tr("Derive PCA model from current mesh buffer"),this);
	QAction* actComputeCovariance = new QAction(tr("Derive covariance tensor field from current PCA model"),this);

	QAction* actExportMatrix = new QAction(tr("Export current mesh vertex matrix as text file"),this);
	QAction* actImportMatrix = new QAction(tr("Import mesh vertex matrix, replacing vertices of current mesh"),this);

	connect( actSelectNone, SIGNAL(triggered()), this, SLOT(selectNone()) );
	connect( actReloadShaders, SIGNAL(triggered()), this, SLOT(reloadShaders()) );
	connect( actSelectFrontFaces, SIGNAL(toggled(bool)), this, SLOT(selectFrontFaces(bool)) );
	connect( actComputeDistance, SIGNAL(triggered()), this, SLOT(computeDistance()) );
	connect( actComputePCA, SIGNAL(triggered()), this, SLOT(computePCA()) );
	connect( actComputeCovariance, SIGNAL(triggered()), this, SLOT(computeCovariance()) );
	connect( actExportMatrix, SIGNAL(triggered()), this, SLOT(exportMatrix()) );
	connect( actImportMatrix, SIGNAL(triggered()), this, SLOT(importMatrix()) );	

	m_actions.push_back( actSelectNone );
	m_actions.push_back( actSelectFrontFaces );
	m_actions.push_back( actReloadShaders );
	m_actions.push_back( actComputeDistance );
	m_actions.push_back( actExportMatrix );	
	m_actions.push_back( actImportMatrix );
	m_actions.push_back( actComputePCA );
	m_actions.push_back( actComputeCovariance );
}

QWidget* SceneViewer::getInspector()
{
	return m_propertiesWidget;
}

QWidget* SceneViewer::getBrowser()
{
	return m_listView;
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
	m_currentObject = index.row();//m_listView->currentIndex().row(); // was: index.row()
	m_propertiesWidget->setSceneObject( m_scene.objects().at( m_currentObject ).get() );
	updateGL();
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
	scene::MeshObject* mo = currentMeshObject();
	if( !mo )
	{
		QMessageBox::warning( this, tr("Error"), tr("Invalid selection!") );
		return;
	}

	mo->meshBuffer().write( filename.toStdString().c_str() );
}

void SceneViewer::saveMesh( QString filename )
{
	scene::MeshObject* mo = currentMeshObject();
	if( !mo )
	{
		QMessageBox::warning( this, tr("Error"), tr("Invalid selection!") );
		return;
	}

	meshtools::Mesh* mesh = mo->meshBuffer().createMesh( mo->meshBuffer().curFrame() );
	meshtools::saveMesh( *mesh, filename.toStdString().c_str() );
	delete mesh;
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

		return 1;
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
	// Assign different colors to scene objects
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
	so->setColor( scene::Color(colors[3*curColor], colors[3*curColor+1], colors[3*curColor+2]) );

	// Add mesh to scene
	m_scene.addSceneObject( so );

	// Update scene graph model
	updateModel();

	// Update camera to show the whole scene
	scene::BoundingBox bbox = m_scene.getBoundingBox();
	setSceneBoundingBox( qglviewer::Vec(bbox.min[0],bbox.min[1],bbox.min[2]),
	                     qglviewer::Vec(bbox.max[0],bbox.max[1],bbox.max[2]) );
	camera()->showEntireScene();
	
	// Debug info
	std::cout << "Bounding box of " << so->getName() << std::endl;
	bbox.print();

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
	// Create scene object
	scene::MeshObject* so = new scene::MeshObject();
	so->setName( name.toStdString() );

	return so;
}

//----------------------------------------------------------------------------
// Rendering
//----------------------------------------------------------------------------

void SceneViewer::reloadShaders()
{
	if( currentMeshObject() )
		currentMeshObject()->reloadShader();
	
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
	glEnable( GL_NORMALIZE );
	glDisable( GL_CULL_FACE );
	glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, 0 );

	// Setup nice material & lighting
	MeshShader::setDefaultLighting();

	// Selection related 
	//setMouseTracking(true);
	this->setSelectRegionWidth( 21 );
	this->setSelectRegionHeight( 21 );	
}

void SceneViewer::updateScene()
{
	// In redrawing the scene all scene objects will again be evaluated
	// and changes will be reflected in the rendering
	updateGL();
}

void SceneViewer::draw()
{
	glPushAttrib( GL_ALL_ATTRIB_BITS );

	GL::CheckGLError("SceneViewer::draw() - berfore rendering the scene");

	//if( m_mode == ModeWireframe )
		//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		//glDisable( GL_CULL_FACE );

	// Draw scene objects
	m_scene.render();

	GL::CheckGLError("SceneViewer::draw() - after rendering the scene");

	glDisable( GL_LIGHTING );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_TEXTURE_1D );

	// Draw selected vertices of current mesh object
	glPointSize( 2.f );
	glColor3f( 1,0,0 );
	if( currentMeshObject() && currentMeshObject()->isVisible() )	
		currentMeshObject()->renderSelectedPoints();

	// Draw surface intersection along selection ray
	if( m_selectionMode!=SelectNone )
	{
		glColor3f( 1,1,0 );
		glPointSize( 10.f );
		glBegin( GL_POINTS );
		glVertex3f( m_selectedPoint.x, m_selectedPoint.y, m_selectedPoint.z );
		glEnd();
	}

	// FIXME: Text rendering does not work correctly, check GL states!
	glColor3f( 1,1,1 );
	drawText( 10,10, tr("current mesh object = %1").arg(m_currentObject) );

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
		currentMeshObject()->selectNone();
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

	// Vertex selection is currently only implemented for MeshObjects
	scene::MeshObject* meshObject = currentMeshObject();
	if( !meshObject )
		return;	
	
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
				double sign = meshObject->projectVertexNormal( id, 
					                ray_origin.x, ray_origin.y, ray_origin.z );

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
	switch( m_selectionMode )
	{
	case SelectAdd    : meshObject->selectVertices( selected );	        break;
	case SelectRemove : meshObject->selectVertices( selected, false );	break;
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

void SceneViewer::exportMatrix()
{
	// Get current mesh object
	scene::MeshObject* mo = currentMeshObject();
	if( !mo )
	{
		QMessageBox::information(this,tr("meshspace"),tr("Please select a mesh to export first!"));
		return;
	}

	// Get filename
	QString filename = QFileDialog::getSaveFileName( this, tr("Export current mesh as text file"),
		QString::fromStdString( mo->getName() ), tr("Text file (*.txt)") );

	if( filename.isEmpty() )
		return;	

	// Create mesh and convert into matrix
	meshtools::Mesh* mesh = mo->meshBuffer().createMesh();
	Eigen::Matrix3Xd mat;
	meshtools::convertMeshToMatrix( *mesh, mat );

	// Write to text file
	std::ofstream of( filename.toStdString().c_str() );
	if( !of.is_open() )
	{
		delete mesh;
		QMessageBox::warning(this,tr("meshspace"),tr("Could not open file for writing!"));
		return;
	}
	meshtools::writeMatrix( mat, of );
	of.close();

	delete mesh;
}

void SceneViewer::importMatrix()
{
	// Get current mesh object
	scene::MeshObject* mo = currentMeshObject();
	if( !mo )
	{
		QMessageBox::information(this,tr("meshspace"),tr("Please select a mesh to export first!"));
		return;
	}

	// Get filename
	QString filename = QFileDialog::getOpenFileName( this, tr("Export current mesh as text file"),
		QString::fromStdString( mo->getName() ), tr("Text file (*.txt)") );

	if( filename.isEmpty() )
		return;	

	// Open file
	std::ifstream f( filename.toStdString().c_str() );
	if( !f.is_open() )
	{
		QMessageBox::warning(this,tr("meshspace"),tr("Could not open file for reading!"));
		return;
	}

	// Read matrix from text file
	Eigen::Matrix3Xd mat;
	meshtools::readMatrix( mat, f );
	f.close();

	// Replace vertices of current mesh by those given in matrix	
	meshtools::Mesh* mesh = mo->meshBuffer().createMesh();	
	meshtools::replaceVerticesFromMatrix( *mesh, mat );

	// Name of new scene object
	QFileInfo info( filename );
	QString name = tr("%1_replaced-vertices").arg(info.baseName());

	// Add as new scene object
	scene::MeshObject* nuobj = new scene::MeshObject;
	nuobj->setMesh( mesh );
	nuobj->setName( name.toStdString() );
	addMeshObject( nuobj );	

	updateGL();
}

//----------------------------------------------------------------------------
// Mesh Filters
//----------------------------------------------------------------------------

#include "filters.h"

void SceneViewer::computeDistance()
{
	using meshtools::Mesh;
	using scene::MeshObject;

	if( m_scene.objects().size() < 2 )
		return;

	MeshObject* mo_source = dynamic_cast<MeshObject*>( m_scene.objects()[0].get() );
	MeshObject* mo_target = dynamic_cast<MeshObject*>( m_scene.objects()[1].get() );

	if( !mo_source || !mo_target )
		return;

	Mesh* source = mo_source->meshBuffer().createMesh();
	Mesh* target = mo_target->meshBuffer().createMesh();

	std::vector<float> dist;
	filters::closestPointDistance( *source, *target, dist );

	mo_source->setScalars( dist );

	delete source;
	delete target;
}

void SceneViewer::computePCA()
{
	using scene::MeshObject;
	using scene::PCAObject;

	MeshObject* mo = currentMeshObject();
	if( !mo )
		return;

	PCAObject* pco = new PCAObject;
	pco->derivePCAModelFrom( *mo );

	pco->setName( std::string("PCA Model of ") + mo->getName() );

	addMeshObject( (MeshObject*)pco );
}

void SceneViewer::computeCovariance()
{
	using scene::MeshObject;
	using scene::PCAObject;
	using scene::TensorfieldObject;

	PCAObject* pco = dynamic_cast<PCAObject*>( currentMeshObject() );
	if( !pco )
	{
#if 1
		TensorfieldObject* tfo = new TensorfieldObject;
		tfo->createTestScene();
		tfo->setName( "Covariance tensor Westin triangle" );
		addMeshObject( (MeshObject*)tfo );
#endif
		return;
	}

	TensorfieldObject* tfo = new TensorfieldObject;
	tfo->deriveTensorsFromPCAModel( pco->getPCAModel() );
	
	tfo->setName( std::string("Covariance tensors of ") + pco->getName() );

	addMeshObject( (MeshObject*)tfo );
}

