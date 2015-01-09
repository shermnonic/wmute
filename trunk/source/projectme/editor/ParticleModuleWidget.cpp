#include "ParticleModuleWidget.h"
#include "ModuleWidgetFactory.h"
#include "ParticleModule.h"

MODULEWIDGETFACTORY_REGISTER( ParticleModuleWidget, "ParticleModule" )

ParticleModuleWidget
	::ParticleModuleWidget( QWidget* parent )
	: ModuleWidget( parent )
{	
}

bool ParticleModuleWidget
	::setModuleInternal( ModuleBase* master )
{
	ParticleModule* m = dynamic_cast<ParticleModule*>( master );
	if( !m )
	{
		m_particleModule = NULL;
		return false;
	}
	
	m_particleModule = m;
}

void ParticleModuleWidget
	::setupGUI()
{
	
}
