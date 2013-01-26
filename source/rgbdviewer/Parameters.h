#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <string>
#include <vector>

/// Real valued parameter with name, description, min, max and default value
struct FloatParameter
{
	/// Default C'tor creates invalid parameter
	FloatParameter()
		: invalid(true)
		{}
	/// Explicit C'tor
	FloatParameter( std::string name_, std::string  desc_, 
		            float value_,
		            float defaultValue_, float min_, float max_ )
		: invalid(false),
		    value(value_),
			defaultValue(defaultValue_),
			min(min_),
			max(max_),
			name(name_),
			desc(desc_)
		{}
	bool invalid;
	float value, defaultValue, min, max;
	std::string name, desc;
};

// FIXME: Use some sort of smart pointer for the FloatParameterVector
typedef std::vector<FloatParameter*> FloatParameterVector;

#endif // PARAMETERS_H