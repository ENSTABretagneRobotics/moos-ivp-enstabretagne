#include "xstimestamp.h"
#include "xsutctime.h"

/*! \class XsUtcTime
	\brief This class can contain UTC time information
*/

/*! \addtogroup cinterface C Interface
	@{
*/

/*! \relates XsTimeStamp
	\brief Returns the current time in UTC date time format
	\param now The object to write the time to.
*/
void XsUtcTime_currentTime(struct XsUtcTime * now)
{
	XsTimeStamp timeStamp;

	if (now == 0)
		return;

	(void)XsTimeStamp_now(&timeStamp);
	XsTimeStamp_toUtcTime(&timeStamp, now);
}


/*! @} */
