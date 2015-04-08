#include "ShaderPrecompiler2.h"
#include "ShaderPreprocessor.h"
#include <iostream>
using std::cerr;
using std::endl;

//----------------------------------------------------------------------------
void ShaderPrecompiler2::startPreprocessing( ParameterList& params, ParameterList& opts )
{
	m_params = params;
	m_opts   = opts;

	m_defineOpts   .clear();
	m_uniformParams.clear();
}

//----------------------------------------------------------------------------
void ShaderPrecompiler2::stopPreprocessing()
{
	//options() = m_superOptions; // Reset module options
	//parameters() = m_superParameters; // Reset to inherited state

	// Update options list
	for( int i=0; i < m_defineOpts.enums.size(); i++ ) // Add define enums
		m_opts.push_back( &m_defineOpts.enums[i] );	
	
	// Update parameter list 
	for( int i=0; i < m_uniformParams.floats.size(); ++i )
		m_params.push_back( &m_uniformParams.floats[i] );
	for( int i=0; i < m_uniformParams.ints.size(); ++i )
		m_params.push_back( &m_uniformParams.ints[i] );
	for( int i=0; i < m_uniformParams.bools.size(); ++i )
		m_params.push_back( &m_uniformParams.bools[i] );	
}

//----------------------------------------------------------------------------
std::string ShaderPrecompiler2::precompile( std::string shader )
{	
	using namespace std;

	ShaderPreprocessor pc;

	// Only treat variables and not defines
	string shaderProcessed = pc.preprocess( shader );
	ShaderPreprocessor::ShaderVariables& vars = pc.vars();

	
	// Precompile twice - in 1st run defines are parsed and set in the 2nd round
	ShaderPreprocessor::ShaderDefines defs = pc.defs();

	// Collect enum defines
	bool defValueChanged = false;
	DefineOptions defines;
	for( int i=0; i < defs.size(); i++ )
	{
		string key = defs[i].name;

		// Get enum names
		vector<string> enumNames = defs[i].values;
		if( enumNames.empty() )
		{
			// Set some default enum names
			enumNames.push_back("Default0");
			enumNames.push_back("Default1");
		}

		// Create enum parameter
		EnumParameter p( key, 
			!defs[i].value.empty() ? atoi(defs[i].value.c_str()) : 0, // value & default
			enumNames );

		// Get existing value from options (if available)
		EnumParameter* cur = dynamic_cast<EnumParameter*>( m_opts.get_param( key ) );
		if( cur )
		{
			p.setValue( cur->value() );
			// Set also ShaderDefine value for 2nd precompilation pass
			stringstream ss; ss << cur->value();
			defs[i].value = ss.str();
			defValueChanged = true;
		}

		defines.enums.push_back( p );
	}
	
	// Second precompilation only necessary on define value change
	if( defValueChanged )
		shaderProcessed = pc.preprocess( shader, defs );
	
	// Update options
	m_defineOpts.add( defines );
	
	
	// Collect 'float', 'int' and 'bool' parameters
	UniformParameters uniforms;
	for( int i=0; i < vars.size(); i++ )
	{
		string key = vars[i].name;
		string type = vars[i].type;

		if( type.compare("float")==0 )
		{		
			DoubleParameter p( key );		

			// Set default value (if specified)
			p.setValueAndDefault( !vars[i].value.empty() ? atof(vars[i].value.c_str()) : 1.0 );

			// Get value from existing 'float' (if available)
			DoubleParameter* cur = dynamic_cast<DoubleParameter*>( m_params.get_param( key ) );
			if( cur )
				p.setValue( cur->value() );
		
			uniforms.floats.push_back( p );
		}
		else
		if( type.compare("int")==0 )
		{
			IntParameter p( key );
			p.setValueAndDefault( !vars[i].value.empty() ? atoi(vars[i].value.c_str()) : 0 );
			IntParameter* cur = dynamic_cast<IntParameter*>( m_params.get_param( key ) );
			if( cur )
				p.setValue( cur->value() );
			uniforms.ints.push_back( p );
		}
		else
		if( type.compare("bool")==0 )
		{
			BoolParameter p( key );
			p.setValueAndDefault( !vars[i].value.empty() ? atoi(vars[i].value.c_str()) : true );
			BoolParameter* cur = dynamic_cast<BoolParameter*>( m_params.get_param( key ) );
			if( cur )
				p.setValue( cur->value() );
			uniforms.bools.push_back( p );
		}
		else
		{
			cerr << "ShaderPrecompiler2::precompile() : Unsupported parameter "
				"type \"" << type << "\"!" << endl;
		}
	}	
	
	// FIXME: Parameters should not be accessed after changing their instances!
	m_uniformParams.add( uniforms );
	

	return shaderProcessed;
}
