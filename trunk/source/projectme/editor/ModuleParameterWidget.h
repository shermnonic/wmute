#ifndef MODULEPARAMETERWIDGET_H
#define MODULEPARAMETERWIDGET_H

#include <QWidget>

// Forwards
class ModuleBase;
class ParameterList;
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

protected slots:
	void onApplyOptions();

protected:
	void clear();
	void setupParameters( ParameterList& , QAutoGUI::Parameters*, QAutoGUI::ParametersWidget* );

private:
	ModuleBase* m_master;
	QAutoGUI::ParametersWidget* m_paramWidget;
	QAutoGUI::ParametersWidget* m_optsWidget;
	QAutoGUI::Parameters* m_parameters;
	QAutoGUI::Parameters* m_options;
};

#endif // MODULEPARAMETERWIDGET_H
