// Sketch for a factory design of item delegates for specific parameter types.
// Sepember 2013

class ParameterItemDelegate : public QStyledItemDelegate
{
//	Q_OBJECT  // No signal/slots used

public:	

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
	virtual void paintParam( QPainter *painter, const QStyleOptionViewItem &option,
		const QModelIndex &index, QStyle* style, ParameterBase* param ) const = 0;

	ParameterBase* getParam( const QModelIndex& index );

private:
	ParameterList* m_params;
};

class BoolParameterItemDelegate : public ParameterItemDelegate
{
//	Q_OBJECT  // No signal/slots used

public:	
	BoolParameterItemDelegate( QObject* parent=0 );

	// -- QItemDelegate implementation --

	void paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const;

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
						const QModelIndex &index) const;

	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model,
					const QModelIndex &index) const;

protected:
	QWidget* getParameterEditor( ParameterBase* p, QWidget* parent ) const;	
};

QWidget* ParameterItemDelegate
  ::createEditor( QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &index ) const
{
	if( !m_params ) return NULL; // FIXME: Return string editor as default?!

	ParameterBase* param = getParam( index );
	
	// Set editor according to parameter type	
	QWidget* editor = getParameterEditor( param, parent );
	
	return editor;
}

ParameterBase* ParameterItemDelegate
  ::getParam( const QModelIndex& index )
{
	// Assume 1:1 mapping between row and parameter index
	ParameterBase* param = getParam( index.row() );
	
	// Sanity check on parameter type
	if( param->type() != m_myParamType )
	{
		qDebug() << "Mismatching parameter type: " 
		         << tr("expected type %1, found type %2")
			        .arg(m_myParamType).arg(param->type());
		return;
	}
}

void ParameterItemDelegate
  ::paint( QPainter *painter, const QStyleOptionViewItem &option,
           const QModelIndex &index ) const
{
	// Draw default style (serves as fallback and/or background)
	QStyledItemDelegate::paint( painter, option, index );		
	
	ParameterBase* param = getParam( index );
	
	QStyle* style = QApplication::style();
	
	// Specialize style for column 1, containing our editable property values
	if( index.column() == 1 )
		paintParam( painter, option, index, style, param );
}

void BoolParameterItemDelegate
  ::paintParam( QPainter *painter, const QStyleOptionViewItem &option,
           const QModelIndex &index, QStyle* style, ParameterBase* param ) const
{
	painter->save();
	
	// Get parameter
	bool checked = (bool)index.model()->data(index, Qt::EditRole).toInt();	

	// Configure checkbox
	QStyleOptionButton butOpt;
	butOpt.state = QStyle::State_Enabled;
	butOpt.state = checked ? QStyle::State_On : QStyle::State_Off;
	butOpt.rect = option.rect;

	// Draw checkbox
	style->drawControl( QStyle::CE_CheckBox, &butOpt, painter );

	painter->restore();	
}

void BoolParameterItemDelegate
  ::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
	int value = index.model()->data(index, Qt::EditRole).toInt();
	QCheckBox* checkBox = static_cast<QCheckBox*>(editor);
	checkBox->setChecked( value!=0 );	
}

void BoolParameterItemDelegate
  ::setModelData( QWidget *editor, QAbstractItemModel *model,
				  const QModelIndex &index ) const
{
	QCheckBox* chbx = static_cast<QCheckBox*>(editor);

	BoolParameter* p_bool = dynamic_cast<BoolParameter*>(param);
	if( p_bool )
	{
		int value = (int)chbx->isChecked();
		model->setData( index, value, Qt::EditRole );
	}	
}

QWidget* BoolParameterItemDelegate
  ::getParameterEditor( ParameterBase* p, QWidget* parent ) const
{
	BoolParameter* p_bool = dynamic_cast<BoolParameter*>( p );
	if( p_bool )
	{
		QCheckBox* chkbx = new QCheckBox(parent);
		return chkbx;
	}
	
	return NULL;
}
