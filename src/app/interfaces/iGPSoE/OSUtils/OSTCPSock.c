/***************************************************************************************************************:')

OSTCPSock.c

TCP sockets handling.
You must call InitNet() before using any network function.

Fabrice Le Bars

Created : 2006-2007

Version status : Not finished

***************************************************************************************************************:)*/

// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "OSTCPSock.h"
