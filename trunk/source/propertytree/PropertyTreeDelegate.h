#ifndef PROPERTYTREEDELEGATE_H
#define PROPERTYTREEDELEGATE_H

#include <QItemDelegate>
#include "ParameterBase.h"

class QObject;
class QWidget;

/// Item delegate for ParameterBase and its specializations
/// See also:
/// http://qt-project.org/doc/qt-4.8/itemviews-spinboxdelegate.html
class PropertyTreeDelegate : public QItemDelegate
{
	Q_OBJECT

public:	
	PropertyTreeDelegate( QObject* parent=0 );

	// -- QItemDelegate implementation --
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
						const QModelIndex &index) const;

	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model,
					const QModelIndex &index) const;

	void updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option, const QModelIndex &index) const;

	// -- Custom functions --
	void setParameters( ParameterList* params ) { m_params = params; }

protected:
	QWidget* getParameterEditor( ParameterBase* p, QWidget* parent ) const;
	
private:
	ParameterList* m_params;
};

#endif // PROPERTYTREEDELEGATE_H
