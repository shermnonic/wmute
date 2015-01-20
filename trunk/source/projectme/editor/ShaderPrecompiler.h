#ifndef SHADERPRECOMPILER_H
#define SHADERPRECOMPILER_H

#include <string>
#include <vector>

/**
	\class ShaderPrecompiler
	
	Very simplistic GLSL preprocessor.
	
	Parses comment-qualified lines of variable definitions and turns them into
	uniforms. The idea is that hard-coded variables from the original (not 
	precompiled) shader can be replaced by live adjustable uniforms/attributes.	
	Currently only float variables are supported, see \a preprocess().
	
	\todo Syntax description of parser.
	
	\author Max Hermann (mnemonic@386dx25.de)
	\date   Jan 2014
*/
class ShaderPrecompiler
{
public:
	/// Parsed shader variables as <type> <name> <(default)-value> strings.
	struct ShaderVariable
	{
		std::string type, name, value;
		
		ShaderVariable() {}
		ShaderVariable( std::string type_, std::string name_, std::string value_ )
			: type(type_), name(name_), value(value_) {}
	};
	
	/// Collection of shader variables.
	typedef std::vector<ShaderVariable> ShaderVariables;
	
	/// Precompiles GLSL shader, returns modified code. Parses shader variables.
	std::string precompile( const std::string& shader );

	///@{ Access shader variables (as parsed in last call to \a preprocess).
	ShaderVariables& vars() { return m_vars; }
	const ShaderVariables& vars() const { return m_vars; }
	///@}

private:
	ShaderVariables m_vars;
};

#endif // SHADERPRECOMPILER_H
