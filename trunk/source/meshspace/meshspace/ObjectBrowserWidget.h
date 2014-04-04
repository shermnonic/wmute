// ObjectBrowserWidget - scene::Object browser for SceneViewer
// Max Hermann, Apr 2014
#ifndef OBJECTBROWSERWIDGET_H
#define OBJECTBROWSERWIDGET_H

#include <QWidget>
#include <QItemSelection>

class QListView;

/** @addtogroup meshspaceGUI_grp meshspace GUI
  * @{ */

/**
	Browser of current scene objects for \a SceneViewer.
*/
class ObjectBrowserWidget : public QWidget
{
	Q_OBJECT
	
signals:
	void selectionChanged(const QItemSelection&, const QItemSelection&);
	void removeObject( int idx );

public:
	ObjectBrowserWidget( QWidget* parent=0 );

	void setModel( QAbstractItemModel* model );

	int selectedObject() const;

	void select( const QModelIndex& index );

protected slots:
	void removeSelectedObject();
	
private:
	QListView* m_listView;
};

/** @} */ // end group

#endif // OBJECTBROWSERWIDGET_H
