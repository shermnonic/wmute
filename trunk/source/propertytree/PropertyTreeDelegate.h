#ifndef PROPERTYTREEDELEGATE_H
#define PROPERTYTREEDELEGATE_H

#include <QStyledItemDelegate>
#include "ParameterBase.h"

class QObject;
class QWidget;

/// Item delegate for ParameterBase and its specializations.
/// Probably a QItemEditorFactory() is more appropriate for the task to supply
/// different delegate widgets for the individual parameter types.
/// See also QStyledItemDelegate.
class PropertyTreeDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:	
	PropertyTreeDelegate( QObject* parent=0 );

	// -- QItemDelegate implementation --

	 void paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const;

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
