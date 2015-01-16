#pragma once
#ifndef PARAMETER_H
#define PARAMETER_H

#include <string>
#include <sstream> // for default value to string conversion
#include <vector>
//#include <map> // for factory
#include <boost/property_tree/ptree.hpp> // for serialization
#include <boost/optional.hpp>
#include <cassert>

typedef boost::property_tree::ptree PTree;

/**\verbatim

	Parameter system
	================
	Max Hermann, September 2013
	
	This is a single header adaption of the original library.
	See ParameterExample class at the end for usage demonstration.

	Class hierarchy
	---------------
	ParameterBase               // Abstract base class provides key(), type()
	+- ParameterBaseDefault<T>  // Provides value(), defaultValue()
	   +- NumericParameter<T>   // Provides limits()
	   |  |                     // Classes below are defined in ParameterTypes.h
	   |  +- DoubleParameter
	   |  +- IntParameter
	   |     +- BoolParameter
	   |     +- EnumParameter
	   +- StringParameter

	Requirements / Features
	-----------------------
	(Marked with + are already implemented features)
	+ Different data types
	+ Default value and range (if applicable)
	+ Description
	+ Serialization (boost::property_tree)
	- Interpolation
	- Easy adaptor system to integrate complex types of different libraries
	- Automatic UI generation, extendable for custom types (Qt)
	- Compound parameter types?
	- Reference semantics (m_value as reference to member variable) and / or
	  some kind of update mechanism (visitor, callback?).

	Serialization
	-------------
	Serialization is realized via boost::property_tree. Each class has to
	provide an implementation of \a read() and \a write() responsible to
	serialize its own members. Note that in this design the derived class is 
	responsible to invoke the serialization functions of its super class.

	Notes
	-----
	Further thoughts:
	- UI implementations for custom types could be realized via a factory 
	  pattern.
	- Serialization could be realized via strategy pattern.
	- Adding value[To|From]String() functions to ParameterBase would enable
	  canonic GUI treatment. Though, in the end we want to use some extendable
	  factory solution for custom Qt delegates.
	- Hierarchy and parameter groups could be realized via naming convention,
	  e.g. a slash ('/') could separate levels of an (acyclic) tree. So far I
	  see no advantage in reflecting a parameter hierarchy in our storage class
	  and would argue that the exisiting ParameterList concept is sufficient.	  

	Future extensions:
	- Support for change history?
	  
	For inspiration see also:
	- Parameter management article and code by Qinghai Zhang:
	  "C++ Design Patterns for Managing Parameters in Scientific Computing."
	  http://www.math.utah.edu/~tsinghai/papers/parameterPatterns.pdf	  

\endverbatim
*/

//-----------------------------------------------------------------------------
// --- Parameter base classes ---
//-----------------------------------------------------------------------------

class ParameterBase;

/// List of pointers to parameter instances.
class ParameterList : public std::vector<ParameterBase*>
{
public:
	/// Access parameter by key, returns NULL if no matching parameter found
	ParameterBase* get_param( std::string key );

#if 0 // NOT YET!
	/// @name Serialization
	///@{
	virtual void write( PTree& pt ) const;
	virtual void read ( const PTree& pt );
	///@}
#endif
};

/// Equality operator based on key comparison.
/// Note that ParameterList has a custom function get_param(key).
bool operator == ( const ParameterBase& a, const ParameterBase& b );


//-----------------------------------------------------------------------------
//	ParameterBase
//-----------------------------------------------------------------------------
/// Abstract parameter base class, providing key and type semantics.
/// Value semantics is not included here but realized in a template child 
/// class. This serves a template free super class for all parameter types.
class ParameterBase // AbstractParameter ?
{
public:
	ParameterBase( const std::string& key );

	// Key and type semantics

	std::string         key()  const;
	virtual std::string type() const;

	// Equality operator (via key comparison, defined globally)

	friend bool operator == ( const ParameterBase& a, const ParameterBase& b );

	// Serialization

	virtual void write( PTree& pt ) const;
	virtual void read ( const PTree& pt );

	/// Return string representation of current value
	virtual std::string str() const;

	/// Reset to default value (implemented in subclass ParameterBaseDefault)
	virtual void reset() { assert(false); /* should never be executed */ };

private:
	std::string m_key;
	std::string m_type;
};

//-----------------------------------------------------------------------------
//	ParameterBaseDefault< T >
//-----------------------------------------------------------------------------

//#define PARAMETERBASE_DEFAULT_CTORS( NAME, TYPE, TYPENAME )   \	
//	NAME( const std::string& key )                            \
//		: ParameterBaseDefault( key )                         \
//	{}                                                        \
//	                                                          \
//	NAME( const std::string& key, TYPE value_and_default )    \
//		: ParameterBaseDefault( key, value_and_default )      \
//	{}                                                        \
//	                                                          \
//	NAME( const std::string& key, TYPE value, TYPE default_ ) \
//		: ParameterBaseDefault( key, value, default_ )        \
//	{}                                                        \
//	virtual std::string type() const { return TYPENAME; }

/// Templated base parameter class providing value and default value semantics.
template<class T> // ParameterBase
class ParameterBaseDefault: public ParameterBase
{
public:
	ParameterBaseDefault( const std::string& key )
		: ParameterBase( key ),
		  m_value  ( T() ), // Initialize w/ default c'tor
		  m_default( T() )
	{}

	ParameterBaseDefault( const std::string& key, T value_and_default_ )
		: ParameterBase( key ),
		  m_value  ( value_and_default_ ),
		  m_default( value_and_default_ )
	{}

	ParameterBaseDefault( const std::string& key, T value, T default_ )
		: ParameterBase( key ),
		  m_value  ( value ),
		  m_default( default_ )
	{}

	// Default value semantics

	void setValueAndDefault( const T& val )
	{
		setValue( val );
		m_default = val;
	}

	void setDefault( const T& val )
	{
		m_default = val;
	}

	T defaultValue() const  // FIXME: default is keyword in C++11
	{ 
		return m_default; 
	}

	void reset()
	{
		m_value = m_default;
	}

	// Value semantics

	void setValue( const T& val )
	{
		m_value = val;
	}

	const T& value() const
	{
		return m_value;
	}	
	
	T& valueRef()
	{
		return m_value;
	}

	// Default serialization (FIXME: How to handle non-basic types?)

	virtual void write( PTree& pt ) const
	{
		ParameterBase::write( pt ); // call super
		pt.put( "value"  ,value() );
		pt.put( "default",defaultValue() );
	}

	virtual void read( const PTree& pt )
	{
		ParameterBase::read( pt ); // call super
		m_value   = pt.get<T>( "value" );
		m_default = pt.get<T>( "default" );
	}

	// Default value to string conversion

	std::string str() const
	{
		std::stringstream ss;
		ss << m_value;
		return ss.str();
	}

private:
	T m_value;
	T m_default;
};

//-----------------------------------------------------------------------------
//	NumericParameter< T >
//-----------------------------------------------------------------------------

#define PARAMETERBASE_NUMERIC_PARAM( NAME, TYPE, TYPENAME )   \
	public:                                                   \
	NAME( const std::string& key )                            \
	 	: NumericParameter( key )                             \
	{}                                                        \
	NAME( const std::string& key, TYPE value_ )               \
		: NumericParameter( key, value_ )                     \
	{}                                                        \
	NAME( const std::string& key,                             \
	                  TYPE value_, TYPE min_, TYPE max_ )     \
		: NumericParameter( key, value_, min_, max_ )         \
	{}                                                        \
	NAME( const std::string& key,                             \
	                  TYPE value_, TYPE min_, TYPE max_,      \
	                  TYPE default_ )                         \
		: NumericParameter( key, value_, min_, max_, default_ ) \
	{}                                                        \
	virtual std::string type() const { return TYPENAME; }

/// Base class for all numeric parameters, providing min/max range semantic.
template<class T>
class NumericParameter: public ParameterBaseDefault<T>
{
public:
	typedef ParameterBaseDefault<T> Super;

	NumericParameter( const std::string& key )
		: Super( key )		  
	{}

	NumericParameter( const std::string& key, T value_and_default )
		: Super( key, value_and_default, value_and_default )
	{}

	NumericParameter( const std::string& key, T value_and_default, T min_, T max_ )
		: Super( key, value_and_default, value_and_default ),
		  m_limits( min_, max_ )
	{}

	NumericParameter( const std::string& key, T value_, T min_, T max_,
	                  T default_ )
		: Super( key, value_, default_ ),
		  m_limits( min_, max_ )
	{}

	struct Range { 
		Range()
			: min_(-std::numeric_limits<T>::max()), // ::lowest() in C++11
			  max_( std::numeric_limits<T>::max()),
			  active(false)
		{}
		
		Range( const T& min__, const T& max__ )
			: min_(min__),
			  max_(max__),
			  active(true)
		{}

		T min_, max_; 
		bool active;

		T clamp( const T& val ) const 
		{ 
			return (val > max_) ? max_ : ((val < min_) ? min_ : val); 
		}
	};

	// Value range limits semantics

	void setLimits( const T& min_, const T& max_ )
	{
		m_limits = Range( min_, max_ );
	}

	void setLimits( const Range& limits )
	{
		m_limits = limits;
	}

	Range limits() const
	{
		return m_limits;
	}

	// Serialization

	virtual void write( PTree& pt ) const
	{
		Super::write( pt ); // call super
		// Do not store limits if they are not used (i.e. initialized)
		if( m_limits.active ) {
			pt.put( "limit_min", limits().min_ );
			pt.put( "limit_max", limits().max_ );
		}
	}

	virtual void read( const PTree& pt )
	{
		Super::read( pt ); // call super
		boost::optional<T> min_ = pt.get_optional<T>( "limit_min" );
		boost::optional<T> max_ = pt.get_optional<T>( "limit_max" );
		// Only set limits if *both* min and max are provided
		if( min_ && max_ )
			setLimits( Range(min_.get(),max_.get()) );		
	}

private:
	Range m_limits;
};


//-----------------------------------------------------------------------------
// --- Parameter factory ---  (NOT YET!)
//-----------------------------------------------------------------------------
/*
class ParameterFactory
{
public:
	/// Return singleton instance
	static ParameterFactory& ref()
	{
		static ParameterFactory singleton;
		return singleton;
	}

	typedef ParameterBase* (*CreateParameterCallback)();

	/// Returns parameter instance for given type string (e.g. "double")
	/// Returns NULL if type string is not supported.
	ParameterBase* create_Parameter( const boost::property_tree::ptree& pt );

	/// Register new parameter class for specific type string
	/// Returns true if registration was succesful
	bool register_format( std::string type, CreateParameterCallback cb );

private:

	typedef std::map<std::string, ParameterFactory> CallbackMap;
	CallbackMap m_callbacks;

	// make c'tors private for singleton
	ParameterFactory() {}
	~ParameterFactory() {}
	ParameterFactory( const ParameterFactory& ) {}
	ParameterFactory& operator = ( const ParameterFactory& ) { return *this; }
};
*/



//-----------------------------------------------------------------------------
// --- Concrete parameter types ---
//-----------------------------------------------------------------------------

/// A double parameter.
class DoubleParameter: public NumericParameter<double>
{
	PARAMETERBASE_NUMERIC_PARAM( DoubleParameter, double, "double" )
};

/// An integer parameter.
class IntParameter: public NumericParameter<int>
{
	PARAMETERBASE_NUMERIC_PARAM( IntParameter, int, "int" )
};

/// A string parameter.
class StringParameter: public ParameterBaseDefault<std::string>
{
	//PARAMETERBASE_DEFAULT_CTORS( StringParameter, std::string, "string" )
public:
	StringParameter( const std::string& key )                            
		: ParameterBaseDefault( key )                         
	{}                                                        
	                                                          
	StringParameter( const std::string& key, std::string value_and_default )    
		: ParameterBaseDefault( key, value_and_default )      
	{}                                                        
	                                                          
	StringParameter( const std::string& key, std::string value, std::string default_ ) 
		: ParameterBaseDefault( key, value, default_ )        
	{}        

	virtual std::string type() const { return "string"; }
};

/// A boolean parameter, realized two-value limited integer parameter.
class BoolParameter: public IntParameter
{
public:
	BoolParameter( const std::string& key )
		: IntParameter( key, (int)true, 0,1 )
	{}

	BoolParameter( const std::string& key, bool value_and_default )
		: IntParameter( key, value_and_default, 0,1 )
	{}

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

	EnumParameter( const std::string& key, int value_and_default, 
		           std::vector<std::string> enumNames )
		: IntParameter( key, value_and_default ),
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




//-----------------------------------------------------------------------------
// --- Example ---
//-----------------------------------------------------------------------------

/// Example usage of Parameter classes
struct ParameterExample
{
	ParameterExample()
		: d0( "d0" ),
		  d1( "d1" ),
		  index( "index" ),
		  title( "title" ),
		  mode( "mode", "NoMode", "FooMode", "BarMode", "FoobarMode" ),
		  doit( "doit" )
	{
		parms.push_back( &d0 );
		parms.push_back( &d1 );
		parms.push_back( &index );
		parms.push_back( &title );
		parms.push_back( &mode );
		parms.push_back( &doit );

		d0.setValue( 42.3 ); d0.setLimits(0.0,100.0);
		d1.setValue( 23.7 ); d1.setLimits(-50.0,50.0);
		index.setValue( 77 ); index.setLimits(0,1000);
		title.setValue( "Foobar" );
		mode.setValue( 2 );
		doit.setValue( true );
	}

	DoubleParameter d0;
	DoubleParameter d1;
	IntParameter    index;
	StringParameter title;
	EnumParameter   mode;
	BoolParameter   doit;

	ParameterList   parms;
};

#endif // PARAMETER_H
