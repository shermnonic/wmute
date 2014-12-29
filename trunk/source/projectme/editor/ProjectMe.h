#ifndef PROJECTME_H
#define PROJECTME_H

#include "RenderSet.h"

/**
	\class ProjectMe
	
	ProjectMe encapsulates ModuleManager and RenderSetManager and provides IO
	via the Serializable interface.	It does manage (own) the manager instances.
*/
class ProjectMe : public Serializable
{
public:
	/// @name Serialization
	///@{
	PropertyTree& serialize() const;
	void deserialize( Serializable::PropertyTree& pt );
	///@}

	ModuleManager&    moduleManager   () { return m_moduleManager; }
	RenderSetManager& renderSetManager() { return m_renderSetManager; }

	void clear();

private:
	ModuleManager     m_moduleManager;
	RenderSetManager  m_renderSetManager;
};

#endif // PROJECTME_H
