#include "ShaderPreprocessor.h"
#include <sstream>
#include <iostream>
#include <iterator>
#include <algorithm>

using namespace std;

vector<string> tokenize( string line )
{
	// Tokenize
	// See http://stackoverflow.com/questions/236129/split-a-string-in-c
	istringstream tmp(line);
	vector<string> tokens;
	copy( istream_iterator<string>(tmp),
			istream_iterator<string>(),
			back_inserter(tokens) );
	return tokens;
}

bool enclosed_in( const string s, char left, char right )
{
	return !s.empty() && s[0]==left && s[s.length()-1]==right;
}

int replace_all( string& s, string target, string replacement )
{
	size_t p;
	int count=0;
	while( (p = s.find(target)) != string::npos )
	{
		s.replace( p, target.length(), replacement );
		count++;
	}
	return count;
}

string remove_trailing_whitespaces( const string& line )
{
	// TODO: Consider also tabs '\t'
	size_t found = line.find_first_not_of(' ');
	if( found != string::npos )
		return line.substr( found );
	return line;
}

bool starts_with( const string& line, const string& prefix )
{
	string tmp = remove_trailing_whitespaces( line );
	return tmp.find(prefix) == 0;
}

/**
	Parse a one-line statement of the form
	\verbatim
		#define <name> [<value>]; //###{values=<value0>,<value1>,...,<valueN>}
	\endverbatim
	In case of success the replaced string is returned (with appended newline)
	else the original input line is returned (again with appended newline).
	The parsed output is stored in \a var. Success is indicated via \a ok.
*/
string parseShaderDefine( string line, string comment,
						  ShaderPreprocessor::ShaderDefine& def, 
						  const ShaderPreprocessor::ShaderDefines& defs,
						  bool& ok )
{
	vector<string> tokens;

	// Parse comment
	tokens = tokenize( comment ); 		
	if( tokens.size() > 0 )
	{
		// We only consider the first token and the rest is treated as comment.

		// Expect expression enclosed by '{' and '}'
		if( enclosed_in( tokens[0], '{', '}' ) )
		{
			// String inside braces
			string s = tokens[0].substr(1,tokens[0].length()-2);

			// Replace ',' with ' '
			replace_all( s, ",", " " );

			tokens = tokenize( s );

			if( !tokens.empty() )
			{
				for( size_t i=0; i < tokens.size(); i++ )
					def.values.push_back( tokens[i] );
			}
		}		
	}
	
	// Parse line
	tokens = tokenize( line );
	if( tokens.size() != 3 )
	{
		// Parse error, return line unchanged and set error indicator
		ok = false;
		return line + '\n';
	}

	if( tokens[0].compare("#define") != 0 )
	{
		// Parse error, return line unchanged and set error indicator
		ok = false;
		return line + '\n';
	}

	ok = true;
	def.name  = tokens[1];
	def.value = tokens[2];

	// Check if a define of this name is given in defs
    for( unsigned i=0; i < defs.size(); i++ )
		if( defs[i].name.compare( def.name )==0 )
		{
			// Found a define of the same name, use its value
			def.value = defs[i].value;
			break;
		}

	// Return processed line
	stringstream os;
	os << "#define " << def.name << " " << def.value << "\n";
	return os.str();
}

/**
	Parse a one-line statement of the form
	\verbatim
		<type> <name> [=<default_value>]; //###
	\endverbatim
	and replace it with
	\verbatim
		uniform <type> <name>;
	\endverbatim
	In case of success the replaced string is returned (with appended newline)
	else the original input line is returned (again with appended newline).
	The parsed output is stored in \a var. Success is indicated via \a ok.
*/
string parseShaderVariable( string line, string /*comment*/,
						    ShaderPreprocessor::ShaderVariable& var, bool& ok )
{
	// Replace '=' with ' '
	size_t p;
	while( (p = line.find( '=' )) != string::npos )
		line.replace( p, 1, " " );
			
	// Erase ';'
	while( (p = line.find( ';' )) != string::npos )
		line.erase( p, 1 );
			
	// Tokenize
	vector<string> tokens = tokenize( line );
	if( tokens.size() < 2 || tokens.size() > 3 )
	{
		// Parse error, return line unchanged and set error indicator
		ok = false;
		return line + '\n';
	}
			
	// Parse 			
	// 	<type> <name> [=<default_value>];
	string 
		sType(tokens[0]), 
		sName(tokens[1]), 
		sDefault(tokens.size()>2?tokens[2]:string());

	// So far we only support types 'float', 'int' and 'bool'
	stringstream os;
	if( sType.compare("float")==0 || 
		sType.compare("int")  ==0 ||  
		sType.compare("bool") ==0 )
	{			
		// Store variable
		var = ShaderPreprocessor::ShaderVariable( sType, sName, sDefault );
				
		// Transform to uniform in output
		os << "uniform " << sType << " " << sName << ";\n";
	}
	else
	{
		// Parse error, return line unchanged and set error indicator
		ok = false;
		os << line << "\n"; // Copy line w/o transforming
	}
	ok = true; // Everything went fine!
	return os.str();
}

std::string ShaderPreprocessor
  ::preprocess( const std::string& shader )
{
	ShaderDefines defs;
	std::string s = preprocess( shader, defs );
	return s;
}

std::string ShaderPreprocessor
  ::preprocess( const std::string& shader, ShaderDefines& defines )
{
	using namespace std;
	
	const string MARKER("//###");
	
	ShaderVariables vars;
	ShaderDefines defs;
	
	// Parse shader source
	istringstream ss( shader );
	stringstream os;	
	string line, comment;
	int lineCount=0;
	while( getline( ss, line ) )
	{
		lineCount++;

		// Look for marker
		// Ignore single-line comments (multi-line comment blocks are not detected!)
		size_t found = line.find( MARKER );
		if( found != string::npos && !starts_with(line,"//") )
		{
			// Marker found
			comment = line.substr( found+MARKER.length() );
			line = line.substr( 0, found );

			bool ok;

			// Distinguish between define statement and variable definition
			if( starts_with(line,"#") )
			{
				// Define statement
				ShaderDefine def;
				os << parseShaderDefine( line, comment, def, defines, ok );
				if( ok )
				{
					// Add define
					defs.push_back( def );
				}
			}
			else
			{
				// Variable definition
				ShaderVariable var;
				os << parseShaderVariable( line, comment, var, ok );
				if( ok )
				{
					// Add variable
					vars.push_back( var );
				}
			}

			// Generic error message in case something went wrong
			if( !ok )
			{
				cerr << "ShaderPreprocessor : "
						"Syntax error in line " 
						<< lineCount << ":" << endl
						<< line << endl;
			}
		}
		else
		{
			// Marker not found, simply copy line
			os << line << "\n";
		}		
	}	

	// Replace variables and defines
	m_vars = vars;
	m_defs = defs;
	
	return os.str();
}
