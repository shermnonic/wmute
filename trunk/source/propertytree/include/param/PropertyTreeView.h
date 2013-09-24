#ifndef PROPERTYTREEVIEW_H
#define PROPERTYTREEVIEW_H

#include <QTreeView>

class QMouseEvent;

/// Custom tree view providing one-click editing.
/// Implementation follows:
/// http://stackoverflow.com/questions/18831242/qt-start-editing-of-cell-after-one-click
class PropertyTreeView : public QTreeView
{
	Q_OBJECT

public:
	PropertyTreeView( QWidget* parent=0 );

	void setOneClickEditing( bool enable );

protected slots:
	void mousePressEvent( QMouseEvent* e );

private:
	bool m_oneClickEditing;
};

#endif // PROPERTYTREEVIEW_H
