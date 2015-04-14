#include "ShaderPrecompiler2.h"
#include "ShaderPreprocessor.h"
#include <iostream>
using std::cerr;
using std::endl;


//----------------------------------------------------------------------------
void ShaderPrecompiler2::startPreprocessing( const ParameterList& parameters, const ParameterList& options )
{
	m_params = parameters;
	m_opts = options;

	m_markedKeys.clear();
	m_uniforms.clear();
}

//----------------------------------------------------------------------------
void ShaderPrecompiler2::eraseNonMarked( ParameterList& pl )
{
	// Delete all non-marked parameters	
	ParameterList::iterator it = pl.begin();
	while( it != pl.end() )
	{
		if( isMarked( (*it)->key() ) )
		{
			// Free memory of Parameter!
			delete *it;
			it = pl.erase( it );
		}
		else
		{
			++it;
		}
	}
}

//----------------------------------------------------------------------------
void ShaderPrecompiler2::stopPreprocessing()
{
#if 0 // FIXME Memory leak!
	eraseNonMarked( m_params );
	eraseNonMarked( m_opts );
#endif
}

//----------------------------------------------------------------------------
void ShaderPrecompiler2::markParameters( const ParameterList& pl )
{
	for( unsigned i=0; i < pl.size(); i++ )
		markKey( pl[i]->key() );
}

//----------------------------------------------------------------------------
bool ShaderPrecompiler2::isMarked( std::string key ) const
{
	return m_markedKeys.count( key ) > 0;
}

//----------------------------------------------------------------------------
void ShaderPrecompiler2::markKey( std::string key )
{
	m_markedKeys[key] = true;
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
    for( unsigned i=0; i < defs.size(); i++ )
	{
		string key = defs[i].name;
		int value = !defs[i].value.empty() ? atoi(defs[i].value.c_str()) : 0;

		// Get enum names
		vector<string> enumNames = defs[i].values;
		if( enumNames.empty() )
		{
			// Set some default enum names
			enumNames.push_back("Default0");
			enumNames.push_back("Default1");
		}

		// Create enum parameter
		EnumParameter p( key, value, enumNames );

		// Get existing value from options (if available)
		EnumParameter* cur = dynamic_cast<EnumParameter*>( m_opts.get_param( key ) );
		if( cur )
		{
			// Update existing parameter
			if( cur->value() != value )
				defValueChanged = true;

			// Copy data
			*cur = p;			
		}

		markKey( key );
	}
	
	// Second precompilation only necessary on define value change
	if( defValueChanged )
		shaderProcessed = pc.preprocess( shader, defs );
	
	// Collect 'float', 'int' and 'bool' parameters
    for( unsigned i=0; i < vars.size(); i++ )
	{
		string key = vars[i].name;
		string type = vars[i].type;

		if( type.compare("float")==0 )
		{
			double value = !vars[i].value.empty() ? atof(vars[i].value.c_str()) : 1.0;
			DoubleParameter* cur = dynamic_cast<DoubleParameter*>( m_params.get_param( key ) );
			if( cur )
			{
				// Update existing parameter default value
				cur->setDefault( value );
			}
			else
			{
				// Add new parameter
				cur = new DoubleParameter( key, value );
				m_params.push_back( cur );
			}
			// Mark key as used
			if( !isMarked(key) )
			{
				m_uniforms.floats.push_back( cur );
			}
			markKey( key );
		}
		else
		if( type.compare("int")==0 )
		{
			int value = !vars[i].value.empty() ? atoi(vars[i].value.c_str()) : 0;
			IntParameter* cur = dynamic_cast<IntParameter*>( m_params.get_param( key ) );
			if( cur )
			{
				cur->setDefault( value );
			}
			else
			{
				cur = new IntParameter( key, value );
				m_params.push_back( cur );
			}

			if( !isMarked(key) )
			{
				m_uniforms.ints.push_back( cur );
			}
			markKey( key );
		}
		else
		if( type.compare("bool")==0 )
		{
			bool value = !vars[i].value.empty() ? atoi(vars[i].value.c_str()) : true;
			BoolParameter* cur = dynamic_cast<BoolParameter*>( m_params.get_param( key ) );
			if( cur )
			{
				cur->setDefault( value );
			}
			else
			{
				cur = new BoolParameter( key, value );
				m_params.push_back( cur );
			}
			
			if( !isMarked(key) )
			{
				m_uniforms.bools.push_back( cur );
			}
			markKey( key );
		}
		else
		{
			cerr << "ShaderPrecompiler2::precompile() : Unsupported parameter "
				"type \"" << type << "\"!" << endl;
		}
	}
	
	return shaderProcessed;
}
