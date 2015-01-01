#ifndef CONNECTION_H
#define CONNECTION_H
#include "RenderSet.h" // for ModuleRenderer

class ProjectMe;

/**
	\class Connection
	
	Connect a ModuleRenderer target (output) to a channel (input).
*/
class Connection : public Serializable
{
public:	
	/// Connections emanate from the single target of a  ModuleRenderer.
	struct Source
	{			
		ModuleRenderer* module;		
		Source(): module(0) {}
		Source(ModuleRenderer* m): module(m) {}
		bool valid() const { return module!=NULL; }
	};
	
	/// Connections end at a specific channel of a ModuleRenderer.
	struct Destination
	{
		ModuleRenderer* module;
		int             channel;		
		Destination(): module(0), channel(-1) {}
		Destination(ModuleRenderer* m, int ch)
			: module(m), channel(ch)
			{}		
		bool valid() const { return module!=NULL; }
	};

	Connection()
	{
		Serializable::setName("Connection");
	}
	
	void connect( ModuleRenderer* src, ModuleRenderer* dst, int channel )
	{
		m_src = Source(src);
		m_dst = Destination(dst,channel);
		update();
	}	
	
	void disconnect() { m_src=Source(); m_dst=Destination(); }	
	
	Source&      source()      { return m_src; }
	Destination& destination() { return m_dst; }
	
	bool isConnected() const { return m_src.valid() && m_dst.valid(); }

	void update();
	
	/// @name Serialization
	///@{
	PropertyTree& serialize() const;
	void deserialize( Serializable::PropertyTree& pt );
	///@}

	// ProjectMe instance required for deserialization (to resolve module pointers)
	void setProjectMe( ProjectMe* pm ) { m_projectMe = pm; }

private:
	Source      m_src;
	Destination m_dst;
	ProjectMe*  m_projectMe;
};

#endif // CONNECTION_H
