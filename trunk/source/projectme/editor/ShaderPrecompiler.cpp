#include "ShaderPrecompiler.h"
#include <sstream>
#include <iostream>
#include <iterator>
#include <algorithm>

using namespace std;

std::string ShaderPrecompiler
  ::precompile( const std::string& shader )
{
	using namespace std;
	
	const string MARKER("//###");
	
	// Clear variables
	m_vars.clear();
	
	// Parse shader source
	istringstream ss( shader );
	stringstream os;	
	string line, comment;
	int lineCount=1;
	while( getline( ss, line ) )
	{
		// Look for marker
		size_t found = line.find( MARKER );
		if( found != string::npos )
		{
			// Marker found
			comment = line.substr( found+MARKER.length() );
			line = line.substr( 0, found );
			
			// Replace '=' with ' '
			size_t p;
			while( (p = line.find( '=' )) != string::npos )
				line.replace( p, 1, " " );
			
			// Erase ';'
			while( (p = line.find( ';' )) != string::npos )
				line.erase( p, 1 );
			
			// Tokenize
			// See http://stackoverflow.com/questions/236129/split-a-string-in-c
			istringstream tmp(line);
			vector<string> tokens;
			copy( istream_iterator<string>(tmp),
				  istream_iterator<string>(),
			      back_inserter(tokens) );			
			
			if( tokens.size() < 2 || tokens.size() > 3 )
			{
				cerr << "Shader precompiler : "
					"Syntax error in shader variable definition on line " 
					<< lineCount << ":" << endl
					<< line << endl;
				os << line << "\n"; // Copy line w/o transforming				
			}
			
			// Parse 			
			// 	<type> <name> [=<default_value>];
			string 
				sType(tokens[0]), 
				sName(tokens[1]), 
				sDefault(tokens.size()>2?tokens[2]:string());
			
			// So far we only support 'float' type
			if( sType.compare("float")==0 )
			{			
				// Store variable
				m_vars.push_back( ShaderVariable( sType, sName, sDefault ) );
				
				// Transform to uniform in output
				os << "uniform " << sType << " " << sName << ";\n";			
			}
			else
			{
				cerr << "Shader precompiler : "
					"Non float shader variable defined on line " 
					<< lineCount << " not supported yet:" << endl
					<< line << endl;
				os << line << "\n"; // Copy line w/o transforming
			}
		}
		else
		{
			// Marker not found, simply copy line
			os << line << "\n";
		}
		
		lineCount++;
	}	
	
	return os.str();
}
