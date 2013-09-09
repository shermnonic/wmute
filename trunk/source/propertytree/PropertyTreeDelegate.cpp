#include "PropertyTreeDelegate.h"
#include "ParameterTypes.h"

#include <QStyleOptionViewItem>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLineEdit>

PropertyTreeDelegate
  ::PropertyTreeDelegate( QObject* parent )
  : QItemDelegate( parent ),
    m_params( NULL )
{
}

QWidget* PropertyTreeDelegate
  ::createEditor( QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &index ) const
{
	if( !m_params ) return NULL; // FIXME: Return string editor as default?!

	// Assume 1:1 mapping between row and parameter index
	ParameterBase* param = (*m_params)[index.row()];
	
	// Set editor according to parameter type	
	QWidget* editor = getParameterEditor( param, parent );
	
	return editor;
}

void PropertyTreeDelegate
  ::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();

	// Assume 1:1 mapping between row and parameter index
	ParameterBase* param = (*m_params)[index.row()];
	
	if( param->type()=="double" )
	{
		double value = index.model()->data(index, Qt::EditRole).toDouble();
		QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
		spinBox->setValue( value );
	}
	else
	if( param->type()=="int" )
	{
		int value = index.model()->data(index, Qt::EditRole).toInt();		
		QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
		spinBox->setValue( value );
	}
	else
	if( param->type()=="string" )
	{
		QString value = index.model()->data(index, Qt::EditRole).toString();
		QLineEdit* edit = static_cast<QLineEdit*>(editor);
		edit->setText( value );
	}
	else
	{
		// Unkown type
	}	
}

void PropertyTreeDelegate
  ::setModelData( QWidget *editor, QAbstractItemModel *model,
				  const QModelIndex &index ) const
{	
	// Assume 1:1 mapping between row and parameter index
	ParameterBase* param = (*m_params)[index.row()];
	
	if( param->type()=="double" )
	{
		QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
		spinBox->interpretText();
		double value = spinBox->value();
		model->setData( index, value, Qt::EditRole );
	}
	else
	if( param->type()=="int" )
	{
		QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
		spinBox->interpretText();
		int value = spinBox->value();
		model->setData( index, value, Qt::EditRole );
	}
	else
	if( param->type()=="string" )
	{
		QLineEdit* edit = static_cast<QLineEdit*>(editor);
		QString value = edit->text();
		model->setData( index, value, Qt::EditRole );
	}
	else
	{
		// Unkown type
	}	
}

void PropertyTreeDelegate
  ::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, 
                          const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}


QWidget* PropertyTreeDelegate
  ::getParameterEditor( ParameterBase* p, QWidget* parent ) const
{
	DoubleParameter* p_double = dynamic_cast<DoubleParameter*>( p );
	if( p_double )
	{
		QDoubleSpinBox* spin = new QDoubleSpinBox(parent);
		spin->setRange( p_double->limits().min_, p_double->limits().max_ );
		return spin;
	}

	IntParameter* p_int = dynamic_cast<IntParameter*>( p );
	if( p_int )
	{
		QSpinBox* spin = new QSpinBox(parent);
		spin->setRange( p_int->limits().min_, p_int->limits().max_ );
		return spin;
	}

	StringParameter* p_string = dynamic_cast<StringParameter*>( p );
	if( p_string )
	{
		QLineEdit* edit = new QLineEdit(parent);		
		return edit;
	}	
	
	return NULL; // FIXME: Is returning NULL here allowed?
}
