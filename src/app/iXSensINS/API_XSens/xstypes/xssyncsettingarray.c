#include "xssyncsettingarray.h"
#include "xssyncsetting.h"
#include <memory.h>
#include <string.h>

/*! \struct XsSyncSettingArray
	\brief A list of XsSyncSetting values
	\sa XsArray
*/

/*! \copydoc XsArrayDescriptor::itemCopy
	\note Specialization for XsSyncSetting*/
void copySyncSetting(XsSyncSetting* to, XsSyncSetting const* from)
{
	*to = *from;
}

/*! \copydoc XsArrayDescriptor::itemCompare
	\note Specialization for XsSyncSetting*/
int compareSyncSetting(XsSyncSetting const* a, XsSyncSetting const* b)
{
	return XsSyncSetting_compare(a, b);
}

//! \brief zero the pointer value
void zeroSyncSetting(XsSyncSetting* a)
{
	memset(a, 0, sizeof(XsSyncSetting));
	a->m_line = XSL_Invalid;
}

//! \brief Descriptor for XsSyncSettingArray
XsArrayDescriptor const g_xsSyncSettingArrayDescriptor = {
	//lint --e{64} ignore exact type mismatches here
	sizeof(XsSyncSetting),
	XSEXPCASTITEMSWAP XsSyncSetting_swap,
	XSEXPCASTITEMMAKE zeroSyncSetting,		// construct
	XSEXPCASTITEMCOPY copySyncSetting,		// copy construct
	0,										// destruct
	XSEXPCASTITEMCOPY copySyncSetting,
	XSEXPCASTITEMCOMP compareSyncSetting,
	XSEXPCASTRAWCOPY XsArray_rawCopy	// raw copy
};

/*! \copydoc XsArray_construct
	\note Specialization for XsSyncSettingArray
*/
void XsSyncSettingArray_construct(XsSyncSettingArray* thisPtr, XsSize count, XsSyncSetting const* src)
{
	XsArray_construct(thisPtr, &g_xsSyncSettingArrayDescriptor, count, src);
}
