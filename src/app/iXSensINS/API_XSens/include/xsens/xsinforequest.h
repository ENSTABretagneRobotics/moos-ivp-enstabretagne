#ifndef XSINFOREQUEST_H
#define XSINFOREQUEST_H

/*!	\addtogroup enums Global enumerations
	@{
*/
/*! \brief Information request identifiers
	\details These values are used by the XsDevice::requestInfo function and
	XsCallback::onInfoResponse functions.
*/
enum XsInfoRequest {
	XIR_BatteryLevel = 0,
	XIR_GnssSvInfo,
	XIR_GpsSvInfo = XIR_GnssSvInfo			//!< \deprecated
};
/*! @} */
typedef enum XsInfoRequest XsInfoRequest;

#endif // file guard
