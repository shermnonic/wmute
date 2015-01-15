#include "Parameter.h"
#include <boost/foreach.hpp>

//-----------------------------------------------------------------------------
// --- Parameter base implementation ---
//-----------------------------------------------------------------------------

ParameterBase::ParameterBase( const std::string& key )
	: m_key(key)
{
	// Internally assign type of derived class
	m_type = type();
}

std::string ParameterBase::key()  const { return m_key; }
std::string ParameterBase::type() const { return m_type; } //=0;

void ParameterBase::write( PTree& pt ) const
{
	pt.put( "key" , key() );
	pt.put( "type", type() );
}

void ParameterBase::read ( const PTree& pt )
{
	m_key  = pt.get<std::string>( "key" );		
	m_type = pt.get<std::string>( "type" );
}

std::string ParameterBase::str() const
{
	return std::string("(No string conversion available.)");
}

ParameterBase* ParameterList::get_param( std::string key )
{
	ParameterList::iterator it = begin();
	for( ; it != end(); it++ )
	{
		if( (*it)->key() == key )
			return *it;
	}
	
	return NULL;
}

// Equality operator (via key comparison, defined globally)
bool operator == ( const ParameterBase& a, const ParameterBase& b )
{
	return a.key() == b.key();
};

#if 0 // NOT YET!
//-----------------------------------------------------------------------------
// --- ParameterList implementation ---
//-----------------------------------------------------------------------------

void ParameterList::write( PTree& pt ) const
{
	ParameterList::const_iterator it = begin();
	for( ; it != end(); ++it )
	{
		if( *it )
		{
			PTree pt;
			(*it)->write( pt );

			pt.add_child( "ParameterList.Parameter", pt );
		}
	}
}

void ParameterList::read( const PTree& pt )
{
	//ParameterList l;

	BOOST_FOREACH( PTree::value_type& v, pt.get_child("ParameterList") )
	{
		if( v.first.compare("Parameter")==0 )
		{
			// TODO: Implement abstract factory to instantiate parameter
			//       of particular type.
		}
	}

}
#endif