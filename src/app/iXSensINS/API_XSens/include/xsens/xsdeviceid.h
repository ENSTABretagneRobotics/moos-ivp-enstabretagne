#ifndef XSDEVICEID_H
#define XSDEVICEID_H

#include "xstypesconfig.h"
#include "pstdint.h"
#include "xsstring.h"
#ifdef __cplusplus
extern "C" {
#else
#define XSDEVICEID_INITIALIZER	{ 0 }
#endif

struct XsDeviceId;

XSTYPES_DLL_API void XsDeviceId_toString(struct XsDeviceId const* thisPtr, XsString* str);
XSTYPES_DLL_API void XsDeviceId_fromString(struct XsDeviceId* thisPtr, XsString const* str);
XSTYPES_DLL_API int XsDeviceId_isValid(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isXbusMaster(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isBodyPack(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isXbusMasterMotionTracker(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isWirelessMaster(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMtw1(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMtw2(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMtw(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMt9c(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMtig(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isLegacyMtig(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isAwinda1(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isAwinda2(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isAwinda(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isAwinda1Station(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isAwinda1Dongle(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isAwinda1Oem(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isAwinda2Station(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isAwinda2Dongle(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isAwinda2Oem(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isAwindaStation(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isAwindaDongle(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isAwindaOem(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isSyncStation(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMtMk4(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMtMk4_X0(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMtMk4_10(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMtMk4_20(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMtMk4_30(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMtMk4_X00(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMtMk4_100(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMtMk4_200(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMtMk4_300(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMtMk4_700(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMtx(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isMtx2(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isContainerDevice(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_containsBroadcast(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API int XsDeviceId_isBroadcast(struct XsDeviceId const* thisPtr);
XSTYPES_DLL_API uint32_t XsDeviceId_broadcast(void);
XSTYPES_DLL_API void XsDeviceId_swap(struct XsDeviceId* a, struct XsDeviceId* b);
XSTYPES_DLL_API int XsDeviceId_contains(struct XsDeviceId const* a, struct XsDeviceId const* b);
XSTYPES_DLL_API int XsDeviceId_isType(struct XsDeviceId const* thisPtr);

#ifdef __cplusplus
} // extern "C"
#endif

struct XsDeviceId {
#ifdef __cplusplus
	/*! \brief Constructor that creates an XsDeviceId from the supplied \a deviceId */
	inline XsDeviceId(uint32_t deviceId = 0)
		: m_deviceId(deviceId)
	{
	}
	/*! \brief Constructor that creates an XsDeviceId from the supplied XsDeviceId \a other */
	inline XsDeviceId(const XsDeviceId& other)
		: m_deviceId(other.m_deviceId)
	{
	}

	/*! \brief Assign the \a other deviceId to this XsDeviceId */
	inline const XsDeviceId& operator=(const XsDeviceId& other)
	{
		m_deviceId = other.m_deviceId;
		return *this;
	}
	/*! \brief Assign the \a deviceId to this XsDeviceId */
	inline const XsDeviceId& operator=(uint32_t deviceId)
	{
		m_deviceId = deviceId;
		return *this;
	}

	/*! \brief Returns the deviceId as an unsigned integer */
	inline uint32_t toInt() const
	{
		return m_deviceId;
	}

	/*! \brief Returns the deviceId as an XsString */
	inline XsString toString() const
	{
		XsString tmp;
		XsDeviceId_toString(this, &tmp);
		return tmp;
	}

	/*! \brief Fills the deviceId with the parsed value from the supplied string \a s */
	inline void fromString(XsString const& s)
	{
		XsDeviceId_fromString(this, &s);
	}

	/*! \brief \copybrief XsDeviceId_isValid */
	inline bool isValid() const
	{
		return 0 != XsDeviceId_isValid(this);
	}
	/*! \brief \copybrief XsDeviceId_isXbusMaster */
	inline bool isXbusMaster() const
	{
		return 0 != XsDeviceId_isXbusMaster(this);
	}
	/*! \brief \copybrief XsDeviceId_isAwinda1 */
	inline bool isAwinda1() const
	{
		return 0 != XsDeviceId_isAwinda1(this);
	}
	/*! \brief \copybrief XsDeviceId_isAwinda2 */
	inline bool isAwinda2() const
	{
		return 0 != XsDeviceId_isAwinda2(this);
	}
	/*! \brief \copybrief XsDeviceId_isAwinda */
	inline bool isAwinda() const
	{
		return 0 != XsDeviceId_isAwinda(this);
	}
	/*! \brief \copybrief XsDeviceId_isXbusMasterMotionTracker */
	inline bool isXbusMasterMotionTracker() const
	{
		return 0 != XsDeviceId_isXbusMasterMotionTracker(this);
	}
	/*! \brief \copybrief XsDeviceId_isMtw1 */
	inline bool isMtw1() const
	{
		return 0 != XsDeviceId_isMtw1(this);
	}
	/*! \brief \copybrief XsDeviceId_isMtw2 */
	inline bool isMtw2() const
	{
		return 0 != XsDeviceId_isMtw2(this);
	}
	/*! \brief \copybrief XsDeviceId_isMtw */
	inline bool isMtw() const
	{
		return 0 != XsDeviceId_isMtw(this);
	}
	/*! \brief \copybrief XsDeviceId_isMt9c */
	inline bool isMt9c() const
	{
		return 0 != XsDeviceId_isMt9c(this);
	}
	/*! \brief \copybrief XsDeviceId_isLegacyMtig */
	inline bool isLegacyMtig() const
	{
		return 0 != XsDeviceId_isLegacyMtig(this);
	}
	/*! \brief \copybrief XsDeviceId_isMtig */
	inline bool isMtig() const
	{
		return 0 != XsDeviceId_isMtig(this);
	}
	/*! \brief \copybrief XsDeviceId_isMtMk4 */
	inline bool isMtMk4() const
	{
		return 0 != XsDeviceId_isMtMk4(this);
	}
	/*! \brief \copybrief XsDeviceId_isMtMk4_X0 */
	inline bool isMtMk4_X0() const
	{
		return 0 != XsDeviceId_isMtMk4_X0(this);
	}
	/*! \brief \copybrief XsDeviceId_isMtMk4_10 */
	inline bool isMtMk4_10() const
	{
		return 0 != XsDeviceId_isMtMk4_10(this);
	}
	/*! \brief \copybrief XsDeviceId_isMtMk4_20 */
	inline bool isMtMk4_20() const
	{
		return 0 != XsDeviceId_isMtMk4_20(this);
	}
	/*! \brief \copybrief XsDeviceId_isMtMk4_30 */
	inline bool isMtMk4_30() const
	{
		return 0 != XsDeviceId_isMtMk4_30(this);
	}
	/*! \brief \copybrief XsDeviceId_isMtMk4_X00 */
	inline bool isMtMk4_X00() const
	{
		return 0 != XsDeviceId_isMtMk4_X00(this);
	}
	/*! \brief \copybrief XsDeviceId_isMtMk4_100 */
	inline bool isMtMk4_100() const
	{
		return 0 != XsDeviceId_isMtMk4_100(this);
	}
	/*! \brief \copybrief XsDeviceId_isMtMk4_200 */
	inline bool isMtMk4_200() const
	{
		return 0 != XsDeviceId_isMtMk4_200(this);
	}
	/*! \brief \copybrief XsDeviceId_isMtMk4_300 */
	inline bool isMtMk4_300() const
	{
		return 0 != XsDeviceId_isMtMk4_300(this);
	}
	/*! \brief \copybrief XsDeviceId_isMtMk4_700 */
	inline bool isMtMk4_700() const
	{
		return 0 != XsDeviceId_isMtMk4_700(this);
	}
	/*! \brief \copybrief XsDeviceId_isMtx2 */
	inline bool isMtx2() const
	{
		return 0 != XsDeviceId_isMtx2(this);
	}
	/*! \brief \copybrief XsDeviceId_isMtx */
	inline bool isMtx() const
	{
		return 0 != XsDeviceId_isMtx(this);
	}
	/*! \brief \copybrief XsDeviceId_isWirelessMaster */
	inline bool isWirelessMaster() const
	{
		return 0 != XsDeviceId_isWirelessMaster(this);
	}
	/*! \brief \copybrief XsDeviceId_isAwinda1Station */
	inline bool isAwinda1Station() const
	{
		return 0 != XsDeviceId_isAwinda1Station(this);
	}
	/*! \brief \copybrief XsDeviceId_isAwinda1Dongle */
	inline bool isAwinda1Dongle() const
	{
		return 0 != XsDeviceId_isAwinda1Dongle(this);
	}
	/*! \brief \copybrief XsDeviceId_isAwinda1Oem */
	inline bool isAwinda1Oem() const
	{
		return 0 != XsDeviceId_isAwinda1Oem(this);
	}
	/*! \brief \copybrief XsDeviceId_isAwinda2Station */
	inline bool isAwinda2Station() const
	{
		return 0 != XsDeviceId_isAwinda2Station(this);
	}
	/*! \brief \copybrief XsDeviceId_isAwinda2Dongle */
	inline bool isAwinda2Dongle() const
	{
		return 0 != XsDeviceId_isAwinda2Dongle(this);
	}
	/*! \brief \copybrief XsDeviceId_isAwinda2Oem */
	inline bool isAwinda2Oem() const
	{
		return 0 != XsDeviceId_isAwinda2Oem(this);
	}
	/*! \brief \copybrief XsDeviceId_isAwindaStation */
	inline bool isAwindaStation() const
	{
		return 0 != XsDeviceId_isAwindaStation(this);
	}
	/*! \brief \copybrief XsDeviceId_isAwindaDongle */
	inline bool isAwindaDongle() const
	{
		return 0 != XsDeviceId_isAwindaDongle(this);
	}
	/*! \brief \copybrief XsDeviceId_isAwindaOem */
	inline bool isAwindaOem() const
	{
		return 0 != XsDeviceId_isAwindaOem(this);
	}
	/*! \brief \copybrief XsDeviceId_isSyncStation */
	inline bool isSyncStation() const
	{
		return 0 != XsDeviceId_isSyncStation(this);
	}
	/*! \brief \copybrief XsDeviceId_isBodyPack */
	inline bool isBodyPack() const
	{
		return 0 != XsDeviceId_isBodyPack(this);
	}
	/*! \brief \copybrief XsDeviceId_isContainerDevice */
	inline bool isContainerDevice() const
	{
		return 0 != XsDeviceId_isContainerDevice(this);
	}
	/*! \brief \copybrief XsDeviceId_containsBroadcast */
	inline bool containsBroadcast() const
	{
		return 0 != XsDeviceId_containsBroadcast(this);
	}
	/*! \brief \copybrief XsDeviceId_isBroadcast */
	inline bool isBroadcast() const
	{
		return 0 != XsDeviceId_isBroadcast(this);
	}

	/*! \brief Returns true if the \a other deviceId matches this deviceId */
	inline bool operator==(const XsDeviceId& other) const { return m_deviceId == other.m_deviceId; }
	/*! \brief Returns true if the \a other deviceId does not match this deviceId */
	inline bool operator!=(const XsDeviceId& other) const { return m_deviceId != other.m_deviceId; }
	/*! \brief Returns true if this deviceId is less than the \a other deviceId */
	inline bool operator<(const XsDeviceId& other) const { return m_deviceId < other.m_deviceId; }
	/*! \brief Returns true if this deviceId is less or equal to the \a other deviceId */
	inline bool operator<=(const XsDeviceId& other) const { return m_deviceId <= other.m_deviceId; }
	/*! \brief Returns true if this deviceId is larger than the \a other deviceId */
	inline bool operator>(const XsDeviceId& other) const { return m_deviceId > other.m_deviceId; }
	/*! \brief Returns true if this deviceId is larger or equal to the \a other deviceId */
	inline bool operator>=(const XsDeviceId& other) const { return m_deviceId >= other.m_deviceId; }

	/*! \brief Creates and returns a XsDeviceId representing the broadcast deviceId */
	static XsDeviceId broadcast()
	{
		return XsDeviceId(XsDeviceId_broadcast());
	}

	/*! \brief Returns true if this is equal to \a other or this is a type-specifier that matches \a other */
	bool contains(const XsDeviceId& other) const
	{
		return 0 != XsDeviceId_contains(this, &other);
	}

	/*! \brief Returns true if the ID is just a device type, not an actual device ID */
	bool isType() const
	{
		return 0 != XsDeviceId_isType(this);
	}

private:
#endif
	uint32_t m_deviceId;	//!< The actual device id
};

typedef struct XsDeviceId XsDeviceId;

#if defined(__cplusplus) && !defined(XSENS_NO_STL)
namespace std {
template<typename _CharT, typename _Traits>
basic_ostream<_CharT, _Traits>& operator<<(basic_ostream<_CharT, _Traits>& o, XsDeviceId const& xd)
{
	return (o << xd.toString());
}
}

inline XsString& operator<<(XsString& o, XsDeviceId const& xd)
{
	o.append(xd.toString());
	return o;
}

#endif

#endif // file guard
