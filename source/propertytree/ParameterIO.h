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

/// Load parameters from disk w/o factory to a known ParameterList.
/// \warning The given ParameterList must match *exactly* the parameters stored
///          in the file to be read. Maybe this restriction will be weakened
///          in future releases by matching parameter names and types.
///          Fully automatic generation of a ParameterList from scratch would
///          require a factory mechanism which is not implemented yet.
void load_params( const char* filename, ParameterList& parms );

#endif // PARAMETERIO_H
