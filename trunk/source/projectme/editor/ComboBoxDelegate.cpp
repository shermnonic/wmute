#include "ComboBoxDelegate.h"
#include <QComboBox>

//----------------------------------------------------------------------------
ComboBoxDelegate
  ::ComboBoxDelegate( QObject* parent )
  : QStyledItemDelegate( parent )
{
	// Dummy data
	m_items << "First" << "Second" << "Third";
}

//----------------------------------------------------------------------------
void ComboBoxDelegate
  ::paint( QPainter *painter, const QStyleOptionViewItem &option,
           const QModelIndex &index ) const
{
#if 1
	// Draw default style
	QStyledItemDelegate::paint( painter, option, index );
#else
	QStyleOptionViewItemV4 myOption = option;
	QString text = Items[index.row()].c_str();
 
	myOption.text = text;
 
	QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);
#endif
}

//----------------------------------------------------------------------------
QWidget* ComboBoxDelegate
  ::createEditor( QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/*index*/ ) const
{
	QComboBox* editor = new QComboBox( parent );
	
	editor->insertItems( 0, m_items );
	
	return (QWidget*)editor;
}

//----------------------------------------------------------------------------
void ComboBoxDelegate
  ::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
#if 0
	QComboBox *comboBox = static_cast<QComboBox*>(editor);
	int value = index.model()->data(index, Qt::EditRole).toUInt();
	comboBox->setCurrentIndex(value);
#else
	QString value = index.model()->data(index, Qt::EditRole).toString();
	QComboBox* comboBox = static_cast<QComboBox*>(editor);		
	comboBox->setCurrentIndex( comboBox->findText(value) );		
	comboBox->showPopup(); // Workaround for one-click editing
#endif
}

//----------------------------------------------------------------------------
void ComboBoxDelegate
  ::setModelData( QWidget *editor, QAbstractItemModel *model,
				  const QModelIndex &index ) const
{	
	QComboBox* combo = static_cast<QComboBox*>(editor);
	QString value = (combo->currentIndex() >= 0 && combo->currentIndex() < m_items.size()) ?
		m_items[ combo->currentIndex() ] : tr("(invalid)");
	model->setData( index, value, Qt::EditRole );
}

//----------------------------------------------------------------------------
void ComboBoxDelegate
  ::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, 
                          const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

//----------------------------------------------------------------------------
void ComboBoxDelegate
	::setItems( QStringList items )
{
	m_items = items;
}

//----------------------------------------------------------------------------
int ComboBoxDelegate
	::getIndex( QString text )
{
	return m_items.indexOf( text );
}
