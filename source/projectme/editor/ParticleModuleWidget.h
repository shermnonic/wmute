#ifndef PARTICLEMODULEWIDGET_H
#define PARTICLEMODULEWIDGET_H
#include "ModuleWidget.h"

class ParticleModule;

class ParticleModuleWidget : public ModuleWidget
{
	Q_OBJECT
	
public:
	ParticleModuleWidget( QWidget* parent=0 );

protected:
	/// ModuleWidget implementation
	bool setModuleInternal( ModuleBase* master );

	void setupGUI();

private:	
	ParticleModule* m_particleModule;
};

#endif // PARTICLEMODULEWIDGET_H
