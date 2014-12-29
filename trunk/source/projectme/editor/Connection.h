#ifndef CONNECTION_H
#define CONNECTION_H
#include "RenderSet.h" // for ModuleRenderer

/**
	\class Connection
	
	Connect a ModuleRenderer target (output) to a channel (input) or
	a RenderArea.
*/
class Connection : public Serializable
{
public:	
	/// We connect from a ModuleRenderer target.
	struct Source
	{			
		ModuleRenderer* module;		
		Source(): module(0) {}
		Source(ModuleRenderer* m): module(m) {}
		bool valid() const { return module!=NULL; }
	};
	
	/// We either connect to a ModuleRenderer channel or a RenderArea.
	struct Destination
	{
		enum Type { TypeNone, TypeModuleRenderer, TypeRenderArea };		
		int             type;

		ModuleRenderer* module;
		int             channel;		
		
		RenderSet*      set;
		int             area;
		
		Destination(): type(TypeNone), module(0), channel(-1), set(0), area(-1) {}
		Destination(ModuleRenderer* m, int ch)
			: type(TypeModuleRenderer), module(m), channel(ch), set(0), area(-1) 
			{}
		Destination(RenderSet* s, int area)
			: type(TypeRenderArea), module(0), channel(-1), set(s), area(area)
			{}
		
		bool valid() const 
			{ return type!=TypeNone && (module!=NULL || set!=NULL); }
	};
	
	Connection();
	
	void connect( ModuleRenderer* src, ModuleRenderer* dst, int channel )
	{
		m_src = Source(src);
		m_dst = Destination(dst,channel);
	}
	
	void connect( ModuleRenderer* src, RenderSet* dst, int area )
	{
		m_src = Source(src);
		m_dst = Destination(dst,area);
	}
	
	void disconnect() { m_src=Source(); m_dst=Destination(); }	
	
	Source&      source()      { return m_src; }
	Destination& destination() { return m_dst; }
	
	bool isConnected() const { return m_src.valid() && m_dst.valid(); }

	void update();
	
	/// @name Serialization (not implemented yet!)
	///@{
	PropertyTree& serialize() const 
	{ 
		static Serializable::PropertyTree cache;
		return cache; 
	}
	void deserialize( Serializable::PropertyTree& pt ) {}
	///@}

private:
	Source      m_src;
	Destination m_dst;
};

#endif // CONNECTION_H
