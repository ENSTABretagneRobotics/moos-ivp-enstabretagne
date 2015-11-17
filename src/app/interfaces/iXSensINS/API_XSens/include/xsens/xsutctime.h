#ifndef XSUTCTIME_H
#define XSUTCTIME_H

#include "xstypesconfig.h"
#include "pstdint.h"

#ifdef __cplusplus
extern "C"
{
#else
#define XSUTCTIME_INITIALIZER	{ 0, 0 ,0, 0, 0, 0, 0, 0}
#endif
struct XsUtcTime;

XSTYPES_DLL_API void XsUtcTime_currentTime(struct XsUtcTime * now);

#ifdef __cplusplus
} // extern "C"
#endif

/*! \brief A structure for storing UTC Time values. */
struct XsUtcTime {
	uint32_t	m_nano;		//!< \brief Nanosecond part of the time
	uint16_t	m_year;		//!< \brief The year (if date is valid)
	uint8_t		m_month;	//!< \brief The month (if date is valid)
	uint8_t		m_day;  	//!< \brief The day of the month (if date is valid)
	uint8_t		m_hour;		//!< \brief The hour (if time is valid)
	uint8_t		m_minute;	//!< \brief The minute (if time is valid)
	uint8_t		m_second;	//!< \brief The second (if time is valid)
	uint8_t		m_valid;	//!< \brief Validity indicator \details When set to 1, the time is valid, when set to 2, the time part is valid, but the date may not be valid. when set to 0, the time is invalid and should be ignored.

#ifdef __cplusplus
	/*! \copydoc XsUtcTime_currentTime
	   \return The current UTC Time
	*/
	inline static XsUtcTime currentTime()
	{
		XsUtcTime tmp;
		XsUtcTime_currentTime(&tmp);
		return tmp;
	}
#endif
};
typedef struct XsUtcTime XsUtcTime;

#endif // file guard
