/***************************************************************************************************************:')

OSUtils.h

Windows 2000/XP/2003/Vista/2008/7 - Linux 2.6.35 32 bits abstraction layer to be used in combination with the C 
standard library with Microsoft Visual Studio 2005/2008/2010 and Borland C++ Builder 5 for Windows and GCC 4.4.5
for Linux (note that most of the tests are primarily done with Microsoft Visual Studio 2008 SP1 on Windows XP SP3 
and Ubuntu 10.10).

Fabrice Le Bars

Created : 2008-07-05

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

#ifndef OSUTILS_H
#define OSUTILS_H

#include "OSCore.h" 
#include "OSTime.h" 
#include "OSThread.h" 
#include "OSCriticalSection.h" 
#include "OSIPMutex.h" 
#include "OSSem.h" 
#include "OSIPSem.h" 
#include "OSEv.h" 
#include "OSIPEv.h" 
#include "OSTimer.h" 
#include "OSProcess.h" 
#include "OSRS232Port.h" 
#include "OSNet.h" 
#include "OSTCPSock.h" 
#include "OSFdList.h" 
#include "OSMisc.h" 

#endif // OSUTILS_H
