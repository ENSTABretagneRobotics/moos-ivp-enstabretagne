#include "xsdeviceid.h"
#include "xsstring.h"
#include "xsdid.h"
#include <stdio.h>

/*! \class XsDeviceId
	\brief Contains an Xsens device ID and provides operations for determining the type of device
*/

/*! \addtogroup cinterface C Interface
	@{
*/

/*!	\relates XsDeviceId
	\brief Returns the value of a broadcast deviceID.
*/
uint32_t XsDeviceId_broadcast(void)
{
	return XS_DID_BROADCAST;
}

/*!	\relates XsDeviceId
	\brief Test if the device ID is a valid id (not 0).
*/
int XsDeviceId_isValid(const XsDeviceId* thisPtr)
{
	return thisPtr->m_deviceId != 0;
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an Xbus Master. */
int XsDeviceId_isXbusMaster(const XsDeviceId* thisPtr)
{
	return ((thisPtr->m_deviceId & XS_DID_TYPEH_MASK) == XS_DID_TYPEH_XM);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an awinda1 device. */
int XsDeviceId_isAwinda1(struct XsDeviceId const* thisPtr)
{
	return XS_DID_AWINDA1(thisPtr->m_deviceId);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an awinda2 device. */
int XsDeviceId_isAwinda2(struct XsDeviceId const* thisPtr)
{
	return XS_DID_AWINDA2(thisPtr->m_deviceId);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an awinda device. */
int XsDeviceId_isAwinda(struct XsDeviceId const* thisPtr)
{
	return XsDeviceId_isAwinda1(thisPtr) || XsDeviceId_isAwinda2(thisPtr);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents a bodypack device. */
int XsDeviceId_isBodyPack(struct XsDeviceId const* thisPtr)
{
	return XS_DID_BODYPACK(thisPtr->m_deviceId) || (thisPtr->m_deviceId == XS_DID_ABMCLOCKMASTER);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents a Wireless Master device (Awinda Station, Awinda Dongle, Awinda OEM). */
int XsDeviceId_isWirelessMaster(const XsDeviceId* thisPtr)
{
	return ((thisPtr->m_deviceId & XS_DID_TYPEH_MASK) == XS_DID_TYPEH_AWINDAMASTER) &&
			!XsDeviceId_isBodyPack(thisPtr) &&
			!XsDeviceId_isSyncStation(thisPtr);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an Awinda1 Station.
*/
int XsDeviceId_isAwinda1Station(const XsDeviceId* thisPtr)
{
	return XS_DID_AWINDA1_STATION(thisPtr->m_deviceId);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an Awinda1 Dongle. */
int XsDeviceId_isAwinda1Dongle(const XsDeviceId* thisPtr)
{
	return XS_DID_AWINDA1_DONGLE(thisPtr->m_deviceId);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an Awinda1 OEM board. */
int XsDeviceId_isAwinda1Oem(const XsDeviceId* thisPtr)
{
	return XS_DID_AWINDA1_OEM(thisPtr->m_deviceId);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an Awinda2 Station.
*/
int XsDeviceId_isAwinda2Station(const XsDeviceId* thisPtr)
{
	return XS_DID_AWINDA2_STATION(thisPtr->m_deviceId);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an Awinda2 Dongle. */
int XsDeviceId_isAwinda2Dongle(const XsDeviceId* thisPtr)
{
	return XS_DID_AWINDA2_DONGLE(thisPtr->m_deviceId);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an Awinda2 OEM board. */
int XsDeviceId_isAwinda2Oem(const XsDeviceId* thisPtr)
{
	return XS_DID_AWINDA2_OEM(thisPtr->m_deviceId);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an Awinda Station.
*/
int XsDeviceId_isAwindaStation(const XsDeviceId* thisPtr)
{
	return XsDeviceId_isAwinda1Station(thisPtr) || XsDeviceId_isAwinda2Station(thisPtr);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an Awinda Dongle. */
int XsDeviceId_isAwindaDongle(const XsDeviceId* thisPtr)
{
	return XsDeviceId_isAwinda1Dongle(thisPtr) || XsDeviceId_isAwinda2Dongle(thisPtr);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an Awinda OEM board. */
int XsDeviceId_isAwindaOem(const XsDeviceId* thisPtr)
{
	return XsDeviceId_isAwinda1Oem(thisPtr) || XsDeviceId_isAwinda2Oem(thisPtr);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents a SyncStation. */
int XsDeviceId_isSyncStation(const XsDeviceId* thisPtr)
{
	return XS_DID_SYNCSTATION(thisPtr->m_deviceId);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MTw1. */
int XsDeviceId_isMtw1(const XsDeviceId* thisPtr)
{
	return XS_DID_WMMT(thisPtr->m_deviceId);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MTw2. */
int XsDeviceId_isMtw2(const XsDeviceId* thisPtr)
{
	return XS_DID_MTW2(thisPtr->m_deviceId);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MTw */
int XsDeviceId_isMtw(const struct XsDeviceId* thisPtr)
{
	return XsDeviceId_isMtw1(thisPtr) || XsDeviceId_isMtw2(thisPtr);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an Xbus Master Motion Tracker. */
int XsDeviceId_isXbusMasterMotionTracker(const XsDeviceId* thisPtr)
{
	return ((thisPtr->m_deviceId & XS_DID_TYPE_MASK) == XS_DID_TYPE_MTX_XBUS);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MT9c. */
int XsDeviceId_isMt9c(const XsDeviceId* thisPtr)
{
	return (((thisPtr->m_deviceId & XS_DID_TYPEH_MASK) == XS_DID_TYPEH_MTI_MTX) && !XsDeviceId_isMtw1(thisPtr));
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents a legacy MTi-G. */
int XsDeviceId_isLegacyMtig(const XsDeviceId* thisPtr)
{
	return ((thisPtr->m_deviceId & XS_DID_TYPEH_MASK) == XS_DID_TYPEH_MTIG);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MTi-G. */
int XsDeviceId_isMtig(const XsDeviceId* thisPtr)
{
	return (((thisPtr->m_deviceId & XS_DID_TYPEH_MASK) == XS_DID_TYPEH_MTIG) || ((thisPtr->m_deviceId & XS_DID_MK4TYPE_MASK) == XS_DID_MK4TYPE_MT_700));
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MTMk4. */
int XsDeviceId_isMtMk4(const XsDeviceId* thisPtr)
{
	return ( ((thisPtr->m_deviceId & XS_DID_TYPEH_MASK) == XS_DID_TYPEH_MT_X0) ||
			 ((thisPtr->m_deviceId & XS_DID_TYPEH_MASK) == XS_DID_TYPEH_MT_X00) );
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MTMk4 10 series. */
int XsDeviceId_isMtMk4_X0(const struct XsDeviceId* thisPtr)
{
	return ((thisPtr->m_deviceId & XS_DID_TYPEH_MASK) == XS_DID_TYPEH_MT_X0);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MTMk4 10. */
int XsDeviceId_isMtMk4_10(const struct XsDeviceId* thisPtr)
{
	return ((thisPtr->m_deviceId & XS_DID_MK4TYPE_MASK) == XS_DID_MK4TYPE_MT_10);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MTMk4 20. */
int XsDeviceId_isMtMk4_20(const struct XsDeviceId* thisPtr)
{
	return ((thisPtr->m_deviceId & XS_DID_MK4TYPE_MASK) == XS_DID_MK4TYPE_MT_20);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MTMk4 30. */
int XsDeviceId_isMtMk4_30(const struct XsDeviceId* thisPtr)
{
	return ((thisPtr->m_deviceId & XS_DID_MK4TYPE_MASK) == XS_DID_MK4TYPE_MT_30);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MTMk4 100 series. */
int XsDeviceId_isMtMk4_X00(const struct XsDeviceId* thisPtr)
{
	return ((thisPtr->m_deviceId & XS_DID_TYPEH_MASK) == XS_DID_TYPEH_MT_X00);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MTMk4 100. */
int XsDeviceId_isMtMk4_100(const struct XsDeviceId* thisPtr)
{
	return ((thisPtr->m_deviceId & XS_DID_MK4TYPE_MASK) == XS_DID_MK4TYPE_MT_100);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MTMk4 200. */
int XsDeviceId_isMtMk4_200(const struct XsDeviceId* thisPtr)
{
	return ((thisPtr->m_deviceId & XS_DID_MK4TYPE_MASK) == XS_DID_MK4TYPE_MT_200);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MTMk4 300. */
int XsDeviceId_isMtMk4_300(const struct XsDeviceId* thisPtr)
{
	return ((thisPtr->m_deviceId & XS_DID_MK4TYPE_MASK) == XS_DID_MK4TYPE_MT_300);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MTMk4 700. */
int XsDeviceId_isMtMk4_700(const struct XsDeviceId* thisPtr)
{
	return ((thisPtr->m_deviceId & XS_DID_MK4TYPE_MASK) == XS_DID_MK4TYPE_MT_700);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MTx2 */
int XsDeviceId_isMtx2(const struct XsDeviceId* thisPtr)
{
	return XS_DID_MTX2(thisPtr->m_deviceId);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents an MTx */
int XsDeviceId_isMtx(const struct XsDeviceId* thisPtr)
{
	return XsDeviceId_isMtx2(thisPtr);
}

/*! \relates XsDeviceId
	\brief Test if this device ID represents any of the container devices such as Xbus Master and Awinda Station
*/
int XsDeviceId_isContainerDevice(const struct XsDeviceId* thisPtr)
{
	return	XsDeviceId_isBodyPack(thisPtr) ||
			XsDeviceId_isWirelessMaster(thisPtr) ||
			XsDeviceId_isXbusMaster(thisPtr);
}

/*!	\relates XsDeviceId
	\brief Test if the device ID has the broadcast bit set
*/
int XsDeviceId_containsBroadcast(const XsDeviceId* thisPtr)
{
	return ((thisPtr->m_deviceId & XS_DID_BROADCAST) != 0);
}

/*! \relates XsDeviceId
	\brief Test if this device ID \e is \e the broadcast id.
*/
int XsDeviceId_isBroadcast(const XsDeviceId* thisPtr)
{
	return (thisPtr->m_deviceId == XS_DID_BROADCAST);
}

/*! \relates XsDeviceId
	\brief Get a string with a readable representation of this device ID. */
void XsDeviceId_toString(const XsDeviceId* thisPtr, XsString* str)
{
	char device[9];
	sprintf(device, "%08X", thisPtr->m_deviceId);	//lint !e534
	XsString_assign(str, 8, device);
}

/*! \relates XsDeviceId
	\brief Read a device ID from the supplied string. */
void XsDeviceId_fromString(XsDeviceId* thisPtr, XsString const* str)
{
	int tmp = 0;
	assert(thisPtr && str && str->m_data);
	if (!thisPtr || !str || !str->m_data)
		return;
	if (sscanf(str->m_data, "%x", &tmp) == 1)
		thisPtr->m_deviceId = (uint32_t) tmp;
}

/*! \brief Swap the contents of \a a with those of \a b
*/
void XsDeviceId_swap(XsDeviceId* a, XsDeviceId* b)
{
	uint32_t tmp = a->m_deviceId;
	a->m_deviceId = b->m_deviceId;
	b->m_deviceId = tmp;
}

/*! \brief Returns true if \a a is equal to \a b or \a a is a type-specifier that matches \a b
*/
int XsDeviceId_contains(XsDeviceId const* a, XsDeviceId const* b)
{
	if (a == b)
		return 1;
	if (a->m_deviceId == b->m_deviceId)
		return 1;
	if (a->m_deviceId & XS_DID_ID_MASK) // NOTE: This produces incorrect results for device ids ending with 0000. See MVN-3876
		return 0;
	if ((a->m_deviceId & XS_DID_FULLTYPE_MASK) == (b->m_deviceId & XS_DID_FULLTYPE_MASK))
		return 1;
	return 0;
}

/*! \brief Returns true if the ID is just a device type, not an actual device ID */
int XsDeviceId_isType(XsDeviceId const* thisPtr)
{
	// true when we have a valid type (not a broadcast) and a 0 ID
	return	(thisPtr->m_deviceId & (XS_DID_FULLTYPE_MASK & ~XS_DID_BROADCAST))
			&& !(thisPtr->m_deviceId & XS_DID_ID_MASK);
}

/*! @} */
