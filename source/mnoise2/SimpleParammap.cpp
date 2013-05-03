#include "SimpleParammap.h"

namespace SimpleParammap {
	
void strip_whitespaces( std::string& s )
{
	for( size_t i=0; i < s.size(); ++i )
	{
		if( s[i] == ' ' ) 
		{
			s.erase( i, 1 );
			i--;
		}
	}
}

void read_parammap( std::istream& is, std::map<std::string,std::string>& params )
{
	using namespace std;

	// read parameter map	
	string line;
	while( getline( is, line ) )
	{
		// parse parameter assignment of the form:
		// <param_name> = <value>

		// consider all lines containing an equality sign
		// and ignore all other lines
		size_t splitter = line.find('=');
		if( splitter != string::npos )
		{
			// split on equality sign
			string param = line.substr(0,splitter-1),
				   value = line.substr(splitter+1);

			// remove white spaces from parameter
			strip_whitespaces( param );

			params[ param ] = value;
		}
	}	
}

void write_parammap( std::ostream& os, const std::map<std::string,std::string>& params )
{
	using namespace std;
	map<string,string>::const_iterator it;
	for( it=params.begin(); it!=params.end(); ++it )
	{
		os << it->first << " = " << it->second << endl;
	}
}

} // namespace SimpleParammap
