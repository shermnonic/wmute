#include "ParameterWidget.h"
#include <QString>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QStringList>
// includes specific to DoubleSpinBoxDelegate
#include <QDoubleSpinBox>
#include <QStyleOptionViewItem>

#include <QDebug>

//=============================================================================
//	DoubleSpinBoxDelegate
//=============================================================================

DoubleSpinBoxDelegate::DoubleSpinBoxDelegate( QObject* parent )
: QItemDelegate( parent )
{
}

QWidget* DoubleSpinBoxDelegate::createEditor( QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &index ) const
{
	QDoubleSpinBox* editor = new QDoubleSpinBox(parent);

	// Set min/max according to model data	
	QModelIndex imin = index.model()->index( index.row(), 2, index.parent() );
	QModelIndex imax = index.model()->index( index.row(), 3, index.parent() );
	double dmin = index.model()->data(imin).toDouble();
	double dmax = index.model()->data(imax).toDouble();
	editor->setMinimum( dmin );
	editor->setMaximum( dmax );

	return editor;
}

void DoubleSpinBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();

    QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
    spinBox->setValue(value);
}

void DoubleSpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const
{
    QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
    spinBox->interpretText();
    double value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void DoubleSpinBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

//=============================================================================
//	ParameterWidget
//=============================================================================

ParameterWidget::ParameterWidget( QWidget* parent )
: QWidget( parent )
{
	m_treeView = new QTreeView();

	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget( m_treeView );
	this->setLayout( layout );
}

void ParameterWidget::setParameters( FloatParameterVector params )
{
	QStandardItemModel* model = new QStandardItemModel( (int)params.size(), 2 );

	model->setHorizontalHeaderLabels( QStringList() 
		<< "Name" << "Value" << "Min" << "Max" );

	FloatParameterVector::iterator it = params.begin();
	int row=0;
	for( ; it != params.end(); it++, row++ )
	{
		QStandardItem *itemName = new QStandardItem( 
			QString::fromStdString( (*it)->name) );		

		QStandardItem *itemValue = new QStandardItem(
			QString::number( (*it)->value ) );

		QStandardItem *itemMin = new QStandardItem(
			QString::number( (double)(*it)->min ) );
		
		QStandardItem *itemMax = new QStandardItem(
			QString::number( (double)(*it)->max ) );
		

		model->setItem( row, 0, itemName  );
		model->setItem( row, 1, itemValue );
		model->setItem( row, 2, itemMin );
		model->setItem( row, 3, itemMax );
	}

	m_treeView->setModel( model );

	connect( model, SIGNAL(itemChanged(QStandardItem*)), 
				this, SLOT(onItemChange(QStandardItem*)) );

	// FIXME: Will the delegate instance created here automatically be deleted?
	DoubleSpinBoxDelegate* valueDelegate = new DoubleSpinBoxDelegate;
	m_treeView->setItemDelegateForColumn( 1, valueDelegate );

	for( int i=0; i < 4; i++ )
		m_treeView->resizeColumnToContents( i );

	m_params = params;
}

void ParameterWidget::onItemChange( QStandardItem* item )
{
	if( item->column() == 1 )
	{
		// Update parameter value.
		// Assume 1-to-1 correspondence between model rows and parameter index.
		int idx = item->row();
		QModelIndex foo = item->model()->index( item->row(), 1 );
		float value = item->model()->data( foo ).toDouble();

		qDebug() << "Setting parameter " << idx << " to value " << value << "\n";

		if( idx>=0 && idx<m_params.size() )
			m_params[ idx ]->value = value;
	}
}
