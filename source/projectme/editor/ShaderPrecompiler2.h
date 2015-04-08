#ifndef SHADERPRECOMPILER2_H
#define SHADERPRECOMPILER2_H

#include "Parameter.h"

/**
	\class ShaderPrecompiler2
	
	Extends ShaderPreprocessor by parameter semantics.
	
	Usage example:
	\verbatim
		startPreprocessing( myParameterListForUniforms, myParameterListForDefines );
		fragShader = precompile( fragShader );
		vertShader = precompile( vertShader );
		stopPreprocessing(); // note that parameter lists get changed here!
	\endverbatim	
*/
class ShaderPrecompiler2
{
public:
	void startPreprocessing( ParameterList& params, ParameterList& opts );
	void stopPreprocessing();
	std::string precompile( std::string shader );
	
	/// Shader parameters (can be changed live)
	struct UniformParameters
	{		
		std::vector<DoubleParameter> floats;
		std::vector<IntParameter>    ints;
		std::vector<BoolParameter>   bools;
		// Maybe later we'll add support for some more types ...
		
		template<typename T>
		void append( const std::vector<T>& in, std::vector<T> a )
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

private:
	ParameterList& m_params;
	ParameterList& m_opts;

	UniformParameters m_uniformParams;
	DefineOptions     m_defineOpts;
};

#endif // SHADERPRECOMPILER2_H
