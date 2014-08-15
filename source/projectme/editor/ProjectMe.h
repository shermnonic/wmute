#ifndef PROJECTME_H
#define PROJECTME_H

#include "RenderSet.h"

class ProjectMe : public Serializable
{
public:
	/// @name Serialization
	///@{
	PropertyTree& serialize() const;
	void deserialize( Serializable::PropertyTree& pt );
	///@}

	/// WORKAROUND: The managers are currently owned externally!
	void setModuleManager( ModuleManager* mm ) { m_moduleManager = mm; }
	void setRenderSetManager( RenderSetManager* rsm ) { m_renderSetManager = rsm; }

private:
	ModuleManager*    m_moduleManager;
	RenderSetManager* m_renderSetManager;
};

#endif // PROJECTME_H
