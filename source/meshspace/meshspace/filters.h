#ifndef FILTERS_H
#define FILTERS_H

#include <meshtools.h>

/** @addtogroup meshspace meshspace
  * @{ */

/// Custom mesh filters
namespace filters
{

using meshtools::Mesh;
	
void closestPointDistance( const Mesh& source, const Mesh& target, std::vector<float>& dist );
	
}; // namespace filters

/** @} */ // end group

#endif // FILTERS_H
