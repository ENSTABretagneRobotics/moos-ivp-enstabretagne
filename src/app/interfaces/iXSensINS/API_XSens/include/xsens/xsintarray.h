#ifndef XSINTARRAY_H
#define XSINTARRAY_H

#include "xsarray.h"

#ifdef __cplusplus
extern "C" {
#endif

extern XsArrayDescriptor const XSTYPES_DLL_API g_xsIntArrayDescriptor;

#ifndef __cplusplus
#define XsIntArray_INITIALIZER	XSARRAY_INITIALIZER(&g_xsIntArrayDescriptor)

XSARRAY_STRUCT(XsIntArray, int);
typedef struct XsIntArray XsIntArray;

XSTYPES_DLL_API void XsIntArray_construct(XsIntArray* thisPtr, XsSize count, int const* src);
#else
} // extern "C"
#endif

#ifdef __cplusplus
struct XsIntArray : public XsArrayImpl<int, g_xsIntArrayDescriptor, XsIntArray> {
	//! \brief Constructs an XsIntArray
	inline explicit XsIntArray(XsSize sz = 0, int const* src = 0)
		 : ArrayImpl(sz, src)
	{
	}

	//! \brief Constructs an XsIntArray as a copy of \a other
	inline XsIntArray(XsIntArray const& other)
		 : ArrayImpl(other)
	{
	}

	//! \brief Constructs an XsIntArray that references the data supplied in \a ref
	inline explicit XsIntArray(int* ref, XsSize sz, XsDataFlags flags = XSDF_None)
		: ArrayImpl(ref, sz, flags)
	{
	}

#ifndef XSENS_NOITERATOR
	//! \brief Constructs an XsIntArray with the array bound by the supplied iterators \a beginIt and \a endIt
	template <typename Iterator>
	inline XsIntArray(Iterator beginIt, Iterator endIt)
		: ArrayImpl(beginIt, endIt)
	{
	}
#endif
};
#endif


#endif // file guard
