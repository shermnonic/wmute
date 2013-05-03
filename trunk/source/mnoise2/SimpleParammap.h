#ifndef SIMPLEPARAMMAP_H
#define SIMPLEPARAMMAP_H

namespace SimpleParammap {

void strip_whitespaces( std::string& s );
void write_parammap( std::ostream& os, const std::map<std::string,std::string>& params );
void read_parammap( std::istream& is, std::map<std::string,std::string>& params );

//------------------------------------------------------------------------------
// Template Implementations
//------------------------------------------------------------------------------
	
template<class T>
void parse_val( std::string s, T& val )
{
	std::stringstream ss( s );
	ss >> val;
}

template<class T>
void parse_vals( std::string s, std::vector<T>& vals )
{
	std::stringstream ss( s );
	T f;
	while( ss >> f )
		vals.push_back( f );
}

template<class T>
void parse_val_from_stringmap( const std::map<std::string,std::string>& params,
							   std::string param, T& val, T default_val )
{
	using namespace std;
	map<string,string>::const_iterator it = params.find( param );
	if( it != params.end() )
	{
		parse_val<T>( it->second, val );
	}
	else
		val = default_val;
}

template<class T>
void parse_vals_from_stringmap( const std::map<std::string,std::string>& params,
								std::string param, std::vector<T>& vals, 
								std::vector<T> default_vals )
{
	using namespace std;
	map<string,string>::const_iterator it = params.find( param );
	if( it != params.end() )
	{
		parse_vals<T>( it->second, vals );
		if( vals.size() != default_vals.size() )
			vals = default_vals;
	}
	else
		vals = default_vals;
}

template<class T>
std::string val_to_string( const T& val )
{
	std::stringstream ss;
	ss << val;
	return ss.str();
}

template<class T>
std::string vals_to_string( const std::vector<T>& vals )
{
	std::stringstream ss;
	for( size_t i=0; i < vals.size(); ++i )
		ss << vals.at(i) << " ";
	return ss.str();
}

template<class T>
std::string vals_to_string( const T* vals, size_t size )
{
	std::stringstream ss;
	for( size_t i=0; i < size; ++i )
		ss << vals[i] << " ";
	return ss.str();
}

} // namespace SimpleParammap

#endif // SIMPLEPARAMMAP_H
