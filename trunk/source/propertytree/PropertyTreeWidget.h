#ifndef PROPERTYTREEWIDGET_H
#define PROPERTYTREEWIDGET_H

#include <QWidget>
#include "ParameterBase.h"

class QStandardItemModel;
class QStandardItem;

class PropertyTreeWidget : public QWidget
{
	//Q_OBJECT
public:
	PropertyTreeWidget( QWidget* parent=0 );

	void setParameters( ParameterList* params );

protected:
	QStandardItem* getParameterValueItem( ParameterBase* p );

private:
	QStandardItemModel* m_model;
	ParameterList* m_params;
};

#endif // PROPERTYTREEWIDGET_H
