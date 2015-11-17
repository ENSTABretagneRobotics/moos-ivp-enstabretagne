/***************************************************************************************************************:')

OSThread.h

Threads handling.

Fabrice Le Bars

Created : 2007

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

#include "OSThread.h"

#ifndef _WIN32
//// Portable programs should use sched_get_priority_min and sched_get_priority_max to 
//// find the range of priorities supported for a particular policy.
//int THREAD_PRIORITY_TIME_CRITICAL = 99;
//int THREAD_PRIORITY_HIGHEST = 70;
//int THREAD_PRIORITY_ABOVE_NORMAL = 60;
//int THREAD_PRIORITY_NORMAL = 50;
//int THREAD_PRIORITY_BELOW_NORMAL = 40;
//int THREAD_PRIORITY_LOWEST = 1;
//int THREAD_PRIORITY_IDLE = 0;
#endif // _WIN32

#ifdef ENABLE_CANCEL_THREAD
#ifdef _WIN32
VOID CALLBACK _CancelThreadAPC(ULONG_PTR dwParam)
{
	//TerminateThread((HANDLE)dwParam, EXIT_CANCELED_THREAD);
	//CloseHandle((HANDLE)dwParam);
	UNREFERENCED_PARAMETER(dwParam);
#ifdef USE_CREATE_THREAD
	ExitThread(EXIT_CANCELED_THREAD);
#else
	_endthreadex(EXIT_CANCELED_THREAD);
#endif // USE_CREATE_THREAD
}
#endif // _WIN32
#endif // ENABLE_CANCEL_THREAD

//THREAD_PROC_RETURN_VALUE _ThreadFunction(void* pParam)
//{
//	DWORD ThreadResult = NULL;
//	THREADPARAM* pThreadParam = (THREADPARAM*)pParam;
//
//	// Specific functions that must be called from the thread.
//
//	// When initially created, a thread is synchronously cancelable.
//	// Here enable the current thread to be canceled (killed) asynchronously (i.e. immediately).
//	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
//	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
//
//	ThreadResult = pThreadParam.ThreadFunction(pThreadParam.pThreadParam);
//
//	free(pThreadParam);
//	
//	return ThreadResult;
//}
