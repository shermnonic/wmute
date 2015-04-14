#ifndef SHADERPRECOMPILER2_H
#define SHADERPRECOMPILER2_H

#include "Parameter.h"
#include <map>

/**
	\class ShaderPrecompiler2
	
	Extends ShaderPreprocessor by parameter semantics.
	
	Usage example in a ShaderModule instance:
	\verbatim
		ShaderPrecompiler2 spc;
		// User active parameters/options for precprocessing
		spc.startPreprocessing( parameters(), options() );		
		// Mark non-shader parameters/options (such that they are not deleted)
		spc.markParameters( m_superParameters );
		spc.markParameters( m_superOptions );
		fragShader = spc.precompile( fragShader );
		vertShader = spc.precompile( vertShader );
		spc.stopPreprocessing(); // delete non-marked parameters/options

		// Append parsed uniforms and defines
		parameters().clear();
		options().clear();
		parameters().insert( parameters().end(), spc.parameters().begin(), spc.parameters().end() );
		options().insert( options().end(), spc.options().begin(), spc.options().end() );
	\endverbatim	
*/
class ShaderPrecompiler2
{
	typedef std::map<std::string,bool> KeyMap;

	/// Shader parameters (can be changed live)
	struct UniformParameters
	{
		// We store pointers to parameters because they are altered elsewere (e.g. in the GUI)
		std::vector<DoubleParameter*> floats;
		std::vector<IntParameter*>    ints;
		std::vector<BoolParameter*>   bools;
		// Maybe later we'll add support for some more types ...		
		
		void clear()
		{
			floats.clear();
			ints  .clear();
			bools .clear();
		}
	};

public:
	void startPreprocessing( const ParameterList& parameters, const ParameterList& options );
	void stopPreprocessing();
	std::string precompile( std::string shader );

	ParameterList& parameters() { return m_params; }
	ParameterList& options()    { return m_opts; }

	void markParameters( const ParameterList& pl );

	const UniformParameters& uniforms() { return m_uniforms; }

protected:
	// Mark preprocessor key as used
	void markKey( std::string key );

	bool isMarked( std::string key ) const;

	void eraseNonMarked( ParameterList& pl );	

private:
	UniformParameters m_uniforms;

	ParameterList m_params;
	ParameterList m_opts;

	KeyMap m_markedKeys;
};

#endif // SHADERPRECOMPILER2_H
