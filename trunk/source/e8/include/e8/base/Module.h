#ifndef MODULE_H
#define MODULE_H

#include "param/ParameterBase.h"
#include <string>

typedef std::vector<ParameterList> ParameterPresets;

/// Base class for all e8 modules providing support for parameter presets.
class Module
{
	// instance count currently only used for default naming
	static int s_moduleCount;
	
public:	
	Module( std::string name="" );
	
	std::string getModuleName() const { return m_name; }
	
	// non-const access
	ParameterList&    paramlist() { return m_params; }
	ParameterPresets& presets()   { return m_presets; }
	
	void addPreset( const ParameterList& plist ) { m_presets.push_back(plist); }
	
	// const access
	const ParameterPresets& getPresets()   const { return m_presets; }
	const ParameterList&    getParamList() const { return m_params; }
	
protected:
	void setModuleName( std::string name ) { m_name = name; }
	
private:
	std::string      m_name;
	ParameterList    m_params;   // currently active parameter set
	ParameterPresets m_presets;  // parameter presets
};

#endif // MODULE_H
