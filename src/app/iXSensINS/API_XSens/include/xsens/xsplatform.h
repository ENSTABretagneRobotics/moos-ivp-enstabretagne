#ifndef XSPLATFORM_H
#define XSPLATFORM_H

#include "xstypesconfig.h"

#ifdef _WIN32
/// microsoft / windows
#include <windows.h>

#define XsIoHandle HANDLE

#else
/// gcc / linux
#include <stdint.h>

typedef int32_t XsIoHandle;

#endif

#endif	// file guard
