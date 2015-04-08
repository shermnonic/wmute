#ifndef SHADERPREPROCESSOR_H
#define SHADERPREPROCESSOR_H

#include <string>
#include <vector>

/**
	\class ShaderPreprocessor
	
	Very simplistic GLSL preprocessor.
	
	Parses comment-qualified lines of variable definitions or define statements.
	Marked variables are turned into uniforms. The idea is that hard-coded 
	variables from the original (not precompiled) shader can be replaced by live 
	adjustable uniforms/attributes.
	Marked defines are replaced with input values (if any given). This allows
	for instance to configure shader compilation.

	Currently only float, int and bool variables are supported!	

	All lines marked via a comment "//###" will be preprocessed.

	Syntax:
	\verbatim
		#define <name> <value>; //###{<value0,value1,...,valueN}
		<type> <name> [=<default_value>]; //###
	\endverbatim
	
	\author Max Hermann (mnemonic@386dx25.de)
	\date   Jan 2014
*/
class ShaderPreprocessor
{
public:
	/// Parsed shader variables definitions of the form
	///		<type> <name> [= <(default)-value>]; //###
	/// Each element is stored as string token.
	struct ShaderVariable
	{
		std::string type, name, value;
		
		ShaderVariable() {}
		ShaderVariable( std::string type_, std::string name_, std::string value_ )
			: type(type_), name(name_), value(value_) {}
	};

	/// Parsed shader defines of the form
	///		#define <name> <value>; //###{<values[0],values[1],...,values[N]}
	/// Each element is stored as string token.
	struct ShaderDefine
	{
		std::string name;
		std::string value;
		std::vector<std::string> values; // can numerical values or enum names
	};
	
	typedef std::vector<ShaderDefine>   ShaderDefines;
	typedef std::vector<ShaderVariable> ShaderVariables;
	
	/// Precompiles GLSL shader, returns modified code. Parses shader variables.
	std::string preprocess( const std::string& shader );
	std::string preprocess( const std::string& shader, ShaderDefines& defines );

	///@{ Access shader variables and defines (as parsed in last call to \a preprocess).
	ShaderVariables& vars() { return m_vars; }
	const ShaderVariables& vars() const { return m_vars; }
	ShaderDefines& defs() { return m_defs; }
	const ShaderDefines& defs() const { return m_defs; }
	///@}

private:
	ShaderVariables m_vars;
	ShaderDefines m_defs;
};

#endif // SHADERPREPROCESSOR_H
