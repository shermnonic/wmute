#ifndef PARAMETERBASE_H
#define PARAMETERBASE_H

#include <string>
//#include <map> // for factory
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>

/**\verbatim

	Parameter system
	================
	Max Hermann, September 2013

	Class hierarchy
	---------------
	ParameterBase               // Abstract base class provides key(), type()
	+- ParameterBaseDefault<T>  // Provides value(), default()
	   +- NumericParameter<T>   // Provides limits()
	   |  |                     // Classes below are defined in ParameterTypes.h
	   |  +- DoubleParameter
	   |  +- IntParameter
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
	serialize its own members. Note that in this design the derived class to 
	invoke the serialization functions of its super class.

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
typedef std::vector<ParameterBase*> ParameterList;

//-----------------------------------------------------------------------------
//	ParameterBase
//-----------------------------------------------------------------------------
/// Abstract parameter base class, providing key and type semantics.
/// Value semantics is not included here but realized in a template child 
/// class. This serves a template free super class for all parameter types.
class ParameterBase // AbstractParameter ?
{
public:
	ParameterBase( const std::string& key )
		: m_key(key)
	{
		m_type = type();
	}

	// Key and type semantics

	std::string         key()  const { return m_key; }
	virtual std::string type() const { return m_type; } //=0;

	// Serialization

	typedef boost::property_tree::ptree PTree;

	virtual void write( PTree& pt ) const
	{
		pt.put( "key" , key() );
		pt.put( "type", type() );
	}

	virtual void read ( const PTree& pt )
	{
		m_key  = pt.get<std::string>( "key" );		
		m_type = pt.get<std::string>( "type" );
	}

private:
	std::string m_key;
	std::string m_type;
};

//-----------------------------------------------------------------------------
//	ParameterBaseDefault< T >
//-----------------------------------------------------------------------------
/// Templated base parameter class providing value and default value semantics.
template<class T> // ParameterBase
class ParameterBaseDefault: public ParameterBase
{
public:
	ParameterBaseDefault( const std::string& key )
		: ParameterBase( key )
	{
		// Initialize w/ default c'tor
		m_default = m_value = T();
	}

	// Default value semantics

	void setDefault( const T& val )
	{
		m_default = val;
	}

	T default() const  // FIXME: default is keyword in C++11
	{ 
		return m_default; 
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

	// Default serialization (FIXME: How to handle non-basic types?)

	virtual void write( PTree& pt ) const
	{
		ParameterBase::write( pt ); // call super
		pt.put( "value"  ,value() );
		pt.put( "default",default() );
	}

	virtual void read( const PTree& pt )
	{
		ParameterBase::read( pt ); // call super
		m_value   = pt.get<T>( "value" );
		m_default = pt.get<T>( "default" );
	}

private:
	T m_value;
	T m_default;
};

//-----------------------------------------------------------------------------
//	NumericParameter< T >
//-----------------------------------------------------------------------------
/// Base class for all numeric parameters, providing min/max range semantic.
template<class T>
class NumericParameter: public ParameterBaseDefault<T>
{
public:
	NumericParameter( const std::string& key )
		: ParameterBaseDefault( key )		  
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
		ParameterBaseDefault::write( pt ); // call super
		// Do not store limits if they are not used (i.e. initialized)
		if( m_limits.active ) {
			pt.put( "limit_min", limits().min_ );
			pt.put( "limit_max", limits().max_ );
		}
	}

	virtual void read( const PTree& pt )
	{
		ParameterBaseDefault::read( pt ); // call super
		boost::optional<T> min_ = pt.get_optional<T>( "limits_min" );
		boost::optional<T> max_ = pt.get_optional<T>( "limits_max" );
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

#endif // PARAMETERBASE_H
