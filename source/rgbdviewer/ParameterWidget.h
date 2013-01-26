#ifndef PARAMETERWIDGET_H
#define PARAMETERWIDGET_H

#include "Parameters.h"
#include <QWidget>
#include <QItemDelegate>

class QTreeView;
class QStandardItem;

class ParameterWidget : public QWidget
{
	Q_OBJECT

public:
	ParameterWidget( QWidget* parent=0 );

	void setParameters( FloatParameterVector params );

	// TODO:
	// - update slot
	// - callback handler

protected slots:
	void onItemChange( QStandardItem* );

private:
	FloatParameterVector m_params;
	QTreeView* m_treeView;
};

/// Double spin box item delegate
/// See also:
/// http://qt-project.org/doc/qt-4.8/itemviews-spinboxdelegate.html
class DoubleSpinBoxDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	DoubleSpinBoxDelegate( QObject* parent=0 );

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
						const QModelIndex &index) const;

	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model,
					const QModelIndex &index) const;

	void updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // PARAMETERWIDGET_H
