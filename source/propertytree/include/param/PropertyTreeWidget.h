#ifndef PROPERTYTREEWIDGET_H
#define PROPERTYTREEWIDGET_H

#include <QWidget>
#include "ParameterBase.h"

/*
  For use in a shared library:
	#include <QtGlobal>
	#ifdef QPARAMS_LIB
	# define QPARAMS_EXPORT Q_DECL_EXPORT
	#else
	# define QPARAMS_EXPORT Q_DECL_IMPORT
	#endif
*/
#define QPARAMS_EXPORT

class QStandardItemModel;
class QStandardItem;
class PropertyTreeView;

/// Widget to show and edit a a \a ParameterList in a \a PropertyTreeView.
class QPARAMS_EXPORT PropertyTreeWidget : public QWidget
{
	//Q_OBJECT

public:
	PropertyTreeWidget( QWidget* parent=0 );

	void setParameters( ParameterList* params );

protected:
	QStandardItem* getParameterValueItem( ParameterBase* p );

private:
	QStandardItemModel* m_model;	
	PropertyTreeView*   m_view;
	ParameterList*      m_params;	
};

#endif // PROPERTYTREEWIDGET_H
