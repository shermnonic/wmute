#ifndef MODULEFACTORY_H
#define MODULEFACTORY_H

#include "RenderSet.h" // for ModuleBase
#include <string>
#include <vector>
#include <map>

#define MODULEFACTORY_REGISTER( CLASSNAME, TYPENAME ) \
	namespace { \
		ModuleBase* create_CLASSNAME() { return new CLASSNAME; } \
		const bool registered = ModuleFactory::ref().registerClass( TYPENAME, create_CLASSNAME ); \
	} 

//------------------------------------------------------------------------------
//	ModuleFactory
//------------------------------------------------------------------------------
/**
	\class ModuleFactory
	
	Manage available modules.
*/
class ModuleFactory
{
public:
	/// Return singleton instance
	static ModuleFactory& ref()
	{
		static ModuleFactory singleton;
		return singleton;
	}

	/// Callback type
	typedef ModuleBase* (*CreateModuleCallback)();

	/// Return a module object for a particular type name.
	/// Returns NULL if type name is not supported.
	ModuleBase* createInstance( std::string moduleType );

	/// Register new module class under specific type name.
	/// Returns true if registration was succesfull.
	bool registerClass( std::string ext, CreateModuleCallback cb );
	
	typedef std::vector<std::string> ModuleTypeList;
	/// Return list of available (registered) module type names.
	ModuleTypeList getAvailableModules() const;

private:
	typedef std::map<std::string, CreateModuleCallback> CallbackMap;
	CallbackMap m_callbacks;

	// make c'tors private for singleton
	ModuleFactory() {}
	~ModuleFactory() {}
	ModuleFactory( const ModuleFactory& ) {}
	ModuleFactory& operator = ( const ModuleFactory& ) { return *this; }
};

#endif // MODULEFACTORY_H
