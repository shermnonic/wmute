#ifndef PARAMETERIO_H
#define PARAMETERIO_H

#include "ParameterBase.h"
#include <iostream>

//-----------------------------------------------------------------------------
// --- Parameter IO ---
//-----------------------------------------------------------------------------

/// Print parameters to an output stream.
void print_params( const ParameterList& parms, std::ostream& os=std::cout );

/// Write parameters to disk.
void save_params( const char* filename, const ParameterList& parms );

/// Load parameters from disk to a given ParameterList, matched by key.
void load_params( const char* filename, ParameterList& parms );

#endif // PARAMETERIO_H
