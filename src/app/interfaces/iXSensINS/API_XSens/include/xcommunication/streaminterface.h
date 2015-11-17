#ifndef STREAMINTERFACE_H
#define STREAMINTERFACE_H

#include "iointerface.h"
#include <xsens/xsresultvalue.h>

/*! \brief A stream interface
	\details This class provides an interface for dealing with streaming I/O devices, such as a COM
	port or a USB port.
*/
class StreamInterface : public IoInterface
{
public:
	/*! \brief Destroy the stream interface
	*/
	inline ~StreamInterface() {}

	/*! \brief Set the read/write timeout to \a ms
		A timeout of 0 means non-blocking operation of writeData() and readData().
	*/
	virtual	XsResultValue setTimeout (uint32_t ms) = 0;

	/*! \brief The timeout used for read/write operations
	*/
	virtual uint32_t getTimeout (void) const = 0;

	XSENS_DISABLE_COPY(StreamInterface)
protected:
	/*! \brief Create a stream interface
	*/
	inline StreamInterface() {}
};

#endif	// file guard
