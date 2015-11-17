#ifndef XSSYNCLINE_H
#define XSSYNCLINE_H

/*!	\addtogroup enums Global enumerations
	@{
*/
/*! \brief Synchronization line identifiers */
enum XsSyncLine
{
	XSL_Inputs,
	XSL_In1 = XSL_Inputs,				/*!< \brief Sync In 1 \remark Applies to Awinda Station and Mt */
	XSL_In2,							/*!< \brief Sync In 2 \remark Applies to Awinda Station */
	XSL_Bi1In,							/*!< \brief Bidirectional Sync 1 In \remark Applies to Xbus Master */
	XSL_ClockIn,						/*!< \brief Clock synchronisation input \remark Applies to Mk4 */
	XSL_CtsIn,							/*!< \brief RS232 CTS sync in \remark Applies to Xbus Master */
	XSL_GnssClockIn,					/*!< \brief Clock synchronisation input line attached to internal GPS unit \remark Applies to Mk4*/
	XSL_GpsClockIn = XSL_GnssClockIn,	/*!< \brief Clock synchronisation input line attached to internal GPS unit \remark Applies to Mk4 \deprecated */
	XSL_ExtTimepulseIn,					/*!< \brief External time pulse input (e.g. for external GNSS unit) \remark Applies to Mk4 with external device*/
	XSL_ReqData,						/*!< \brief Serial data sync option, use \a XMID_ReqData message id for this \remark Applies to Mk4*/
	XSL_Outputs,
	XSL_Out1 = XSL_Outputs,				/*!< \brief Sync Out 1 \remark Applies to Awinda Station and Mt */
	XSL_Out2,							/*!< \brief Sync Out 2 \remark Applies to Awinda Station */
	XSL_Bi1Out,							/*!< \brief Bidirectional Sync 1 Out \remark Applies to Xbus Master */
	XSL_RtsOut,							/*!< \brief RS232 RTS sync out \remark Applies to Xbus Master */

	XSL_Invalid
};
/*! @} */
typedef enum XsSyncLine XsSyncLine;

#endif // file guard
