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

#ifndef OSTHREAD_H
#define OSTHREAD_H

#include "OSTime.h"

/*
Debug macros specific to OSThread.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSTHREAD
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSTHREAD
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSTHREAD
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSTHREAD
#	define PRINT_DEBUG_MESSAGE_OSTHREAD(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSTHREAD(params)
#endif // _DEBUG_MESSAGES_OSTHREAD

#ifdef _DEBUG_WARNINGS_OSTHREAD
#	define PRINT_DEBUG_WARNING_OSTHREAD(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSTHREAD(params)
#endif // _DEBUG_WARNINGS_OSTHREAD

#ifdef _DEBUG_ERRORS_OSTHREAD
#	define PRINT_DEBUG_ERROR_OSTHREAD(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSTHREAD(params)
#endif // _DEBUG_ERRORS_OSTHREAD

#ifdef _WIN32
#ifndef USE_CREATE_THREAD
#include <process.h>
#endif // USE_CREATE_THREAD
#else 
#include <pthread.h>
//#include <sched.h>
//#include <sys/resource.h>
#endif // _WIN32

#define MAX_THREAD_TIMEOUT (LONG_MAX-2)

// Default stack size for a thread (in bytes).
#define DEFAULT_THREAD_STACK_SIZE 1048576

#ifdef _WIN32
typedef DWORD THREAD_IDENTIFIER;

#ifdef USE_CREATE_THREAD
#define THREAD_PROC_RETURN_VALUE DWORD WINAPI
typedef LPTHREAD_START_ROUTINE PTHREAD_PROC;
#else
#if defined(__cplusplus) && defined(_M_CEE)
#define THREAD_PROC_RETURN_VALUE unsigned __clrcall
typedef unsigned (__clrcall *PTHREAD_PROC)(void*);
#else
#define THREAD_PROC_RETURN_VALUE unsigned __stdcall
typedef unsigned (__stdcall *PTHREAD_PROC)(void*);
#endif // defined(__cplusplus) && defined(_M_CEE)
#endif // USE_CREATE_THREAD
#else
typedef pthread_t THREAD_IDENTIFIER;

#define THREAD_PROC_RETURN_VALUE void*
typedef void* (*PTHREAD_PROC)(void*);

// These priorities are only valid for Linux (might be different on other UNIX-like OS)
// and only THREAD_PRIORITY_IDLE is valid for a scheduling policy SCHED_OTHER whereas 
// only the values between 1 and 99 are valid for the realtime policies SCHED_RR and 
// SCHED_FIFO.
#define THREAD_PRIORITY_TIME_CRITICAL 99
#define THREAD_PRIORITY_HIGHEST 70
#define THREAD_PRIORITY_ABOVE_NORMAL 60
#define THREAD_PRIORITY_NORMAL 50
#define THREAD_PRIORITY_BELOW_NORMAL 40
#define THREAD_PRIORITY_LOWEST 1
#define THREAD_PRIORITY_IDLE 0

//// Portable programs should use sched_get_priority_min and sched_get_priority_max to 
//// find the range of priorities supported for a particular policy.
//extern int THREAD_PRIORITY_TIME_CRITICAL;
//extern int THREAD_PRIORITY_HIGHEST;
//extern int THREAD_PRIORITY_ABOVE_NORMAL;
//extern int THREAD_PRIORITY_NORMAL;
//extern int THREAD_PRIORITY_BELOW_NORMAL;
//extern int THREAD_PRIORITY_LOWEST;
//extern int THREAD_PRIORITY_IDLE;
#endif // _WIN32

/*
Create a thread with default attributes.
The thread should be joinable (i.e. WaitForThread() should be called to wait for 
its end).
To make it detached, call DetachThread() after CreateDefaultThread().
KillThread() or CancelThread() can also be called (whatever the joinable or detached 
state, instead of WaitForThread() or DetachThread()), but the use of these functions is 
not recommended in most of the cases.

PTHREAD_PROC pThreadProc : (IN) Pointer to the application-defined 
function to be executed by the thread. This function should have this prototype : 
THREAD_PROC_RETURN_VALUE function(void* pParam);
void* pParam : (IN) Pointer to a variable to be passed to the thread.
THREAD_IDENTIFIER* pThreadId : (INOUT) Valid pointer that will receive the 
thread identifier.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int CreateDefaultThread(PTHREAD_PROC pThreadProc, void* pParam, THREAD_IDENTIFIER* pThreadId)
{
#ifdef _WIN32
	// CreateThread is not advised when used in combination with the C standard library...
	// But it is not clear whether it is a problem in our case...
#ifdef USE_CREATE_THREAD
	HANDLE hThread = CreateThread( 
		NULL, // Default security attributes.
		DEFAULT_THREAD_STACK_SIZE, // Initial size of the stack, in bytes.  
		pThreadProc, // Thread function. 
		pParam, // Argument to thread function. 
		0, // Use default creation flags. 
		pThreadId); // Returns the thread identifier. 

	if (hThread == NULL)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("CreateDefaultThread error (%s) : %s"
			"(pThreadProc=%#x, pParam=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pThreadProc, pParam));
		return EXIT_FAILURE;
	}
#else
	HANDLE hThread = (HANDLE)_beginthreadex( 
		NULL, // Default security attributes.
		DEFAULT_THREAD_STACK_SIZE, // Initial size of the stack, in bytes.  
		pThreadProc, // Thread function. 
		pParam, // Argument to thread function. 
		0, // Use default creation flags. 
		(unsigned int*)pThreadId); // Returns the thread identifier. 

	if (hThread == NULL)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("CreateDefaultThread error (%s) : %s"
			"(pThreadProc=%#x, pParam=%#x)\n", 
			strtime_m(), 
			"_beginthreadex failed. ", 
			pThreadProc, pParam));
		return EXIT_FAILURE;
	}
#endif // USE_CREATE_THREAD

	// Close the thread handle (this does not terminate the thread and makes the handle 
	// returned by CreateThread() be closed automatically and invalid when the thread terminates,
	// if no other handles to it are opened. Anyway, all the resources used by the thread will
	// be released when it is terminated and all the handles to it are closed).
	if (!CloseHandle(hThread))
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("CreateDefaultThread error (%s) : %s"
			"(pThreadProc=%#x, pParam=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pThreadProc, pParam));
		return EXIT_FAILURE;
	}
#else
	pthread_attr_t attr;
	int policy = 0;
	struct sched_param param;
	//int prio_max = 0;
	//int prio_min = 0;

	// Initialize thread attribute.
	if (pthread_attr_init(&attr) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("CreateDefaultThread error (%s) : %s"
			"(pThreadProc=%#x, pParam=%#x)\n", 
			strtime_m(), 
			"pthread_attr_init failed. ", 
			pThreadProc, pParam));
		return EXIT_FAILURE;
	}

	// Set a default stacksize.
	if (pthread_attr_setstacksize(&attr, DEFAULT_THREAD_STACK_SIZE) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("CreateDefaultThread error (%s) : %s"
			"(pThreadProc=%#x, pParam=%#x)\n", 
			strtime_m(), 
			"pthread_attr_setstacksize failed. ", 
			pThreadProc, pParam));
		pthread_attr_destroy(&attr);
		return EXIT_FAILURE;
	}

	// Set the thread as joinable. This is the default on Linux. 
	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	// Indicate whether the scheduling policy and scheduling parameters for the 
	// newly created thread are determined by the values of the schedpolicy and 
	// schedparam attributes (value PTHREAD_EXPLICIT_SCHED) or are inherited from 
	// the parent thread (value PTHREAD_INHERIT_SCHED). 
	if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("CreateDefaultThread error (%s) : %s"
			"(pThreadProc=%#x, pParam=%#x)\n", 
			strtime_m(), 
			"pthread_attr_setinheritsched failed. ", 
			pThreadProc, pParam));
		pthread_attr_destroy(&attr);
		return EXIT_FAILURE;
	}

	// Only PTHREAD_SCOPE_SYSTEM is supported on Linux. This is the default on Linux. 
	// That is a pity as PTHREAD_SCOPE_PROCESS seems to describe a behaviour similar 
	// to Windows threads and process for priorities but it is not clear...
	//if (pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS) != EXIT_SUCCESS)
	//{
	//	PRINT_DEBUG_ERROR_OSTHREAD(("CreateDefaultThread error (%s) : %s"
	//		"(pThreadProc=%#x, pParam=%#x)\n", 
	//		strtime_m(), 
	//		"pthread_attr_setscope failed. ", 
	//		pThreadProc, pParam));
	//	pthread_attr_destroy(&attr);
	//	return EXIT_FAILURE;
	//}

	// Set the scheduling policy attribute of the thread. The supported values 
	// are SCHED_FIFO, SCHED_RR, and SCHED_OTHER.
	policy = SCHED_OTHER;
	if (pthread_attr_setschedpolicy(&attr, policy) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("CreateDefaultThread error (%s) : %s"
			"(pThreadProc=%#x, pParam=%#x)\n", 
			strtime_m(), 
			"pthread_attr_setschedpolicy failed. ", 
			pThreadProc, pParam));
		pthread_attr_destroy(&attr);
		return EXIT_FAILURE;
	}

	// Set the priority of the thread.
	// For the SCHED_OTHER policy, sched_priority is not used in scheduling decisions 
	// (it must be specified as 0).
	param.sched_priority = 0;
	if (pthread_attr_setschedparam(&attr, &param) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("CreateDefaultThread error (%s) : %s"
			"(pThreadProc=%#x, pParam=%#x)\n", 
			strtime_m(), 
			"pthread_attr_setschedparam failed. ", 
			pThreadProc, pParam));
		pthread_attr_destroy(&attr);
		return EXIT_FAILURE;
	}

	//// To retrieve the priorities in case of realtime scheduling policies.
	//pthread_attr_getschedpolicy(&attr, &policy);

	//prio_max = sched_get_priority_max(policy);
	//prio_min = sched_get_priority_min(policy);

	//PRINT_DEBUG_MESSAGE_OSTHREAD(("prio_min = %d\n", prio_min));
	//PRINT_DEBUG_MESSAGE_OSTHREAD(("prio_max = %d\n", prio_max));

	//// Maybe find better quantified values...
	//THREAD_PRIORITY_TIME_CRITICAL = prio_max;
	//THREAD_PRIORITY_HIGHEST = (prio_max-(prio_max+prio_min+1)/2)*2/3+(prio_max+prio_min+1)/2;
	//THREAD_PRIORITY_ABOVE_NORMAL = (prio_max-(prio_max+prio_min+1)/2)/3+(prio_max+prio_min+1)/2;
	//THREAD_PRIORITY_NORMAL = (prio_max+prio_min+1)/2;
	//THREAD_PRIORITY_BELOW_NORMAL = ((prio_max+prio_min+1)/2-prio_min)/2+prio_min;
	//THREAD_PRIORITY_LOWEST = prio_min;
	//THREAD_PRIORITY_IDLE = 0;
	//// 31 corresponds to THREAD_PRIORITY_TIME_CRITICAL-THREAD_PRIORITY_IDLE+1 
	//// (number of levels of priority) for Windows.
	//// Unix should have at least 32 distinct priority levels. 

	if (pthread_create(pThreadId, &attr, pThreadProc, pParam) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("CreateDefaultThread error (%s) : %s"
			"(pThreadProc=%#x, pParam=%#x)\n", 
			strtime_m(), 
			"pthread_create failed. ", 
			pThreadProc, pParam));
		pthread_attr_destroy(&attr);
		return EXIT_FAILURE;
	}

	// Free attribute.
	if (pthread_attr_destroy(&attr) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("CreateDefaultThread error (%s) : %s"
			"(pThreadProc=%#x, pParam=%#x)\n", 
			strtime_m(), 
			"pthread_attr_destroy failed. ", 
			pThreadProc, pParam));
		pthread_detach(*pThreadId);
		pthread_cancel(*pThreadId);
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Put a thread created by CreateDefaultThread() in a detached state.
A call to WaitForThread() will be useless and should fail.
All the memory used by the thread object should be released when it terminates.

THREAD_IDENTIFIER ThreadId : (IN) The thread identifier.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int DetachThread(THREAD_IDENTIFIER ThreadId)
{
#ifdef _WIN32
	// For Windows, there is no problem of detached or joinable state.
	// Anyway, all the resources used by the thread are released when
	// it is terminated and all the handles to it are closed. 
	// All attempts to open a handle to it after it is terminated will fail.
	// And while it is not terminated, we can wait for it with WaitForSingleObject(). 
	UNREFERENCED_PARAMETER(ThreadId);
#else 
	// Indicate that storage for the thread can be reclaimed when it terminates.
	if (pthread_detach(ThreadId) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("DetachThread error (%s) : %s"
			"(ThreadId=%#x)\n", 
			strtime_m(), 
			"pthread_detach failed. ", 
			ThreadId));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Wait until the thread identified by ThreadId has terminated.

THREAD_IDENTIFIER ThreadId : (IN) The thread identifier.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int WaitForThread(THREAD_IDENTIFIER ThreadId)
{
#ifdef _WIN32
	// For Windows, there is no problem of detached or joinable state.
	// Anyway, all the resources used by the thread are released when
	// it is terminated and all the handles to it are closed. 
	// All attempts to open a handle to it after it is terminated will fail.
	// And while it is not terminated, we can wait for it with WaitForSingleObject(). 
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, TRUE, ThreadId);

	if (hThread != NULL)
	{ 
		// The thread is not already terminated.
		// Wait until the thread has terminated.
		if (WaitForSingleObject(hThread, INFINITE) == WAIT_FAILED)
		{
			PRINT_DEBUG_ERROR_OSTHREAD(("WaitForThread error (%s) : %s"
				"(ThreadId=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				ThreadId));
			CloseHandle(hThread);
			return EXIT_FAILURE;
		}

		// Close the thread handle.
		if (!CloseHandle(hThread))
		{
			PRINT_DEBUG_ERROR_OSTHREAD(("WaitForThread error (%s) : %s"
				"(ThreadId=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				ThreadId));
			return EXIT_FAILURE;
		}
	}
#else 
	// Wait until the thread has terminated.
	if (pthread_join(ThreadId, NULL) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("WaitForThread error (%s) : %s"
			"(ThreadId=%#x)\n", 
			strtime_m(), 
			"pthread_join failed. ", 
			ThreadId));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Compare 2 thread identifiers.

THREAD_IDENTIFIER ThreadId1 : (IN) First thread identifier.
THREAD_IDENTIFIER ThreadId2 : (IN) Second thread identifier.

Return : TRUE or FALSE.
*/
inline BOOL CompareThreadId(THREAD_IDENTIFIER ThreadId1, THREAD_IDENTIFIER ThreadId2)
{
#ifdef _WIN32
	if (ThreadId1 == ThreadId2)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
#else 
	if (pthread_equal(ThreadId1, ThreadId2))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
#endif // _WIN32
}

#ifdef _WIN32
#else
/*
Retrieve the thread identifier of the calling thread.

Return : The thread identifier.
*/
inline THREAD_IDENTIFIER GetCurrentThreadId(void)
{
	return pthread_self();
}
#endif // _WIN32

/*
Cause the calling thread to yield execution to another thread that is ready to 
run.

Return : Nothing.
*/
inline void ThreadYield(void)
{
#ifdef _WIN32
	SwitchToThread();
#else 
	// Disabled by default because it is a GNU extension.
#ifdef ENABLE_PTHREAD_YIELD
	pthread_yield();
#else
	sched_yield();
#endif // ENABLE_PTHREAD_YIELD
#endif // _WIN32
}

// Disabled by default because not always available under Linux.
#ifdef ENABLE_WAIT_FOR_THREAD_TIMEOUT
/*
Wait until the thread identified by ThreadId has terminated or a timeout 
elapse.

THREAD_IDENTIFIER ThreadId : (IN) The thread identifier.
int timeout : (IN) Timeout in ms (max is MAX_THREAD_TIMEOUT).

Return : EXIT_SUCCESS if the thread was successfully joined, EXIT_TIMEOUT if 
the timeout elapse or EXIT_FAILURE if there is an error.
*/
inline int WaitForThreadWithTimeout(THREAD_IDENTIFIER ThreadId, int timeout)
{
#ifdef _WIN32
	DWORD dwWaitResult = WAIT_FAILED; 

	// For Windows, there is no problem of detached or joinable state.
	// Anyway, all the resources used by the thread are released when
	// it is terminated and all the handles to it are closed. 
	// All attempts to open a handle to it after it is terminated will fail.
	// And while it is not terminated, we can wait for it with WaitForSingleObject(). 
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, TRUE, ThreadId);

	if (hThread != NULL)
	{ 
		// The thread is not already terminated.
		// Wait until the thread has terminated.
		dwWaitResult = WaitForSingleObject(hThread, (DWORD)timeout);

		switch (dwWaitResult)	 
		{
		case WAIT_OBJECT_0: 
			// The thread was successfully joined.
			break; 
		case WAIT_TIMEOUT: 
			CloseHandle(hThread);
			return EXIT_TIMEOUT; 
		default:
			PRINT_DEBUG_ERROR_OSTHREAD(("WaitForThreadWithTimeout error (%s) : %s"
				"(ThreadId=%#x, timeout=%d)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				ThreadId, timeout));
			CloseHandle(hThread);
			return EXIT_FAILURE;
		}		

		// Close the thread handle.
		if (!CloseHandle(hThread))
		{
			PRINT_DEBUG_ERROR_OSTHREAD(("WaitForThreadWithTimeout error (%s) : %s"
				"(ThreadId=%#x, timeout=%d)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				ThreadId, timeout));
			return EXIT_FAILURE;
		}
	}
#else 
	int iResult = EXIT_FAILURE;
	struct timespec abstime;

	// Calculate relative interval as current time plus duration given by timeout.

	if (clock_gettime(CLOCK_REALTIME, &abstime) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("WaitForThreadWithTimeout error (%s) : %s"
			"(ThreadId=%#x, timeout=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ThreadId, timeout));
		return EXIT_FAILURE;
	}

	abstime.tv_sec += timeout/1000; // Seconds.
	abstime.tv_nsec += (timeout%1000)*1000000; // Additional nanoseconds.
	abstime.tv_sec += abstime.tv_nsec/1000000000;
	abstime.tv_nsec = abstime.tv_nsec%1000000000;

	// Wait until the thread has terminated.
	iResult = pthread_timedjoin_np(ThreadId, NULL, &abstime);
	switch (iResult)	 
	{
	case EXIT_SUCCESS: 
		break;
	case ETIMEDOUT: 
		return EXIT_TIMEOUT;
	default:
		PRINT_DEBUG_ERROR_OSTHREAD(("WaitForThreadWithTimeout error (%s) : %s"
			"(ThreadId=%#x, timeout=%d)\n", 
			strtime_m(), 
			"pthread_timedjoin_np failed. ", 
			ThreadId, timeout));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}
#endif // ENABLE_WAIT_FOR_THREAD_TIMEOUT

// Bad to use, but if it can avoid using the task manager to kill an application...
#ifdef ENABLE_KILL_THREAD

#ifdef _MSC_VER
// Disable Visual Studio warnings about TerminateThread().
#pragma warning(disable : 6258) 
#endif // _MSC_VER

/*
Enable the use of either KillThread() or CancelThread() for the current thread.
Note that by default (if KillOrCancelThreadMode() was never called for the current 
thread), it is CancelThread() that is enabled.

BOOL bKill : (IN) FALSE to enable CancelThread() otherwise KillThread() is enabled.

Return : Nothing.
*/
inline void KillOrCancelThreadMode(BOOL bKill)
{
#ifdef _WIN32
	// For Windows, KillThread() or CancelThread() can be used without restrictions 
	// (except that they are dangerous and uncommon functions that must be used with care).
	UNREFERENCED_PARAMETER(bKill);
#else 
	if (bKill)
	{
		// When initially created, a thread is synchronously cancelable.
		// Here enable the current thread to be canceled (killed) asynchronously (i.e. immediately).
		//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	}
	else
	{
		// When initially created, a thread is synchronously cancelable.
		// Here reset to this default cancel state.
		//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	}
#endif // _WIN32
}

/*
Terminate a thread immediately. 
The thread to terminate should have called KillOrCancelThreadMode(TRUE) before, otherwise the results are 
undefined.
It is a dangerous function that should only be used in the most extreme cases 
You should call KillThread() only if you know exactly what the target thread is doing, and you control 
all of the code that the target thread could possibly be running at the time of the termination (C++ 
destructors might not be called automatically...). 

THREAD_IDENTIFIER ThreadId : (IN) The thread identifier.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int KillThread(THREAD_IDENTIFIER ThreadId)
{
#ifdef _WIN32
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, TRUE, ThreadId);

	if (hThread == NULL)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("KillThread error (%s) : %s"
			"(ThreadId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ThreadId));
		return EXIT_FAILURE;
	}

	if (!TerminateThread(hThread, EXIT_KILLED_THREAD))
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("KillThread error (%s) : %s"
			"(ThreadId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ThreadId));
		CloseHandle(hThread);
		return EXIT_FAILURE;
	}

	if (!CloseHandle(hThread))
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("KillThread error (%s) : %s"
			"(ThreadId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ThreadId));
		return EXIT_FAILURE;
	}
#else 
	int iResult = pthread_detach(ThreadId);
	switch (iResult)
	{
	case EXIT_SUCCESS:
		break;
	case EINVAL:
		// The thread is already detached.
		break;
	default:
		PRINT_DEBUG_ERROR_OSTHREAD(("KillThread error (%s) : %s"
			"(ThreadId=%#x)\n", 
			strtime_m(), 
			"pthread_detach failed. ", 
			ThreadId));
		return EXIT_FAILURE;
	}

	// By default, a thread is synchronously cancelable. But if KillOrCancelThread(TRUE) is 
	// called from its thread function, it should have been set to asynchronously cancelable 
	// (i.e. can be killed immediately).
	if (pthread_cancel(ThreadId) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("KillThread error (%s) : %s"
			"(ThreadId=%#x)\n", 
			strtime_m(), 
			"pthread_cancel failed. ", 
			ThreadId));
		return EXIT_FAILURE;
	}
	
	// Instead of detaching, it could be also possible to join...
	//int iResult = pthread_join(ThreadId, NULL);
	//switch (iResult)
	//{
	//case EXIT_SUCCESS:
	//	break;
	//case EINVAL:
	//	// The thread is detached.
	//	break;
	//default:
	//	PRINT_DEBUG_ERROR_OSTHREAD(("KillThread error (%s) : %s"
	//		"(ThreadId=%#x)\n", 
	//		strtime_m(), 
	//		"pthread_join failed. ", 
	//		ThreadId));
	//	return EXIT_FAILURE;
	//}
#endif // _WIN32

	return EXIT_SUCCESS;
}

#ifdef _MSC_VER
// Restore the Visual Studio warnings previously disabled.
#pragma warning(default : 6258) 
#endif // _MSC_VER

#endif // ENABLE_KILL_THREAD

// Bad to use, but if it can avoid using the task manager to kill an application...
#ifdef ENABLE_CANCEL_THREAD
#ifdef _WIN32
EXTERN_C VOID CALLBACK _CancelThreadAPC(ULONG_PTR dwParam);
#endif // _WIN32

/*
Cancel a thread. 
If the thread to cancel have called KillOrCancelThreadMode(TRUE) before, the results are 
undefined unless KillOrCancelThreadMode(FALSE) was called to reset to the default 
behaviour (CancelThread() enabled and KillThread() disabled).
The thread should only exit when it calls a function that is a 
cancellation point (use TestCancelThread() to create a cancellation point but note also 
that other functions may have internal cancellation points). 
It is a dangerous function that should only be used in the most extreme cases 
You should call CancelThread() only if you know exactly what the target thread is doing, and you control 
all of the code that the target thread could possibly be running at the time of the termination (C++ 
destructors might not be called automatically...). 

THREAD_IDENTIFIER ThreadId : (IN) The thread identifier.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int CancelThread(THREAD_IDENTIFIER ThreadId)
{
#ifdef _WIN32
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, TRUE, ThreadId);

	if (hThread == NULL)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("CancelThread error (%s) : %s"
			"(ThreadId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ThreadId));
		return EXIT_FAILURE;
	}

	//	if (!QueueUserAPC(_CancelThreadAPC, hThread, (ULONG_PTR)hThread))
	if (!QueueUserAPC(_CancelThreadAPC, hThread, 0))
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("CancelThread error (%s) : %s"
			"(ThreadId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ThreadId));
		CloseHandle(hThread);
		return EXIT_FAILURE;
	}

	if (!CloseHandle(hThread))
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("CancelThread error (%s) : %s"
			"(ThreadId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ThreadId));
		return EXIT_FAILURE;
	}
	// CloseHandle() called from _CancelThreadAPC()?
#else 
	int iResult = pthread_detach(ThreadId);
	switch (iResult)
	{
	case EXIT_SUCCESS:
		break;
	case EINVAL:
		// The thread is already detached.
		break;
	default:
		PRINT_DEBUG_ERROR_OSTHREAD(("CancelThread error (%s) : %s"
			"(ThreadId=%#x)\n", 
			strtime_m(), 
			"pthread_detach failed. ", 
			ThreadId));
		return EXIT_FAILURE;
	}

	// When initially created, a thread is synchronously cancelable.
	// The thread should only exit when it calls a function that is a 
	// cancellation point (such as read(), write(), sleep(), wait(), pthread_testcancel()...). 
	if (pthread_cancel(ThreadId) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("CancelThread error (%s) : %s"
			"(ThreadId=%#x)\n", 
			strtime_m(), 
			"pthread_cancel failed. ", 
			ThreadId));
		return EXIT_FAILURE;
	}

	// Instead of detaching, it could be also possible to join...
	//int iResult = pthread_join(ThreadId, NULL);
	//switch (iResult)
	//{
	//case EXIT_SUCCESS:
	//	break;
	//case EINVAL:
	//	// The thread is detached.
	//	break;
	//default:
	//	PRINT_DEBUG_ERROR_OSTHREAD(("CancelThread error (%s) : %s"
	//		"(ThreadId=%#x)\n", 
	//		strtime_m(), 
	//		"pthread_join failed. ", 
	//		ThreadId));
	//	return EXIT_FAILURE;
	//}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Create a cancellation point in the current thread.

Return : Nothing.
*/
inline void TestCancelThread(void)
{
#ifdef _WIN32
	SleepEx(0, TRUE);
#else 
	pthread_testcancel();
#endif // _WIN32
}
#endif // ENABLE_CANCEL_THREAD

// Useless, multiprocessors should be handled correctly automatically...
#ifdef ENABLE_SET_THREAD_DEFAULT_PROCESSOR
/*
Set a preferred processor for a thread. The system schedules threads on their 
preferred processors whenever possible.

THREAD_IDENTIFIER ThreadId : (IN) The thread identifier.
int ProcessorNumber : (IN) The number of the preferred processor for the 
thread (zero-based).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SetThreadDefaultProcessor(THREAD_IDENTIFIER ThreadId, int ProcessorNumber)
{
#ifdef _WIN32
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, TRUE, ThreadId);

	if (hThread == NULL)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("SetThreadDefaultProcessor error (%s) : %s"
			"(ThreadId=%#x, ProcessorNumber=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ThreadId, ProcessorNumber));
		return EXIT_FAILURE;
	}

	if (SetThreadIdealProcessor(hThread, (DWORD)ProcessorNumber) == (DWORD)-1)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("SetThreadDefaultProcessor error (%s) : %s"
			"(ThreadId=%#x, ProcessorNumber=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ThreadId, ProcessorNumber));
		CloseHandle(hThread);
		return EXIT_FAILURE;
	}

	// Close the thread handle (this does not terminate the thread).
	if (!CloseHandle(hThread))
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("SetThreadDefaultProcessor error (%s) : %s"
			"(ThreadId=%#x, ProcessorNumber=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ThreadId, ProcessorNumber));
		return EXIT_FAILURE;
	}
#else 
	cpu_set_t cpuset;

	CPU_ZERO(&cpuset);
	CPU_SET(ProcessorNumber, &cpuset);

	if (pthread_setaffinity_np(ThreadId, sizeof(cpu_set_t), &cpuset) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("SetThreadDefaultProcessor error (%s) : %s"
			"(ThreadId=%#x, ProcessorNumber=%d)\n", 
			strtime_m(), 
			"pthread_setaffinity_np failed. ", 
			ThreadId, ProcessorNumber));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}
#endif // ENABLE_SET_THREAD_DEFAULT_PROCESSOR

// This is disabled by default as the behaviour should be really different between Windows and Linux...
#ifdef ENABLE_PRIORITY_HANDLING
/*
Set the priority of the thread identified by ThreadId. Priority should be one 
of the following constants :
THREAD_PRIORITY_TIME_CRITICAL, THREAD_PRIORITY_HIGHEST, 
THREAD_PRIORITY_ABOVE_NORMAL, THREAD_PRIORITY_NORMAL,
THREAD_PRIORITY_BELOW_NORMAL, THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_IDLE.
This function should be called when the thread is not already terminated.

THREAD_IDENTIFIER ThreadId : (IN) The thread identifier.
int Priority : (IN) Integer representing the thread priority.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SetThreadDefaultPriority(THREAD_IDENTIFIER ThreadId, int Priority)
{
#ifdef _WIN32
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, TRUE, ThreadId);

	if (hThread == NULL)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("SetThreadDefaultPriority error (%s) : %s"
			"(ThreadId=%#x, Priority=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ThreadId, Priority));
		return EXIT_FAILURE;
	}

	if (!SetThreadPriority(hThread, Priority))
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("SetThreadDefaultPriority error (%s) : %s"
			"(ThreadId=%#x, Priority=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ThreadId, Priority));
		CloseHandle(hThread);
		return EXIT_FAILURE;
	}

	// Close the thread handle (this does not terminate the thread).
	if (!CloseHandle(hThread))
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("SetThreadDefaultPriority error (%s) : %s"
			"(ThreadId=%#x, Priority=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ThreadId, Priority));
		return EXIT_FAILURE;
	}
#else 
	struct sched_param param;
	int policy = 0;

	if (Priority == THREAD_PRIORITY_IDLE)
	{
		policy = SCHED_OTHER;
		param.sched_priority = 0; // Only this priority level is available for this policy.
	}
	else
	{
		policy = SCHED_RR;
		param.sched_priority = Priority;
	}

	if (pthread_setschedparam(ThreadId, policy, &param) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("SetThreadDefaultPriority error (%s) : %s"
			"(ThreadId=%#x, Priority=%d)\n", 
			strtime_m(), 
			"pthread_setschedparam failed. ", 
			ThreadId, Priority));
		return EXIT_FAILURE;
	}

	//if (pthread_setschedprio(ThreadId, Priority) != EXIT_SUCCESS)
	//{
	//	PRINT_DEBUG_ERROR_OSTHREAD(("SetThreadDefaultPriority error (%s) : %s"
	//		"(ThreadId=%#x, Priority=%d)\n", 
	//		strtime_m(), 
	//		"pthread_setschedprio failed. ", 
	//		ThreadId, Priority));
	//	return EXIT_FAILURE;
	//}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Get the priority of the thread identified by ThreadId. *pPriority should be 
one of the following constants :
THREAD_PRIORITY_TIME_CRITICAL, THREAD_PRIORITY_HIGHEST, 
THREAD_PRIORITY_ABOVE_NORMAL, THREAD_PRIORITY_NORMAL,
THREAD_PRIORITY_BELOW_NORMAL, THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_IDLE.
This function should be called when the thread is not already terminated.

THREAD_IDENTIFIER ThreadId : (IN) The thread identifier.
int* pPriority : (INOUT) Valid pointer that will receive an integer 
representing the thread priority.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int GetThreadDefaultPriority(THREAD_IDENTIFIER ThreadId, int* pPriority)
{
#ifdef _WIN32
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, TRUE, ThreadId);

	if (hThread == NULL)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("GetThreadDefaultPriority error (%s) : %s"
			"(ThreadId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ThreadId));
		return EXIT_FAILURE;
	}

	*pPriority = GetThreadPriority(hThread);
	if (*pPriority == THREAD_PRIORITY_ERROR_RETURN)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("GetThreadDefaultPriority error (%s) : %s"
			"(ThreadId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ThreadId));
		CloseHandle(hThread);
		return EXIT_FAILURE;
	}

	// Close the thread handle (this does not terminate the thread).
	if (!CloseHandle(hThread))
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("GetThreadDefaultPriority error (%s) : %s"
			"(ThreadId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ThreadId));
		return EXIT_FAILURE;
	}
#else 
	struct sched_param param;
	int policy = 0;

	if (pthread_getschedparam(ThreadId, &policy, &param) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTHREAD(("GetThreadDefaultPriority error (%s) : %s"
			"(ThreadId=%#x)\n", 
			strtime_m(), 
			"pthread_getschedparam failed. ", 
			ThreadId));
		return EXIT_FAILURE;
	}

	// Get the thread priority.
	*pPriority = param.sched_priority;
#endif // _WIN32

	return EXIT_SUCCESS;
}
#endif // ENABLE_PRIORITY_HANDLING

#endif // OSTHREAD_H
