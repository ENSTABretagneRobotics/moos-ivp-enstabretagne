#ifndef XSSTRINGARRAY_H
#define XSSTRINGARRAY_H

#include "xsarray.h"

#ifdef __cplusplus
#include "xsstring.h"
extern "C" {
#endif

extern XsArrayDescriptor const XSTYPES_DLL_API g_xsStringArrayDescriptor;

#ifndef __cplusplus
#define XSSTRINGARRAY_INITIALIZER	XSARRAY_INITIALIZER(&g_xsStringArrayDescriptor)
struct XsString;

XSARRAY_STRUCT(XsStringArray, struct XsString);
typedef struct XsStringArray XsStringArray;

XSTYPES_DLL_API void XsStringArray_construct(XsStringArray* thisPtr, XsSize count, struct XsString const* src);
#define XsStringArray_destruct(thisPtr)		XsArray_destruct(thisPtr)
#endif
XSTYPES_DLL_API void XsStringArray_fromSplicedString(struct XsStringArray* thisPtr, struct XsString const* src, struct XsString const* separators);
XSTYPES_DLL_API void XsStringArray_join(struct XsStringArray const* thisPtr, struct XsString* result, struct XsString const* separator);

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
struct XsStringArray : public XsArrayImpl<XsString, g_xsStringArrayDescriptor, XsStringArray> {
	//! \brief Constructs an XsStringArray
	inline explicit XsStringArray(XsSize sz = 0, XsString const* src = 0)
		 : ArrayImpl(sz, src)
	{
	}

	//! \brief Constructs an XsStringArray as a copy of \a other
	inline XsStringArray(XsStringArray const& other)
		 : ArrayImpl(other)
	{
	}

	//! \brief Constructs an XsStringArray that references the data supplied in \a ref
	inline explicit XsStringArray(XsString* ref, XsSize sz, XsDataFlags flags = XSDF_None)
		: ArrayImpl(ref, sz, flags)
	{
	}

#ifndef XSENS_NOITERATOR
	//! \brief Constructs an XsStringArray with the array bound by the supplied iterators \a beginIt and \a endIt
	template <typename Iterator>
	inline XsStringArray(Iterator beginIt, Iterator endIt)
		: ArrayImpl(beginIt, endIt)
	{
	}
#endif

	/*! \brief Join the non-empty strings contained in the XsStringArray into one XsString, separating each item with \a separator
		\param separator An optional separator string to put between substrings
		\return The joined string
	*/
	XsString join(XsString const& separator) const
	{
		XsString tmp;
		XsStringArray_join(this, &tmp, &separator);
		return tmp;
	}

	/*! \copydoc XsStringArray_fromSplicedString */
	void fromSplicedString(XsString const& src, XsString const& separators)
	{
		XsStringArray_fromSplicedString(this, &src, &separators);
	}

};
#endif

#endif // file guard
