#ifndef XSSIMPLEVERSION_H
#define XSSIMPLEVERSION_H

#include "xstypesconfig.h"
#include "xsversion.h"
#include "xsstring.h"

typedef struct XsSimpleVersion XsSimpleVersion;

#ifdef __cplusplus
extern "C" {
#else
#define XSSIMPLEVERSION_INITIALIZER { 0, 0, 0 }
#endif

XSTYPES_DLL_API int XsSimpleVersion_empty(const XsSimpleVersion* thisPtr);
XSTYPES_DLL_API void XsSimpleVersion_toString(const XsSimpleVersion* thisPtr, XsString* version);
XSTYPES_DLL_API void XsSimpleVersion_fromString(XsSimpleVersion* thisPtr, const XsString* version);
XSTYPES_DLL_API void XsSimpleVersion_swap(XsSimpleVersion* a, XsSimpleVersion* b);
XSTYPES_DLL_API int XsSimpleVersion_compare(XsSimpleVersion const* a, XsSimpleVersion const* b);

#ifdef __cplusplus
} // extern "C"
#endif

struct XsSimpleVersion {
#ifdef __cplusplus
	//! \brief Constructs a simple-version object using the supplied parameters or an empty version object if no parameters are given.
	explicit XsSimpleVersion(int vmaj = 0, int vmin = 0, int vrev = 0)
		: m_major((uint8_t) vmaj)
		, m_minor((uint8_t) vmin)
		, m_revision((uint8_t) vrev)
	{}

	//! \brief Constructs a simple-version object based upon the \a other object
	XsSimpleVersion(const XsSimpleVersion& other)
		: m_major(other.m_major)
		, m_minor(other.m_minor)
		, m_revision(other.m_revision)
	{}

	//! \brief Constructs a version object based upon the \a other XsVersion object
	explicit XsSimpleVersion(const XsVersion& other)
	{
		m_major = other.major();
		m_minor = other.minor();
		m_revision = other.revision();
	}

	//! \brief Constructs a version object based upon the verison contained by \a vString
	explicit XsSimpleVersion(const XsString& vString)
	{
		XsSimpleVersion_fromString(this, &vString);
	}

	//! \brief Assign the simple-version from the \a other object
	XsSimpleVersion& operator = (const XsSimpleVersion& other)
	{
		m_major = other.m_major;
		m_minor = other.m_minor;
		m_revision = other.m_revision;
		return *this;
	}

	//! \brief Assign the simple-version from the \a other XsVersion object
	XsSimpleVersion& operator = (const XsVersion& other)
	{
		m_major = other.major();	
		m_minor = other.minor();
		m_revision = other.revision();
		return *this;
	}

	/*! \brief Test if the \a other simple-version is equal to this. */
	inline bool operator == (const XsSimpleVersion& other) const
	{
		return !XsSimpleVersion_compare(this, &other);
	}

	/*! \brief Test if the \a other simple-version is different to this. */
	inline bool operator != (const XsSimpleVersion& other) const
	{
		if (m_major != other.m_major || m_minor != other.m_minor || m_revision != other.m_revision)
			return true;

		return false;
	}

	//! \brief \copybrief XsSimpleVersion_empty
	inline bool empty() const
	{
		return 0 != XsSimpleVersion_empty(this);
	}
	
	//! \brief \copybrief XsSimpleVersion_toString
	inline XsString toString() const
	{
		XsString tmp;
		XsSimpleVersion_toString(this, &tmp);
		return tmp;
	}

	//! \brief Return the \e major part of the version
	inline int major() const { return (int) m_major; }
	//! \brief Return the \e minor part of the version
	inline int minor() const { return (int) m_minor; }
	//! \brief Return the \e revision part of the version
	inline int revision() const { return (int) m_revision; }

private:
#endif
	uint8_t m_major;			//!< The major part of the version number 
	uint8_t m_minor;			//!< The minor part of the version number
	uint8_t m_revision;			//!< The revision number of the version	
};

#endif // file guard
