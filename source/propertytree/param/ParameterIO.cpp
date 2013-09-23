#include "ParameterIO.h"

#include <vector>
#include <algorithm>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

void print_params( const ParameterList& parms, std::ostream& os )
{
	ParameterList::const_iterator it = parms.begin();
	for( ; it != parms.end(); it++ )
	{
		std::cout << (*it)->key() 
			<< " = " << (*it)->str()
			<< "  (" << (*it)->type() << ")" << std::endl;
	}
}

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
	using namespace std;
	using boost::property_tree::ptree;
	ptree root;
	
	read_xml( filename, root );

	// ParameterList's on disk and in parms must match!
	// Iterate simultaneously over both.
	ParameterList::iterator it = parms.begin();	

#if 0
	// Debugging
	std::cout << "Read following parameter list from " << filename << ":" << std::endl;
	BOOST_FOREACH( ptree::value_type &v, root.get_child("ParameterList") )
	{
		std::cout << v.second.get<std::string>("key")
			<< "(" << v.second.get<std::string>("type") << ")" << std::endl;
	}
#endif

	// Iterate through read property_tree
	ptree pt = root.get_child("ParameterList");
	boost::property_tree::ptree::iterator pit = pt.begin();	
#if 0
	// Assume exactly matching ParameterList and property_tree instances
	// (static solution)  OBSOLETE
	for( ; it != parms.end() && pit != pt.end(); it++, pit++ )
	{	
		// Read base fields (key,type) from file
		ParameterBase param("");
		param.read( pit->second );

		// Sanity check
		if( (*it)->type() == param.type() )
		{
			(*it)->read( pit->second );
		}
		else
		{
			// Type mismatch
			std::cerr << "Error: Type mismatch for variable " 
				<< param.key() << std::endl;

			// Reset to default value
			(*it)->reset();
		}
	}	
#else
	// Match found variable names in property_tree to ParameterList
	// (dynamic solution)
	for( ; pit != pt.end(); pit ++ )
	{
		// Read base fields (key,type) from file
		ParameterBase param("");
		param.read( pit->second );

		ParameterBase* ptr = parms.get_param( param.key() );
		if( ptr )
		{
			// Sanity check type
			if( ptr->type() == param.type() )
			{
				// Found matching variable in given parameter list
				// Read again from file, this time with all fields for the
				// actual type.
				ptr->read( pit->second );
			}
			else
			{
				// Type mismatch
				std::cerr << "Error: Type mismatch for variable " 
					<< param.key() << std::endl;
			}
		}
	}
#endif
}
