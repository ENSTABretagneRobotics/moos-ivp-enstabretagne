#include "xsgpspvtdata.h"
#include <string.h>

/*! \addtogroup cinterface C Interface
	@{
*/

/*! \brief Destroy the %XsGpsPvtData object 
	\deprecated
*/
void XsGpsPvtData_destruct(XsGpsPvtData* thisPtr)
{
	memset(thisPtr, 0, sizeof(XsGpsPvtData));
	thisPtr->m_pressureAge = 255;
	thisPtr->m_gpsAge = 255;
}

/*! \brief Returns true if the object is empty (when it contains no valid data) 
	\return non-zero (true) if the object is considered empty
	\deprecated
*/
int XsGpsPvtData_empty(const XsGpsPvtData* thisPtr)
{
	return thisPtr->m_pressureAge == 255 && thisPtr->m_gpsAge == 255;
}

/*! @} */
