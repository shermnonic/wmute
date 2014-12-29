#ifndef PROJECTME_H
#define PROJECTME_H

#include "RenderSet.h"
#include "Connection.h"
#include <vector>

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

	void clear();

	ModuleManager&    moduleManager   () { return m_moduleManager; }
	RenderSetManager& renderSetManager() { return m_renderSetManager; }

	void addConnection( ModuleRenderer* src, ModuleRenderer* dst, int channel );
	void delConnection( ModuleRenderer* src, ModuleRenderer* dst, int channel );

private:
	ModuleManager    m_moduleManager;
	RenderSetManager m_renderSetManager;
	std::vector<Connection> m_connections;
};

#endif // PROJECTME_H
