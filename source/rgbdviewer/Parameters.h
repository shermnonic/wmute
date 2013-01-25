#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <string>

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

#endif // PARAMETERS_H