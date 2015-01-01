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
	typedef std::vector<Connection> Connections;

	/// @name Serialization
	///@{
	PropertyTree& serialize() const;
	void deserialize( Serializable::PropertyTree& pt );
	///@}

	void clear();
	
	ModuleManager&    moduleManager   () { return m_moduleManager; }
	RenderSetManager& renderSetManager() { return m_renderSetManager; }
	RenderSet*        activeRenderSet() { return m_renderSetManager.getActiveRenderSet(); }
	Connections&      connections() { return m_connections; }

	void addConnection( ModuleRenderer* src, ModuleRenderer* dst, int channel );
	void delConnection( ModuleRenderer* src, ModuleRenderer* dst, int channel );

	void touchConnections();

	ModuleRenderer* moduleFromTarget( int texid );
	ModuleRenderer* moduleFromNameAndType( std::string name, std::string type );

private:
	ModuleManager    m_moduleManager;
	RenderSetManager m_renderSetManager;
	Connections      m_connections;
};

#endif // PROJECTME_H
