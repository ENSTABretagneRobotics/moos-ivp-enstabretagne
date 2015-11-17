#include "xsvector3.h"
#include <string.h>

//lint -e641 conversion from enum to int should not be a problem

/*! \class XsVector3
	\brief A class that represents a fixed size (3) vector
*/	

/*! \addtogroup cinterface C Interface
	@{
*/

/*! \relates XsVector3 \brief Init the %XsVector3 and copy the data from \a src into the vector if \a src is not null */
void XsVector3_construct(XsVector3* thisPtr, const XsReal* src)
{
	XsVector_ref(&thisPtr->m_vector, 3, (XsReal*) thisPtr->m_fixedData, XSDF_FixedSize);
	if (src)
		memcpy((XsReal*) thisPtr->m_fixedData, src, 3*sizeof(XsReal));
}

/*! \relates XsVector3 \brief Init the %XsVector3 and copy the data from \a src into the vector if \a src is not null */
void XsVector3_assign(XsVector3* thisPtr, const XsReal* src)
{
	if (src)
		memcpy((XsReal*) thisPtr->m_fixedData, (XsReal*) src, 3*sizeof(XsReal));
}

/*! \relates XsVector3 \brief Frees the XsVector3 */
void XsVector3_destruct(XsVector3* thisPtr)
{
	// don't do anything, no memory needs to be freed
	assert(thisPtr->m_vector.m_flags & XSDF_FixedSize);	
	(void)thisPtr;
}

/*! \relates XsVector3 \brief Copy the contents of the %XsVector3 to \a copy */
void XsVector3_copy(XsVector* copy, XsVector3 const* src)
{
	XsVector_copy(copy, &src->m_vector);
}

/*! @} */
