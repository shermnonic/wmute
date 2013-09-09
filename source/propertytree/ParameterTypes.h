#ifndef PARAMETERTYPES_H
#define PARAMETERTYPES_H

#include "ParameterBase.h"

//-----------------------------------------------------------------------------
// --- Concrete parameter types ---
//-----------------------------------------------------------------------------

class DoubleParameter: public NumericParameter<double>
{
public:
	DoubleParameter( const std::string& key )
		: NumericParameter( key )
	{}

	std::string type() const { return "double"; };
};

class IntParameter: public NumericParameter<int>
{
public:
	IntParameter( const std::string& key )
		: NumericParameter( key )
	{}

	std::string type() const { return "int"; };
};

class StringParameter: public ParameterBaseDefault<std::string>
{
public:
	StringParameter( const std::string& key )
		: ParameterBaseDefault( key )
	{}

	std::string type() const { return "string"; };
};

#endif // PARAMETERTYPES_H
