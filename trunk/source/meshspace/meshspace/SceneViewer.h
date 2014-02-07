// SceneViewer - GUI for scene::Scene
// Max Hermann, Jan 2014
#ifndef SCENEVIEWER_H
#define SCENEVIEWER_H

#include <GL/glew.h> // must be included before GL.h (and thus before qglviewer.h)
#include <QGLViewer/qglviewer.h>
#include <QStandardItemModel>
#include <QRect>

#include "scene.h"
#include "meshtools.h"

class ObjectPropertiesWidget;
class QListView;
class QMouseEvent;

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
	
public slots:
	/// Load a mesh from disk and add it under its filename to the scene.
	/// Internally calls addMesh() if file could be loaded succesfully.
	bool loadMesh( QString filename );

	/// Load list of meshes as mesh animation with same connectivity.
	/// Returns number of successfully loaded frames.
	int loadMeshAnimation( QStringList filenames );

	/// Add a mesh from memory to the scene, takes ownership of Mesh pointer.
	scene::MeshObject* addMesh( meshtools::Mesh* mesh, QString name );
	
	/// Show a list widget of the current scene objects, allowing to toggle
	/// visibility.
	void showBrowser();

	/// Show \a ObjectPropertiesWidget holding information about currently
	/// selected scene object.
	void showInspector();

	/// Write \a MeshBuffer of currently selected scene object to disk.
	void saveMeshBuffer( QString filename );

protected slots:
	void updateModel();
	void updateScene();
	void onModelItemChanged( QStandardItem* );
	void selectModelItem( const QModelIndex& );
	void selectModelItem( const QModelIndex& current, const QModelIndex& /*previous*/ );
	
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
	int m_currentObject; ///< Currently selected scene object (row) in list view

	// Vertex selection	
	std::vector<unsigned> m_selection; ///< Selected vertex indices in current object
	int m_selectionMode;
	QRect m_brushRectangle;
};

#endif // SCENEVIEWER_H
