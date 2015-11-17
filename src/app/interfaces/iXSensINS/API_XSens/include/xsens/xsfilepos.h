#ifndef XSFILEPOS_H
#define XSFILEPOS_H

/*! \addtogroup cinterface C Interface
	@{
*/

/*!	\typedef XsFilePos
	\brief The type that is used for positioning inside a file
*/
/*!	\typedef XsIoHandle
	\brief The type that is used for low-level identification of an open I/O device
*/
/*!	\typedef XsFileHandle
	\brief The type that is used for low-level identification of an open file
*/

/*! @} */

#include <stdio.h>
#ifdef _WIN32
#ifndef _PSTDINT_H_INCLUDED
#	include "pstdint.h"
#endif
typedef __int64 XsFilePos;
#ifndef HANDLE
#	include <windows.h>
#endif
typedef HANDLE XsIoHandle;
#else
#include <sys/types.h>
/* off_t is practically guaranteed not to be 64 bits on non64 bit systems.
   We'd better explicitly use __off64_t to be sure of it's size.
*/
#if defined(__off64_t_defined)
typedef  __off64_t	XsFilePos;
#else
typedef int64_t XsFilePos;
#endif
typedef int32_t XsIoHandle;
#endif
typedef FILE XsFileHandle;

#endif	// file guard
