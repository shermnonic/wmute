// SceneViewer - GUI for scene::Scene
// Max Hermann, Jan 2014
#include "SceneViewer.h"
#include "ObjectPropertiesWidget.h"
#include "ObjectBrowserWidget.h"
#include "MeshObject.h"
#include "PCAObject.h"
#include "TensorfieldObject.h"
#include "Crossvalidate.h"

#include <qfileinfo.h>
#include <QDebug>
#include <QProgressDialog>
#include <QApplication>
#include <QMessageBox>
#include <QMouseEvent>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileInfo>

#include <fstream>

#include <glutils/GLError.h>

QAction* genSeparator( QWidget* parent )
{
	QAction* sep = new QAction(parent); 
	sep->setSeparator(true);
	return sep;
}

SceneViewer::SceneViewer( QWidget* parent )
  : QGLViewer(parent),
	m_selectionMode(SelectNone),
	m_selectFrontFaces(true),
	m_pointSize(4.2f)
{
	// --- Widgets ---

	m_browserWidget = new ObjectBrowserWidget;
	m_browserWidget->setWindowTitle(tr("Browser"));
	m_browserWidget->setModel( &m_model );

	m_propertiesWidget = new ObjectPropertiesWidget();
	m_propertiesWidget->setWindowTitle(tr("Inspector"));
	
	connect( m_browserWidget, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
			 this, SLOT(selectModelItems(const QItemSelection&, const QItemSelection&)) );
	connect( m_browserWidget, SIGNAL(removeObject(int)), this, SLOT(removeObject(int)) );
	connect( m_propertiesWidget, SIGNAL(redrawRequired()), this, SLOT(updateScene()) );
	connect( m_propertiesWidget, SIGNAL(modelChanged()), this, SLOT(updateModel()) );

	// --- Actions ---
	// Shortcut description is also added to QGLViewer help.

	QAction* actSelectNone = new QAction(tr("Select none"),this);
	actSelectNone->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_A );
	QGLViewer::setKeyDescription( Qt::CTRL + Qt::SHIFT + Qt::Key_A, "Select none (deselects all vertices)" );

	QAction* actSelectFrontFaces = new QAction(tr("Select vertices only on front faces"),this);
	actSelectFrontFaces->setCheckable( true );
	actSelectFrontFaces->setChecked( m_selectFrontFaces );

	QAction* actExportSelection = new QAction(tr("Export selection..."),this);

	QAction* actReloadShaders = new QAction(tr("Reload shaders"),this);
	actReloadShaders->setShortcut( Qt::CTRL + Qt::Key_R );
	QGLViewer::setKeyDescription( Qt::CTRL + Qt::Key_R, "Reload shaders" );

	QAction* actNormalizeScale = new QAction(tr("Scale mesh animation to unit diagonal"),this);

	QAction* actComputeDistance = new QAction(tr("Compute closest point distance"),this);
	QAction* actComputePCA = new QAction(tr("Derive PCA model from current mesh buffer"),this);
	QAction* actComputeEmbedding = new QAction(tr("Compute covariance embedding"),this);
	QAction* actComputeCovariance = new QAction(tr("Derive covariance tensor field from current PCA model"),this);
	QAction* actClusterCovariance = new QAction(tr("Cluster current covariance tensor field"),this);
	QAction* actLoadCovariance = new QAction(tr("Load covariance tensor field from disk"),this);
	QAction* actExportCovariance = new QAction(tr("Export covariance tensor field to Nrrd"),this);
	QAction* actCrossvalidate = new QAction(tr("Cross-validate gamma on current PCA model"),this);
	QAction* actComputeEigenmodes = new QAction(tr("Compute eigenmodes"),this);

	QAction* actExportMatrix = new QAction(tr("Export current mesh vertex matrix as text file"),this);
	QAction* actImportMatrix = new QAction(tr("Import mesh vertex matrix, replacing vertices of current mesh"),this);

	connect( actSelectNone, SIGNAL(triggered()), this, SLOT(selectNone()) );
	connect( actSelectFrontFaces, SIGNAL(toggled(bool)), this, SLOT(selectFrontFaces(bool)) );
	connect( actExportSelection, SIGNAL(triggered()), this, SLOT(exportSelection()) );
	connect( actReloadShaders, SIGNAL(triggered()), this, SLOT(reloadShaders()) );
	connect( actNormalizeScale, SIGNAL(triggered()), this, SLOT(normalizeScale()) );
	connect( actComputeDistance, SIGNAL(triggered()), this, SLOT(computeDistance()) );
	connect( actComputePCA, SIGNAL(triggered()), this, SLOT(computePCA()) );
	connect( actComputeEmbedding, SIGNAL(triggered()), this, SLOT(computeCovarianceEmbedding()) );
	connect( actComputeCovariance, SIGNAL(triggered()), this, SLOT(computeCovariance()) );
	connect( actClusterCovariance, SIGNAL(triggered()), this, SLOT(computeClustering()) );
	connect( actLoadCovariance, SIGNAL(triggered()), this, SLOT(loadCovariance()) );
	connect( actExportCovariance, SIGNAL(triggered()), this, SLOT(exportCovariance()) );
	connect( actCrossvalidate, SIGNAL(triggered()), this, SLOT(computeCrossValidation()) );
	connect( actExportMatrix, SIGNAL(triggered()), this, SLOT(exportMatrix()) );
	connect( actImportMatrix, SIGNAL(triggered()), this, SLOT(importMatrix()) );	
	connect( actComputeEigenmodes, SIGNAL(triggered()), this, SLOT(computeEigenmodes()) );

	m_actions.push_back( actSelectNone );
	m_actions.push_back( actSelectFrontFaces );
	m_actions.push_back( actExportSelection );
	m_actions.push_back( actExportSelection );
	m_actions.push_back( genSeparator(this) );
	m_actions.push_back( actNormalizeScale );
	m_actions.push_back( genSeparator(this) );
	m_actions.push_back( actReloadShaders );
	m_actions.push_back( genSeparator(this) );
	m_actions.push_back( actComputeDistance );
	m_actions.push_back( actComputeEigenmodes );
	m_actions.push_back( actExportMatrix );	
	m_actions.push_back( actImportMatrix );
	m_actions.push_back( genSeparator(this) );
	m_actions.push_back( actComputePCA );
	m_actions.push_back( genSeparator(this) );
	m_actions.push_back( actComputeEmbedding );
	m_actions.push_back( genSeparator(this) );
	m_actions.push_back( actComputeCovariance );
	m_actions.push_back( actClusterCovariance );
	m_actions.push_back( actLoadCovariance );
	m_actions.push_back( actExportCovariance );
	m_actions.push_back( actCrossvalidate );
}

QWidget* SceneViewer::getInspector()
{
	return m_propertiesWidget;
}

QWidget* SceneViewer::getBrowser()
{
	return m_browserWidget;
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

void SceneViewer::removeObject( int idx )
{
	m_scene.removeSceneObject( idx );
	updateModel();
	updateBoundingBox();

	// Force redraw
	updateGL();
}

int SceneViewer::selectedObject() const
{	
	return m_browserWidget->selectedObject();
}

void SceneViewer::selectModelItem( const QModelIndex& current, const QModelIndex& previous )
{
	selectModelItem( current );
}

void SceneViewer::selectModelItem( const QModelIndex& index )
{
	m_propertiesWidget->setSceneObject( m_scene.objects().at( selectedObject() ).get() );
	updateGL();
}

void SceneViewer::selectModelItems( const QItemSelection& selected , const QItemSelection& deselected )
{
	if( selected.empty() )
		m_propertiesWidget->setSceneObject( NULL );
	else
	{
		int idx = selected.indexes().front().row();
		m_propertiesWidget->setSceneObject( m_scene.objects().at( idx ).get() );
	}
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
	// Sanity
	int sel = selectedObject();
	if( sel < 0 || sel > m_scene.objects().size() || m_scene.objects().empty() ) 
		return NULL;
	return dynamic_cast<scene::MeshObject*>( m_scene.objects().at(sel).get() );
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

	// was: meshtools::Mesh* mesh = mo->meshBuffer().createMesh( mo->meshBuffer().curFrame() );
	meshtools::Mesh* mesh = mo->createMesh();
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
	qDebug() << "Mesh has vertex normals : " << (mesh->has_vertex_normals() ? "yes" : "no");

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
	updateBoundingBox();
	camera()->showEntireScene();

	// Auto-select the last added object
	QModelIndex insertedItem = m_model.item( m_model.rowCount()-1 )->index();
	// Highlight selection
	m_browserWidget->select( insertedItem );
	// Set as current scene object
	selectModelItem( insertedItem );

	// Force redraw
	updateGL();
}

void SceneViewer::updateBoundingBox()
{
	scene::BoundingBox bbox = m_scene.getBoundingBox();
	setSceneBoundingBox( qglviewer::Vec(bbox.min[0],bbox.min[1],bbox.min[2]),
	                     qglviewer::Vec(bbox.max[0],bbox.max[1],bbox.max[2]) );	
	
	// Debug info
	std::cout << "Scene bounding box : "; bbox.print();
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
	glEnable( GL_POINT_SMOOTH );

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

	//......................................
	// Render scene

	GL::CheckGLError("SceneViewer::draw() - berfore rendering the scene");

	//if( m_mode == ModeWireframe )
		//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		//glDisable( GL_CULL_FACE );

	// Draw scene objects
	glPointSize( m_pointSize ); // Point size for point cloud objects
	m_scene.render();

	GL::CheckGLError("SceneViewer::draw() - after rendering the scene");

	//......................................
	// Render color bar

	// Show transfer function only for tensor field objects for now
	if( currentMeshObject() && dynamic_cast<scene::TensorfieldObject*>(currentMeshObject()) )
		currentMeshObject()->meshShader().getTransferFunction().draw();

	//......................................
	// Render selection

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

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_LIGHTING );

	// Draw brush shape
	if( m_selectionMode != SelectNone )
		drawSelectionRectangle();

	//......................................
	// Render text

	glPopAttrib();	

	// FIXME: Text rendering does not always work correctly, check GL states!
	glColor3f( 1,1,1 );
	drawText( 10,23, tr("current mesh object = %1").arg(selectedObject()) );
	
	scene::MeshObject* mo = currentMeshObject();
	if( mo && mo->isAnimation() )
	{
		drawText( 10,35, tr("frame %1 / %2").arg(mo->curFrame()+1).arg(mo->numFrames()) );
	}
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

void SceneViewer::exportSelection()
{
	// Get mesh object
	scene::MeshObject* mo = currentMeshObject();
	if( !mo )
	{
		QMessageBox::warning( this, tr("Export selection warning"),
			tr("No mesh object selected!") );
		return;
	}

	// Get selection
	typedef std::set<unsigned> Selection;
	Selection selection = mo->getSelectedVertices();
	if( selection.empty() )
	{
		QMessageBox::warning( this, tr("Export selection warning"),
			tr("No vertices selected on current mesh object!") );
		return;
	}

	// Get filename
	QString filename = QFileDialog::getSaveFileName( this, 
		tr("Export selection"), QString(), tr("*.txt") );

	if( filename.isEmpty() )
		return;

	// Save selection to file
	std::ofstream f( filename.toStdString().c_str() );
	if( !f.is_open() )
	{
		QMessageBox::warning( this, tr("Export selection warning"),
			tr("Could not open %1!").arg(filename) );
		return;
	}

	std::cout << "Selection:" << std::endl;
	for( Selection::const_iterator it=selection.begin(); it!=selection.end(); ++it )
	{
		std::cout << (*it) << std::endl; // DEBUG
		f << *it << std::endl;
	}

	f.close();
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

	// DEBUG
	if( m_selectionMode==SelectAdd && !selected.empty())
	{
		qDebug() << "Selected vertices:";
		for( int i=0; i < selected.size(); i++ )
			qDebug() << selected[i];
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

void SceneViewer::wheelEvent( QWheelEvent* e )
{
	if( e->modifiers() == Qt::AltModifier )
	{
		// Modify point size
		int steps = e->delta() / 120;
		m_pointSize += steps;
		if( m_pointSize <= 0.5 ) m_pointSize = 0.5;
		if( m_pointSize > 20.0 ) m_pointSize = 20.0;
		updateScene(); // Force re-render
		e->accept();
	}
	else
		e->ignore();
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

#include "MeshLaplacian.h"
void SceneViewer::computeEigenmodes()
{
	using meshtools::Mesh;
	using scene::MeshObject;
	
	MeshObject* mo = currentMeshObject();
	if( !mo )
		return;

	Mesh* mesh = mo->createMesh();

	MeshLaplacian ml;
	ml.compute( *mesh );

	std::vector<float> eigenmode;	
	ml.getMode( 1, eigenmode );

	mo->setScalars( eigenmode );

	delete mesh;
}

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

	// Get PCA object, if not available create demo scene
	PCAObject* pco = dynamic_cast<PCAObject*>( currentMeshObject() );
	if( !pco )
	{
		// Create demo scene (Westin triangle)
		TensorfieldObject* tfo = new TensorfieldObject;
		tfo->createTestScene();
		tfo->setName( "Covariance tensor Westin triangle" );
		addMeshObject( (MeshObject*)tfo );
		return;
	}

	// Ask for tensor mode
	QStringList modes;
	modes << "Anatomic covariance" 
		  << "Inter-point covariance"
	      << "Inter-point covariance (unweighted)";
	bool ok;
	QString mode_s = QInputDialog::getItem( this, tr("Tensor field options"),
		tr("Choose type of tensor field"), modes, 0, false, &ok );
	if( !ok ) // User cancelled?
		return;

	int mode;
	if( mode_s == modes.at(0) ) mode = TensorfieldObject::AnatomicCovariance; else
	if( mode_s == modes.at(1) ) mode = TensorfieldObject::InterPointCovariance; else
	if( mode_s == modes.at(2) ) mode = TensorfieldObject::InterPointCovarianceUnweighted;
	else
	{
		// Should never happen
		qDebug() << "SceneViewer::computeCovariance() : mismatching tensor mode?!";
		return;
	}

	// Ask inter-point covariance specific parameters
	double gamma=0.0;
	double scale=1.0;
	if( mode>0 )	
	{
		gamma = QInputDialog::getDouble( this, tr("Tensor field options"),
		tr("Specify regularization parameter gamma"), 100.0, 0.0001, 2147483647.0, 4, &ok );
		if( !ok ) // User cancelled?
			return;

		scale = QInputDialog::getDouble( this, tr("Tensor field options"),
		tr("Specify scale factor for inter-point tensor"), 1.0, 0.0000001, 2147483647.0, 7, &ok );
		if( !ok ) // User cancelled?
			return;
	}

	// Create tensorfield object
	TensorfieldObject* tfo = new TensorfieldObject;
	tfo->deriveTensorsFromPCAModel( pco->getPCAModel(), mode, gamma, scale );
	tfo->setName( modes.at(mode).toStdString() + std::string(" of ") + pco->getName() );

	addMeshObject( (MeshObject*)tfo );
}

void SceneViewer::loadCovariance()
{
	using scene::TensorfieldObject;
	using scene::MeshObject;

	// Duplicate code from TensorfieldObjectWidget::loadTensors()!

	// Ask for filename
	QString filename = 
		QFileDialog::getOpenFileName( this, tr("Load tensor field"), "",
		tr("Tensor field matrix (*.tensorfield)") );
	if( filename.isEmpty() ) // User cancelled ?
		return;

	// Create tensorfield object
	TensorfieldObject* tfo = new TensorfieldObject;
	if( !tfo->loadTensorfield( filename.toStdString() ) )
	{
		QMessageBox::warning( this, tr("Error loading tensor field"),
			tr("Could not load a valid tensor field from \"%1\"!")
			.arg(filename) );
		delete tfo;
		return;
	}
	QFileInfo fi( filename );
	tfo->setName( fi.baseName().toStdString() );

	addMeshObject( (MeshObject*)tfo );
}

void SceneViewer::exportCovariance()
{
	using scene::TensorfieldObject;
	using scene::MeshObject;

	TensorfieldObject* tfo = dynamic_cast<TensorfieldObject*>( currentMeshObject() );
	if( !tfo )
		return;

	// Ask for filename
	QString filename = 
		QFileDialog::getSaveFileName( this, tr("Export tensor field as nrrd"), "",
		tr("Nrrd file format (.nrrd)") );
	if( filename.isEmpty() ) // User cancelled ?
		return;

	QFileInfo fi( filename );
	tfo->exportTensorfieldAsNrrd( fi.absolutePath().toStdString(), fi.baseName().toStdString() );
}

void SceneViewer::computeCrossValidation()
{
	using scene::MeshObject;
	using scene::PCAObject;

	PCAObject* pco = dynamic_cast<PCAObject*>( currentMeshObject() );
	if( !pco )
		return;

	// FIXME: Hard-coded test
	std::vector<double> gamma, 
		                error,
						baseline;
	//gamma[ 0] = 10000000;
	//gamma[ 1] =  5000000;
	//gamma[ 2] =  2000000;
	//gamma[ 3] =  1500000;
	//gamma[ 4] =  1200000;
	//gamma[ 5] =  1000000;
	//gamma[ 6] =   800000;
	//gamma[ 7] =   500000;
	//gamma[ 8] =   200000;
	//gamma[ 9] =    50000;
	//gamma[10] =    10000;
	//gamma[11] =     2000;
	//gamma[12] =      500;
	//gamma[13] =       50;
	//gamma[14] =       10;
	//gamma[15] =        1;
	//gamma[16] =        0.1;

	gamma.push_back( 10 );
	gamma.push_back( 5 );
	gamma.push_back( 1 );
	gamma.push_back( 0.5 );
	gamma.push_back( 0.1 );
	gamma.push_back( 0.05 );
	gamma.push_back( 0.01 );
	gamma.push_back( 0.005 );
	gamma.push_back( 0.001 );
	gamma.push_back( 0.0005 );

	unsigned numSamples = (unsigned)gamma.size();

	// HACK: Assemble data matrix from PCA model
	Eigen::MatrixXd X = pco->getPCAModel().X;
	for( unsigned col=0; col < X.cols(); col++ )
		X.col(col) += pco->getPCAModel().mu;

	// Perform cross validation (expensive!)
	crossvalidate( X, gamma, error, baseline );

	// Print results
	using namespace std;
	cout << "Cross-validation result:" << endl;
	for( unsigned i=0; i < error.size(); i++ )
		cout << gamma[i] << ", " << error[i] << endl;	
	cout << "Baseline error:" << endl;
	for( unsigned i=0; i < baseline.size(); i++ )
		cout << baseline[i] << endl;
}

#include "CovarianceClustering.h"
void SceneViewer::computeClustering()
{
	using scene::TensorfieldObject;
	using scene::MeshObject;

	TensorfieldObject* tfo = dynamic_cast<TensorfieldObject*>( currentMeshObject() );
	if( !tfo )
		return;

	// Cluster parameters
	CovarianceClustering::ClusterParms parms;
	parms.k       = 10;
	parms.maxIter = 100;
	parms.repetitions = 10;

	parms.k = QInputDialog::getInt( this, tr("Clustering parameters"),
		tr("Number of clusters"), parms.k, 2, 1000 );
	parms.repetitions = QInputDialog::getInt( this, tr("Clustering parmeters"),
		tr("Number of repetitions"), parms.repetitions, 1, 1000 );
	//parms.maxIter = QInputDialog::getInt( this, tr("Clustering parmeters"),
	//	tr("Max. number of iterations"), parms.maxIter, 1, 10000 );
	parms.weightTensorDist = QInputDialog::getDouble( this, tr("Clustering parameters"),
		tr("Weight factor for Tensor distance"), parms.weightTensorDist, 0.0, 1000000.0, 2 );
	parms.weightPointDist = QInputDialog::getDouble( this, tr("Clustering parameters"),
		tr("Weight factor for Point distance"), parms.weightPointDist, 0.0, 1000000.0, 2 );
	
	// Compute clustering
	CovarianceClustering cl;
	cl.compute( tfo->getTensorField(), tfo->getPositions(), parms );
	
	// Update tensor visualization
	tfo->setClusters( cl.getLabels(), parms.k );

	// Transfer to mesh buffer
	bool ok;
	QStringList names;
	for( unsigned i=0; i < m_scene.objects().size(); i++ )
		names.push_back( QString(m_scene.objects().at(i)->getName().c_str()) );
	QString selName = QInputDialog::getItem( this, tr("Select mesh to color"), 
		tr("Select a mesh object to apply the cluster coloring (optional)"),
		names, 0, false, &ok );
	if( ok )
	{
		int selIdx=names.size();
		for( int i=0; i < names.size(); i++ )
			if( names.at(i).compare( selName )==0 )
			{
				selIdx = i;
				break;
			}
		if( selIdx < names.size() )
		{
			MeshObject* mo = dynamic_cast<MeshObject*>( m_scene.objects().at(selIdx).get() );
			if( !mo )
				qDebug() << "SceneViewer::computeClustering() : Transfer selection is not a mesh!";				
			else
			{
				if( mo->numVertices() != cl.getLabels().size() )
					qDebug() << "SceneViewer::computeClustering() : Mismatch between mesh and cluster label size!";
				else
				{
					// Set mesh scalars according to cluster index
					std::vector<float> scalars( cl.getLabels().size() );
					for( unsigned i=0; i < cl.getLabels().size(); i++ )
						scalars[i] = (float)cl.getLabels().at(i) / (parms.k-1);
					mo->setScalars( scalars );
				}
			}
		}
	}
}

#include "CovarianceEmbedding.h"
void SceneViewer::computeCovarianceEmbedding()
{
	CovarianceEmbedding ce;

	scene::MeshObject* mo = currentMeshObject();
	if( !mo )
		return;

	int numGroups = 20,
		numFramesPerGroup = 10;

	numGroups = QInputDialog::getInt( this, tr("Embedding parameters"),
		tr("Number of groups"), numGroups, 2, mo->numFrames() );

	numFramesPerGroup = QInputDialog::getInt( this, tr("Embedding parameters"),
		tr("Number of frames per group"), numFramesPerGroup, 1, mo->numFrames() );

	ce.setup( mo->meshBuffer(), 
		CovarianceEmbedding::genLabels( mo->meshBuffer().numFrames(), numGroups, numFramesPerGroup ) );
	ce.compute();
}

void SceneViewer::normalizeScale()
{
	scene::MeshObject* mo = currentMeshObject();
	if( !mo )
		return;

	// Rescale vertex buffer to unit diagonal
	mo->meshBuffer().normalizeSize();

	// Update MeshObject mesh
	mo->updateMesh();

	// Update camera to show the whole scene
	updateBoundingBox();
	camera()->showEntireScene();

	// Force redraw
	updateGL();

}
