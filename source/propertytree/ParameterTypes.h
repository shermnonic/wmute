#ifndef PARAMETERTYPES_H
#define PARAMETERTYPES_H

#include "ParameterBase.h"
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
// --- Concrete parameter types ---
//-----------------------------------------------------------------------------

/// A double parameter.
class DoubleParameter: public NumericParameter<double>
{
public:
	DoubleParameter( const std::string& key )
		: NumericParameter( key )
	{}

	std::string type() const { return "double"; };
};

/// An integer parameter.
class IntParameter: public NumericParameter<int>
{
public:
	IntParameter( const std::string& key )
		: NumericParameter( key )
	{}

	virtual std::string type() const { return "int"; };
};

/// A string parameter.
class StringParameter: public ParameterBaseDefault<std::string>
{
public:
	StringParameter( const std::string& key )
		: ParameterBaseDefault( key )
	{}

	std::string type() const { return "string"; };
};

/// A boolean parameter, realized two-value limited integer parameter.
class BoolParameter: public IntParameter
{
public:
	BoolParameter( const std::string& key )
		: IntParameter( key )
	{
		setLimits( 0, 1 );
	}

	std::string type() const { return "bool"; };

protected:
	// Override limit functions and make them inaccessible from outside
	void setLimits( const int& min_, const int& max_ )
	{
		IntParameter::setLimits( min_, max_ );
	}
	void setLimits( const Range& limits )
	{
		IntParameter::setLimits( limits );
	}
};

/// An enum parameter, realized as a named and range limited integer parameter.
class EnumParameter: public IntParameter
{
public:
	EnumParameter( const std::string& key, std::vector<std::string> enumNames )
		: IntParameter( key ),
		  m_enumNames( enumNames )
	{
		setLimits( 0, (int)enumNames.size() );
	}

	// vector<> initializers are quite inconvenient, therefore we provide some
	// unrolled explicit initializer lists

	EnumParameter( const std::string& key, std::string enumName0 )
		: IntParameter( key )
	{
		m_enumNames.push_back( enumName0 );
		setLimits( 0, (int)m_enumNames.size() );
	}

	EnumParameter( const std::string& key, std::string enumName0, std::string enumName1 )
		: IntParameter( key )
	{
		m_enumNames.push_back( enumName0 );
		m_enumNames.push_back( enumName1 );
		setLimits( 0, (int)m_enumNames.size() );
	}

	EnumParameter( const std::string& key, std::string enumName0, std::string enumName1, std::string enumName2 )
		: IntParameter( key )
	{
		m_enumNames.push_back( enumName0 );
		m_enumNames.push_back( enumName1 );
		m_enumNames.push_back( enumName2 );
		setLimits( 0, (int)m_enumNames.size() );
	}

	EnumParameter( const std::string& key, std::string enumName0, std::string enumName1, std::string enumName2, std::string enumName3 )
		: IntParameter( key )
	{
		m_enumNames.push_back( enumName0 );
		m_enumNames.push_back( enumName1 );
		m_enumNames.push_back( enumName2 );
		m_enumNames.push_back( enumName3 );
		setLimits( 0, (int)m_enumNames.size() );
	}

	EnumParameter( const std::string& key, std::string enumName0, std::string enumName1, std::string enumName2, std::string enumName3, std::string enumName4 )
		: IntParameter( key )
	{
		m_enumNames.push_back( enumName0 );
		m_enumNames.push_back( enumName1 );
		m_enumNames.push_back( enumName2 );
		m_enumNames.push_back( enumName3 );
		m_enumNames.push_back( enumName4 );
		setLimits( 0, (int)m_enumNames.size() );
	}

	EnumParameter( const std::string& key, std::string enumName0, std::string enumName1, std::string enumName2, std::string enumName3, std::string enumName4, std::string enumName5 )
		: IntParameter( key )
	{
		m_enumNames.push_back( enumName0 );
		m_enumNames.push_back( enumName1 );
		m_enumNames.push_back( enumName2 );
		m_enumNames.push_back( enumName3 );
		m_enumNames.push_back( enumName4 );
		m_enumNames.push_back( enumName5 );
		setLimits( 0, (int)m_enumNames.size() );
	}

	EnumParameter( const std::string& key, std::string enumName0, std::string enumName1, std::string enumName2, std::string enumName3, std::string enumName4, std::string enumName5, std::string enumName6 )
		: IntParameter( key )
	{
		m_enumNames.push_back( enumName0 );
		m_enumNames.push_back( enumName1 );
		m_enumNames.push_back( enumName2 );
		m_enumNames.push_back( enumName3 );
		m_enumNames.push_back( enumName4 );
		m_enumNames.push_back( enumName5 );
		m_enumNames.push_back( enumName6 );
		setLimits( 0, (int)m_enumNames.size() );
	}

	std::string type() const { return "enum"; };

	const std::vector<std::string>& enumNames() const { return m_enumNames; }

protected:
	// Override limit functions and make them inaccessible from outside
	void setLimits( const int& min_, const int& max_ )
	{
		IntParameter::setLimits( min_, max_ );
	}
	void setLimits( const Range& limits )
	{
		IntParameter::setLimits( limits );
	}

private:
	std::vector<std::string> m_enumNames;
};

#endif // PARAMETERTYPES_H
