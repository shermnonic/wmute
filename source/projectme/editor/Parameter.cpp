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

//-----------------------------------------------------------------------------
// --- ParameterList implementation ---
//-----------------------------------------------------------------------------

void ParameterList::write( PTree& pt ) const
{
	std::string key = name() + ".Parameter";

	ParameterList::const_iterator it = begin();
	for( ; it != end(); ++it )
	{
		if( *it )
		{
			PTree pt;
			(*it)->write( pt );

			pt.add_child( key, pt );
		}
	}
}

void ParameterList::read( const PTree& pt )
{
	std::string root = name();

	if( pt.count( root )==0 )
	{
		// No root node found, do nothing.
		return;
	}

	// FIXME: Deserialization is hardcoded for specialized types.
	BOOST_FOREACH( const PTree::value_type& v, pt.get_child(root) )
	{
		if( v.first.compare("Parameter")==0 )
		{
			// Parse key and type
			ParameterBase base("<Unknown>");
			base.read( v.second );

			// Check if parameter with given key already exists
			ParameterBase* existing = get_param( base.key() );

			// Parameter with given key exists but type does not match!
			if( existing && base.type().compare(existing->type())!=0 )
			{
				// FIXME: Throw typed exception
				throw("ParameterList::read() : Type mismatch!");
			}

			// Handle specializations

			// DoubleParameter
			if( base.type().compare( "double" )==0 )
			{
				DoubleParameter* p = new DoubleParameter("<UnknownDouble>");
				p->read( v.second );

				if( existing )
				{
					DoubleParameter* pe = dynamic_cast<DoubleParameter*>( existing );
					assert( pe );
					// Set value and limits
					pe->setValue( p->value() );
					if( p->limits().active )
						pe->setLimits( p->limits().min_, p->limits().max_ );
				}
				else
				{
					// Add as new parameter
					push_back( p );
				}
			}
			else
			// IntParameter
			if( base.type().compare( "int" )==0 )
			{
				IntParameter* p = new IntParameter("<UnknownInt>");
				p->read( v.second );

				if( existing )
				{
					IntParameter* pe = dynamic_cast<IntParameter*>( existing );
					assert( pe );
					// Set value and limits
					pe->setValue( p->value() );
					if( p->limits().active )
						pe->setLimits( p->limits().min_, p->limits().max_ );
				}
				else
				{
					// Add as new parameter
					push_back( p );
				}
			}
			else
			// StringParameter
			if( base.type().compare( "int" )==0 )
			{
				StringParameter* p = new StringParameter("<UnknownString>");
				p->read( v.second );

				if( existing )
				{
					StringParameter* pe = dynamic_cast<StringParameter*>( existing );
					assert( pe );
					// Set value
					pe->setValue( p->value() );
				}
				else
				{
					// Add as new parameter
					push_back( p );
				}
			}
			else			
			// BoolParameter
			if( base.type().compare( "bool" )==0 )
			{
				BoolParameter* p = new BoolParameter("<UnknownBool>");
				p->read( v.second );

				if( existing )
				{
					BoolParameter* pe = dynamic_cast<BoolParameter*>( existing );
					assert( pe );
					// Set value
					pe->setValue( p->value() );
				}
				else
				{
					// Add as new parameter
					push_back( p );
				}
			}
			else
			// EnumParameter
			if( base.type().compare( "enum" )==0 )
			{
				EnumParameter* p = new EnumParameter("<UnknownEnum>");
				p->read( v.second );

				if( existing )
				{
					EnumParameter* pe = dynamic_cast<EnumParameter*>( existing );
					assert( pe );
					// Set value
					pe->setValue( p->value() );
				}
				else
				{
					// Add as new parameter
					push_back( p );
				}
			}
			else
				// FIXME: Throw typed exception
				throw("ParameterList::read() : Unknown parameter type!");
		}		
	}
}
