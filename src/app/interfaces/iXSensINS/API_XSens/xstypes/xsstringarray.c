#include "xsstringarray.h"
#include "xsstring.h"

/*! \struct XsStringArray
	\brief A list of XsString values
	\sa XsArray
*/

//! \brief Descriptor for XsStringArray
XsArrayDescriptor const g_xsStringArrayDescriptor = {
	//lint --e{64} ignore exact type mismatches here
	sizeof(XsString),
	XSEXPCASTITEMSWAP XsArray_swap,
	XSEXPCASTITEMMAKE XsString_construct,
	XSEXPCASTITEMCOPY XsArray_copyConstruct,
	XSEXPCASTITEMMAKE XsArray_destruct,
	XSEXPCASTITEMCOPY XsArray_copy,
	XSEXPCASTITEMCOMP XsArray_compare,
	0
};

/*! \copydoc XsArray_construct
	\note Specialization for XsStringArray
*/
void XsStringArray_construct(XsStringArray* thisPtr, XsSize count, XsString const* src)
{
	XsArray_construct(thisPtr, &g_xsStringArrayDescriptor, count, src);
}

/*! \relates XsStringArray
	\brief Splice the supplied string and put the resulting substrings in the string array
	\details The source string will be searched for instances of characters in \a separators and spliced whereever
	one was found. The spliced list will not include characters in \a separators and empty substrings will be
	discarded.
	\param src The source string to splice
	\param separators A list of separator characters that will be used to splice \a src.
*/
void XsStringArray_fromSplicedString(struct XsStringArray* thisPtr, struct XsString const* src, struct XsString const* separators)
{
	XsString s;
	XsString_construct(&s);
	XsArray_destruct(thisPtr);
	if (src->m_size > 0)
	{
		// check against 1 because an empty string can either be size 0 or size 1 (just the null-terminator)
		if (separators->m_size <= 1)
		{
			// no separator
			XsArray_insert(thisPtr, 0, 1, src);
		}
		else
		{
			char const* sep = (char const*) separators->m_data;
			char const* idx = (char const*) src->m_data;
			char const* newIdx = strpbrk(idx, sep);
			while (newIdx && *idx)
			{
				if (newIdx != idx)
				{
					XsString_assign(&s, newIdx-idx, idx);
					XsArray_insert(thisPtr, thisPtr->m_size, 1, &s);
				}
				idx = newIdx+1;
				newIdx = strpbrk(idx, sep);
			}
			if (*idx)
			{
				XsString_assignCharArray(&s, idx);
				XsArray_insert(thisPtr, thisPtr->m_size, 1, &s);
			}
		}
	}
	XsString_destruct(&s);
}

/*! \relates XsStringArray
	\brief Join the string array into a single string, inserting \a separator between substrings.
	\param result The result of the join
	\param separator The separator to insert between successive items
*/
void XsStringArray_join(struct XsStringArray const* thisPtr, struct XsString* result, struct XsString const* separator)
{
	// determine required buffer size
	XsSize i;
	XsSize chars = (thisPtr->m_size ? (thisPtr->m_size-1) : 0) * (separator->m_size ? separator->m_size-1 : 0);
	for (i = 0; i < thisPtr->m_size; ++i)
	{
		XsSize sz = ((const XsString*)XsArray_at(thisPtr, i))->m_size;
		chars += (sz ? sz-1 : 0);
	}

	XsArray_destruct(result);
	if (chars)
	{
		XsArray_reserve(result, chars+1);
		for (i = 0; i < thisPtr->m_size; ++i)
		{
			const XsString* s = (const XsString*)XsArray_at(thisPtr, i);
			if (s->m_size > 1)
			{
				if (result->m_size > 1)
					XsString_append(result, separator);
				XsString_append(result, s);
			}
		}
	}
}
