#ifndef MODULEWIDGETFACTORY_H
#define MODULEWIDGETFACTORY_H

#include "ModuleWidget.h"
#include <string>
#include <vector>
#include <map>

#define MODULEWIDGETFACTORY_REGISTER( CLASSNAME, TYPENAME ) \
	namespace { \
		QWidget* create_CLASSNAME() { return new CLASSNAME; }  \
		const bool registered = ModuleWidgetFactory::          \
			ref().registerClass( TYPENAME, create_CLASSNAME ); \
	} 

//------------------------------------------------------------------------------
//	ModuleWidgetFactory
//------------------------------------------------------------------------------
/**
	\class ModuleWidgetFactory
	
	Manage available module widgets. See also \a ModuleWidget.
*/
class ModuleWidgetFactory
{
public:
	/// Return singleton instance
	static ModuleWidgetFactory& ref()
	{
		static ModuleWidgetFactory singleton;
		return singleton;
	}

	/// Callback type
	typedef ModuleWidget* (*CreateInstanceCallback)();

	/// Return a module object for a particular type name.
	/// Returns NULL if type name is not supported.
	ModuleWidget* createInstance( std::string moduleType );

	/// Register new module class under specific type name.
	/// Returns true if registration was succesfull.
	bool registerClass( std::string ext, CreateInstanceCallback cb );
	
	typedef std::vector<std::string> ModuleTypeList;
	/// Return list of available (registered) module type names.
	ModuleTypeList getAvailableModuleWidgets() const;

private:
	typedef std::map<std::string, CreateInstanceCallback> CallbackMap;
	CallbackMap m_callbacks;

	// make c'tors private for singleton
	ModuleWidgetFactory() {}
	~ModuleWidgetFactory() {}
	ModuleWidgetFactory( const ModuleWidgetFactory& ) {}
	ModuleWidgetFactory& operator = ( const ModuleWidgetFactory& ) { return *this; }
};

#endif // MODULEWIDGETFACTORY_H
