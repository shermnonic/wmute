#ifndef SHADERPRECOMPILER2_H
#define SHADERPRECOMPILER2_H

#include "Parameter.h"

/**
	\class ShaderPrecompiler2
	
	Extends ShaderPreprocessor by parameter semantics.
	
	Usage example in a ShaderModule instance:
	\verbatim
		ShaderPrecompiler2 spc;
		// User active parameters/options for precprocessing
		spc.startPreprocessing( parameters(), options() );		
		fragShader = spc.precompile( fragShader );
		vertShader = spc.precompile( vertShader );
		spc.stopPreprocessing();

		// Reset parameters/options
		parameters() = m_superParameters;
		options() = m_superOptions;
		// Append parsed uniforms and defines
		parameters().insert( parameters().end(), spc.parameters().begin(), spc.parameters().end() );
		options().insert( options().end(), spc.options().begin(), spc.options().end() );
	\endverbatim	
*/
class ShaderPrecompiler2
{
public:
	void startPreprocessing( const ParameterList& parameters, const ParameterList& options );
	void stopPreprocessing();
	std::string precompile( std::string shader );

	ParameterList& parameters() { return m_params; }
	ParameterList& options()    { return m_opts; }
	
	/// Shader parameters (can be changed live)
	struct UniformParameters
	{		
		std::vector<DoubleParameter> floats;
		std::vector<IntParameter>    ints;
		std::vector<BoolParameter>   bools;
		// Maybe later we'll add support for some more types ...
		
		template<typename T>
		void append( const std::vector<T>& in, std::vector<T>& a )
		{
			for( int i=0; i < in.size(); i++ )
				a.push_back( in[i] );
		}
		
		void add( const UniformParameters& other ) {
			append<DoubleParameter>( other.floats, floats );
			append<IntParameter   >( other.ints,   ints   );
			append<BoolParameter  >( other.bools,  bools  );
		}		
		
		void clear()
		{
			floats.clear();
			ints  .clear();
			bools .clear();
		}
	};
	
	/// Shader defines (changes require recompilation)
	struct DefineOptions
	{
		std::vector<EnumParameter> enums;
		
		void add( const DefineOptions& other )
		{
			for( int i=0; i < other.enums.size(); i++ )
				enums.push_back( other.enums[i] );
		}
		
		void clear()
		{
			enums.clear();
		}
	};


	const UniformParameters& uniforms() { return m_uniformParams; }
	const DefineOptions&     defines()  { return m_defineOpts; }

private:
	ParameterList m_params;
	ParameterList m_opts;

	UniformParameters m_uniformParams;
	DefineOptions     m_defineOpts;
};

#endif // SHADERPRECOMPILER2_H
