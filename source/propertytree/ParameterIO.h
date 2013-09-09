#ifndef PARAMETERIO_H
#define PARAMETERIO_H

#include "ParameterBase.h"

//-----------------------------------------------------------------------------
// --- Parameter IO ---
//-----------------------------------------------------------------------------

void save_params( const char* filename, const ParameterList& parms );

// Load w/o factory to known ParameterList
void load_params( const char* filename, ParameterList& parms );

#endif // PARAMETERIO_H
