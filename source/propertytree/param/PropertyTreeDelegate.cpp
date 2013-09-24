#include "PropertyTreeDelegate.h"
#include "ParameterTypes.h"

#include <QStyleOptionViewItem>
#include <QStyleOptionButton>
#include <QStyle>
#include <QApplication> // for QApplication::style()
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPainter>


PropertyTreeDelegate
  ::PropertyTreeDelegate( QObject* parent )
  : QStyledItemDelegate( parent ),
    m_params( NULL )
{
}

void PropertyTreeDelegate
  ::paint( QPainter *painter, const QStyleOptionViewItem &option,
           const QModelIndex &index ) const
{
	// Draw default style
	QStyledItemDelegate::paint( painter, option, index );

	// Overdraw specific content
	if( index.column() == 1 )
	{
		// Specifically style column 1, containing our editable property values

		// Assume 1:1 mapping between row and parameter index
		ParameterBase* param = (*m_params)[index.row()];

		// Only the checkbox for the bool type is specialized for now
		if( param->type()=="bool" )
		{
			painter->save();

			// Configure checkbox
			bool checked = (bool)index.model()->data(index, Qt::EditRole).toInt();
			QStyleOptionButton butOpt;
			butOpt.state = QStyle::State_Enabled;
			butOpt.state = checked ? QStyle::State_On : QStyle::State_Off;
			butOpt.rect = option.rect;

			// Draw checkbox
			QStyle* style = QApplication::style();			
			style->drawControl( QStyle::CE_CheckBox, &butOpt, painter );

			painter->restore();
		}
	}
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
	if( param->type()=="enum" )
	{
		QString value = index.model()->data(index, Qt::EditRole).toString();
		QComboBox* comboBox = static_cast<QComboBox*>(editor);		
		comboBox->setCurrentIndex( comboBox->findText(value) );		
		comboBox->showPopup(); // Workaround for one-click editing
	}
	else
	if( param->type()=="bool" )
	{
		int value = index.model()->data(index, Qt::EditRole).toInt();
		QCheckBox* checkBox = static_cast<QCheckBox*>(editor);
		checkBox->setChecked( value!=0 );
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
	if( param->type()=="enum" )
	{
		QComboBox* combo = static_cast<QComboBox*>(editor);

		// Instead of casting to EnumParameter we could add a valueAsString() 
		// function to ParameterBase which is overloaded for EnumParameter to 
		// return the enum name.
		EnumParameter* p_enum = dynamic_cast<EnumParameter*>(param);
		if( p_enum )
		{
			std::string value = p_enum->enumNames().at( combo->currentIndex() );
			model->setData( index, QString::fromStdString(value), Qt::EditRole );
		}
	}
	else
	if( param->type()=="bool" )
	{
		QCheckBox* chbx = static_cast<QCheckBox*>(editor);

		BoolParameter* p_bool = dynamic_cast<BoolParameter*>(param);
		if( p_bool )
		{
			int value = (int)chbx->isChecked();
			model->setData( index, value, Qt::EditRole );
		}
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

	// EnumParameter must be checked before its parent class IntParameter
	EnumParameter* p_enum = dynamic_cast<EnumParameter*>( p );
	if( p_enum )
	{
		QComboBox* combo = new QComboBox(parent);
		for( unsigned i=0; i < p_enum->enumNames().size(); i++ )
			combo->addItem( QString::fromStdString(p_enum->enumNames().at(i)) );
		return combo;
	}

	// BoolParameter must be checked before its parent class IntParameter
	BoolParameter* p_bool = dynamic_cast<BoolParameter*>( p );
	if( p_bool )
	{
		QCheckBox* chkbx = new QCheckBox(parent);
		return chkbx;
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
