// SceneViewer - GUI for scene::Scene
// Max Hermann, Jan 2014
#ifndef SCENEVIEWER_H
#define SCENEVIEWER_H

#include <GL/glew.h> // must be included before GL.h (and thus before qglviewer.h)
#include <QGLViewer/qglviewer.h>
#include <QStandardItemModel>
#include <QItemSelection>
#include <QRect>
#include <QList>

#include <glutils/PhongShader.h>
#include "MeshShader.h"

#include "scene.h"
#include "MeshObject.h"
#include "meshtools.h"

class ObjectPropertiesWidget;
class QListView;
class QMouseEvent;
class QAction;

/** @addtogroup meshspaceGUI_grp meshspace GUI
  * @{ */

/**
	QGLViewer widget to render a \a scene::Scene.
*/
class SceneViewer : public QGLViewer
{
	Q_OBJECT

	enum SelectionMode {
		SelectNone,
		SelectAdd,
		SelectRemove		
	};

public:
	SceneViewer( QWidget* parent=0 );

	QList<QAction*> getActions() { return m_actions; }
	
public slots:
	/// Load a mesh from disk and add it under its filename to the scene.
	/// Internally calls addMesh() if file could be loaded succesfully.
	bool loadMesh( QString filename );

	/// Load list of meshes as mesh animation with same connectivity.
	/// Returns number of successfully loaded frames.
	int loadMeshAnimation( QStringList filenames );

	/// Add a mesh from memory to the scene, takes ownership of Mesh pointer.
	scene::MeshObject* addMesh( meshtools::Mesh* mesh, QString name );

	/// Returns a list widget of the current scene objects, allowing to toggle
	/// visibility.
	QWidget* getBrowser();
	/// Returns \a ObjectPropertiesWidget holding information about currently
	/// selected scene object.
	QWidget* getInspector();	

	/// Write \a MeshBuffer of currently selected scene object to disk.
	void saveMeshBuffer( QString filename );
	/// Write current mesh to disk.
	void saveMesh( QString filename );

protected slots:
	void updateModel();
	void updateScene();
	void onModelItemChanged( QStandardItem* );
	void selectModelItem( const QModelIndex& );
	void selectModelItem( const QModelIndex& current, const QModelIndex& /*previous*/ );
	void selectModelItems(const QItemSelection& selected , const QItemSelection& deselected );
	int  selectedObject() const; // Returns index of currently selected row in list view

	void selectNone();
	void reloadShaders();
	void selectFrontFaces(bool);
	void computeDistance();
	void computePCA();
	void computeCovariance();
	void exportMatrix();
	void importMatrix();
	
protected:
	///@{ QGLViewer implementation
	void draw();
	void init();
	QString helpString() const;
	// Selection stuff
	void drawWithNames();
	//void postSelection( const QPoint& point );
	void endSelection( const QPoint& point );
	///@}

	///@{ Custom mouse events
	void mousePressEvent  ( QMouseEvent* e );
	void mouseReleaseEvent( QMouseEvent* e );
	void mouseMoveEvent   ( QMouseEvent* e );
	///@}

	///@{ Mesh functions
	/// Add a MeshObject to the scene, update bounding box and force redraw
	void addMeshObject( scene::MeshObject* so );
	/// Add a MeshBuffer directly to the scene, internally calls \a addMeshObject()
	void addMeshBuffer( const MeshBuffer& mb, QString name );
	/// Create a new MeshObject (does not add it to the scene and does not set the the mesh itself!)
	scene::MeshObject* newMeshObject( QString name );
	/// Load a mesh from disk
	meshtools::Mesh* loadMeshInternal( QString filename );	
	/// Return currently selected mesh object or NULL otherwise.
	scene::MeshObject* SceneViewer::currentMeshObject();
	///@}

	///@{ Selection / brush functions
	void drawSelectionRectangle() const;
	///@}

private:
	scene::Scene       m_scene; ///< The scene to be rendered
	QStandardItemModel m_model; ///< A model for manipulating the scenegraph
	ObjectPropertiesWidget* m_propertiesWidget;
	QListView* m_listView;

	// Vertex selection	
	int            m_selectionMode;
	QRect          m_brushRectangle;  ///< Selection viewport rectangle
	qglviewer::Vec m_selectedPoint;   ///< Closest surface intersection
	bool           m_selectFrontFaces;///< Select only vertices on front-faces?

	QList<QAction*> m_actions;
};

/** @} */ // end group

#endif // SCENEVIEWER_H
