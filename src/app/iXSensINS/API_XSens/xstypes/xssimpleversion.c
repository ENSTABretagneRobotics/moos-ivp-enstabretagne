#include "xssimpleversion.h"
#include "xsstring.h"
#include <stdio.h>

/*! \class XsSimpleVersion
	\brief A class to store version information
*/

/*! \addtogroup cinterface C Interface
	@{
*/

/*! \relates XsSimpleVersion \brief Test if this is a null-version. */
int XsSimpleVersion_empty(const XsSimpleVersion* thisPtr)
{
	return thisPtr->m_major == 0 && thisPtr->m_minor == 0 && thisPtr->m_revision == 0;
}

/*! \relates XsSimpleVersion
	\brief Get a string with the version expressed in a readable format.
*/
void XsSimpleVersion_toString(const XsSimpleVersion* thisPtr, XsString* version)
{
	char buffer[256];
	int chars;

	assert(thisPtr);
	assert(version);

	chars = sprintf(buffer, "%d.%d.%d", (int) thisPtr->m_major, (int) thisPtr->m_minor, (int) thisPtr->m_revision);
	if (chars > 0 && chars < 256)
		XsString_assign(version, chars, buffer);
	else
		XsString_assign(version, 0, 0);
}

/*! \relates XsSimpleVersion
	\brief Set the version to the values in the string
*/
void XsSimpleVersion_fromString(XsSimpleVersion* thisPtr, XsString const* version)
{
	int major = 0, minor = 0, revision = 0;

	assert(thisPtr);
	thisPtr->m_major = 0;
	thisPtr->m_minor = 0;
	thisPtr->m_revision = 0;
	if (!version || XsString_empty(version))
		return;

	(void) sscanf(version->m_data, "%d.%d.%d", &major, &minor, &revision);
	thisPtr->m_major = (uint8_t) major;
	thisPtr->m_minor = (uint8_t) minor;
	thisPtr->m_revision = (uint8_t) revision;
}

/*! \brief Swap the contents of \a a with those of \a b
*/
void XsSimpleVersion_swap(struct XsSimpleVersion* a, struct XsSimpleVersion* b)
{
	XsSimpleVersion tmp = *a;
	*a = *b;
	*b = tmp;
}

/*! \brief Compare two XsSimpleVersion objects.
	\param a The left hand side of the comparison
	\param b The right hand side of the comparison
	\return 0 when they're equal
*/
int XsSimpleVersion_compare(XsSimpleVersion const* a, XsSimpleVersion const* b)
{
	return a->m_major != b->m_major || a->m_minor != b->m_minor || a->m_revision != b->m_revision;
}

/*! @} */
