/***************************************************************************************************************:')

OSProcess.h

Processes handling.

Fabrice Le Bars

Created : 2010-05-24

Version status : Tested some parts

***************************************************************************************************************:)*/

// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "OSProcess.h"

/*
#ifndef _WIN32
int REALTIME_PRIORITY_CLASS = 99;
int HIGH_PRIORITY_CLASS = 70;
int ABOVE_NORMAL_PRIORITY_CLASS = 60;
int NORMAL_PRIORITY_CLASS = 50;
int BELOW_NORMAL_PRIORITY_CLASS = 40;
int IDLE_PRIORITY_CLASS = 0;
#endif // _WIN32
*/

//#ifndef _WIN32
//void _ChildEndHandler(int sig)
//{
//  wait(NULL);
//  return;
//}
//#endif // _WIN32
