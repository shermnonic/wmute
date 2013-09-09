#include "ParameterIO.h"

#include <iostream>
#include <vector>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

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
