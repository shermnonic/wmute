#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include <QStyledItemDelegate>
#include <QStringList>

class QObject;
class QWidget;

/// Generic item delegate for a combo box selection.
class ComboBoxDelegate : public QStyledItemDelegate
{
	//Q_OBJECT

public:	
	ComboBoxDelegate( QObject* parent=0 );

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

	void setItems( QStringList items );

	int getIndex( QString text );

private:
	QStringList m_items;
};

#endif // COMBOBOXDELEGATE_H
