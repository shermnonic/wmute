#ifndef MODULEWIDGET_H
#define MODULEWIDGET_H

#include <QWidget>
//#include "RenderSet.h" // for ModuleBase
class ModuleBase;

//------------------------------------------------------------------------------
//	ModuleWidget
//------------------------------------------------------------------------------
/**
	\class ModuleWidget
	
	Base class for module control widgets. See also \a ModuleWidgetFactory.
	
	The function setModule() is called to set the master module which is 
	modified through the user interface provided by the particular widget 
	implementation. Subclasses implement setModuleInternal().
*/
class ModuleWidget : public QWidget
{
	Q_OBJECT
	
signals:
	void moduleParametersChanged();
	
public:
	ModuleWidget( QWidget* parent=0 ): QWidget(parent) {}		
		
	ModuleBase* module() { return m_master; }
	const ModuleBase* module() const { return m_master; }
	
public slots:
	void setModule( ModuleBase* master )
	{
		if( setModuleInternal( master ) )
			m_module = master;
		else
			m_module = NULL;
	}	

protected:
	// Returns false if module could not be set, e.g. when type mismatches.
	virtual bool setModuleInternal( ModuleBase* master ) { return false; }

private:
	ModuleBase* m_master;
};

#endif // MODULEWIDGET_H
