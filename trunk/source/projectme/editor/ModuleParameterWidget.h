#ifndef MODULEPARAMETERWIDGET_H
#define MODULEPARAMETERWIDGET_H

#include <QWidget>

// Forwards
class ModuleBase;
namespace QAutoGUI {
	class Parameters;
	class ParametersWidget;
};

/**
	\class ModuleParameterWidget
	
	Generic parameter editing widget based on \a Parameter and \a QAutoGUI.
*/
class ModuleParameterWidget : public QWidget
{
	Q_OBJECT

public:
	ModuleParameterWidget( QWidget* parent=0 );

	ModuleBase* module() { return m_master; }
	const ModuleBase* module() const { return m_master; }

public slots:
	void setModule( ModuleBase* master );	

private:
	ModuleBase* m_master;
	QAutoGUI::ParametersWidget* m_widget;
	QAutoGUI::Parameters* m_parameters;
};

#endif // MODULEPARAMETERWIDGET_H
