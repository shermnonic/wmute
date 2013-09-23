#include <iostream>
#include <vector>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/optional.hpp>
#include <boost/foreach.hpp>

/*
	Requirements:
	- Different data types
	- Default value and range (if applicable)
	- Description
	- Serialization (boost::property_tree)
	- Interpolation
	- Easy adaptor system to integrate complex types of different libraries
	- Automatic UI generation, extendable for custom types (Qt)
	- Compound parameter types?

	Default UI should be provided for the following (base) classes:
	- ParameterBaseDefault
	- ParameterNumericParameter

	Serialization is realized via boost::property_tree. Each class has to
	provide an implementation of \a read() and \a write() responsible to
	serialize its own members. Note that in this design the derived class to 
	invoke the serialization functions of its super class.

	Further thoughts:
	- UI implementations for custom types could be realized via a factory 
	  pattern.
	- Serialization could be realized via strategy pattern.
	- Reference semantics (m_value as reference to member variable) and / or
	  some kind of update mechanism (visitor, callback?).
	  
	For inspiration see also:
	- Parameter management article and code by Qinghai Zhang:
	  "C++ Design Patterns for Managing Parameters in Scientific Computing."
	  http://www.math.utah.edu/~tsinghai/papers/parameterPatterns.pdf	  
*/

// --- Parameter base classes ---

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

template<class T> // ParameterBase
class ParameterBaseDefault: public ParameterBase
{
public:
	ParameterBaseDefault( const std::string& key )
		: ParameterBase( key )
	{}

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

// --- Parameter factory ---
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
// --- Concrete parameter types ---

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

// --- Parameter IO ---

typedef std::vector<ParameterBase*> ParameterList;

void save_params( const char* filename, const ParameterList& parms )
{
	using boost::property_tree::ptree;	
	ptree root;

	ParameterList::const_iterator it = parms.begin();
	for( ; it != parms.end(); it++ )
	{
		ptree pt;
		(*it)->write( pt );		
		root.add_child( "ParameterList.Parameter", pt );
	}

	write_xml( filename, root );
}

// Load w/o factory to known ParameterList
void load_params( const char* filename, ParameterList& parms )
{
	using boost::property_tree::ptree;
	ptree root;
	
	read_xml( filename, root );

	// ParameterList's on disk and in parms must match!
	// Iterate simultaneously over both.
	ParameterList::iterator it = parms.begin();	

	BOOST_FOREACH( ptree::value_type &v, root.get_child("ParameterList") )
	{
		std::cout << v.second.get<std::string>("key")
			<< "(" << v.second.get<std::string>("type") << ")" << std::endl;
	}

	ptree pt = root.get_child("ParameterList");
	boost::property_tree::ptree::iterator pit = pt.begin();	
	for( ; it != parms.end() && pit != pt.end(); it++, pit++ )
	{			
		(*it)->read( pit->second );
	}	

	//boost::property_tree::ptree::iterator pit = root.get_child("Parameterlist").begin();
	//for( ; it != parms.end() && pit != root.get_child("Parameterlist").end(); it++ )
}


// --- Parameter test program ---

struct Foo
{
	Foo()
		: d0( "d0" ),
		  d1( "d1" ),
		  index( "index" ),
		  title( "title" )
	{
		parms.push_back( &d0 );
		parms.push_back( &d1 );
		parms.push_back( &index );
		parms.push_back( &title );		
	}

	void test()
	{
		save_params( "foobar.xml", parms );

		load_params( "foobar.xml", parms );
	}

	DoubleParameter d0;
	DoubleParameter d1;
	IntParameter    index;
	StringParameter title;

	ParameterList   parms;
};

int main( int argc, char* argv[] )
{
	using namespace std;

	Foo foo;
	foo.test();

	return EXIT_SUCCESS;
}
