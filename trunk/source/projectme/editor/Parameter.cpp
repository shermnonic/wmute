#include "Parameter.h"

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
