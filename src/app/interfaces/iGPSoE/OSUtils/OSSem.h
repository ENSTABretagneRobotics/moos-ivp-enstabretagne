/***************************************************************************************************************:')

OSSem.h

Semaphores handling (not interprocess).

Fabrice Le Bars

Created : 2009-02-10

Version status : Tested quickly

***************************************************************************************************************:)*/

// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef OSSEM_H
#define OSSEM_H

#include "OSTime.h"

/*
Debug macros specific to OSSem.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSSEM
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSSEM
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSSEM
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSSEM
#	define PRINT_DEBUG_MESSAGE_OSSEM(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSSEM(params)
#endif // _DEBUG_MESSAGES_OSSEM

#ifdef _DEBUG_WARNINGS_OSSEM
#	define PRINT_DEBUG_WARNING_OSSEM(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSSEM(params)
#endif // _DEBUG_WARNINGS_OSSEM

#ifdef _DEBUG_ERRORS_OSSEM
#	define PRINT_DEBUG_ERROR_OSSEM(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSSEM(params)
#endif // _DEBUG_ERRORS_OSSEM

#define MAX_SEM_TIMEOUT (LONG_MAX-2)
#define MAX_SEM_COUNT 2147483646

#ifdef _WIN32
#else 
#include <semaphore.h>
#endif // _WIN32

#ifdef _WIN32
typedef HANDLE SEMAPHORE;
#else
typedef sem_t SEMAPHORE;
#endif // _WIN32

/*
Create a default semaphore (not interprocess). Use DeleteSemaphore() to delete it at the end.

SEMAPHORE* pSemaphore : (INOUT) Valid pointer that will receive a structure 
corresponding to a semaphore.
int InitialCount : (IN) Initial count for the semaphore.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int CreateDefaultSemaphore(SEMAPHORE* pSemaphore, int InitialCount)
{
#ifdef _WIN32
	HANDLE hSemaphore;

	hSemaphore = CreateSemaphore( 
		NULL, // Default security attributes.
		InitialCount, // Initial count.
		MAX_SEM_COUNT, // Maximum count.
		NULL); // Unnamed semaphore.

	if (hSemaphore == NULL) 
	{
		PRINT_DEBUG_ERROR_OSSEM(("CreateDefaultSemaphore error (%s) : %s"
			"(InitialCount=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			InitialCount));
		return EXIT_FAILURE;
	}

	*pSemaphore = hSemaphore;
#else 
	// The pshared argument indicates whether the semaphore is local to the 
	// current process (pshared is zero) or is to be shared between several 
	// processes (pshared is not zero).
	if (sem_init(pSemaphore, 0, InitialCount) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSSEM(("CreateDefaultSemaphore error (%s) : %s"
			"(InitialCount=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			InitialCount));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Delete a semaphore created by CreateDefaultSemaphore().

SEMAPHORE* pSemaphore : (INOUT) Valid pointer to a structure corresponding to 
a semaphore.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int DeleteSemaphore(SEMAPHORE* pSemaphore)
{
#ifdef _WIN32
	if (!CloseHandle(*pSemaphore))
	{
		PRINT_DEBUG_ERROR_OSSEM(("DeleteSemaphore error (%s) : %s"
			"(pSemaphore=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pSemaphore));
		return EXIT_FAILURE;
	}
#else 
	if (sem_destroy(pSemaphore) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSSEM(("DeleteSemaphore error (%s) : %s"
			"(pSemaphore=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pSemaphore));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Increase the count of a semaphore.

SEMAPHORE* pSemaphore : (INOUT) Valid pointer to a structure corresponding to 
a semaphore.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int PostSemaphore(SEMAPHORE* pSemaphore)
{
#ifdef _WIN32
	if (!ReleaseSemaphore( 
		*pSemaphore, // Handle to semaphore.
		1, // Increase count by 1.
		NULL)) // Not interested in previous count.	
	{
		PRINT_DEBUG_ERROR_OSSEM(("PostSemaphore error (%s) : %s"
			"(pSemaphore=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pSemaphore));
		return EXIT_FAILURE;
	}
#else 
	if (sem_post(pSemaphore) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSSEM(("PostSemaphore error (%s) : %s"
			"(pSemaphore=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pSemaphore));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Decrease the count of a semaphore without blocking if the semaphore is not 
available.

SEMAPHORE* pSemaphore : (INOUT) Valid pointer to a structure corresponding to 
a semaphore.

Return : EXIT_SUCCESS if the semaphore is successfully decreased, 
EXIT_OBJECT_NONSIGNALED if it is not available or EXIT_FAILURE if there is 
an error.
*/
inline int TryAndGetSemaphore(SEMAPHORE* pSemaphore)
{
#ifdef _WIN32
	DWORD dwWaitResult = WAIT_FAILED; 

	// Return when the specified object is in the signaled state or the time-out interval elapses.
	// In this case, we just check if the semaphore is available.
	dwWaitResult = WaitForSingleObject(
		*pSemaphore, // Handle to the semaphore.
		(DWORD)0); // Time-out interval (ms).

	switch (dwWaitResult)	 
	{
	case WAIT_OBJECT_0: 
		// The semaphore object was signaled.
		break; 
	case WAIT_TIMEOUT: 
		// The semaphore was nonsignaled, so a time-out occurred.
		return EXIT_OBJECT_NONSIGNALED; 
	default:
		PRINT_DEBUG_ERROR_OSSEM(("TryAndGetSemaphore error (%s) : %s"
			"(pSemaphore=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pSemaphore));
		return EXIT_FAILURE;
	}
#else 
	if (sem_trywait(pSemaphore) != EXIT_SUCCESS)
	{
		switch (errno)	 
		{
		case EAGAIN: 
			return EXIT_OBJECT_NONSIGNALED;
		default:
			PRINT_DEBUG_ERROR_OSSEM(("TryAndGetSemaphore error (%s) : %s"
				"(pSemaphore=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pSemaphore));
			return EXIT_FAILURE;
		}
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Decrease the count of a semaphore or block until the semaphore be available or 
a timeout elapses.

SEMAPHORE* pSemaphore : (INOUT) Valid pointer to a structure corresponding to 
a semaphore.
int timeout : (IN) Timeout in ms (max is MAX_SEM_TIMEOUT).

Return : EXIT_SUCCESS if the semaphore is successfully decreased, 
EXIT_TIMEOUT if it is not available after the timeout elapses or EXIT_FAILURE if
there is an error.
*/
inline int WaitAndGetSemaphore(SEMAPHORE* pSemaphore, int timeout)
{
#ifdef _WIN32
	DWORD dwWaitResult = WAIT_FAILED; 

	// Returns when the specified object is in the signaled state or the time-out interval elapses.
	dwWaitResult = WaitForSingleObject(
		*pSemaphore, // Handle to the semaphore.
		(DWORD)timeout); // Time-out interval (ms).
	
	switch (dwWaitResult)	 
	{
	case WAIT_OBJECT_0: 
		// The semaphore object was signaled.
		break; 
	case WAIT_TIMEOUT: 
		// The semaphore was nonsignaled, so a time-out occurred.
		return EXIT_TIMEOUT; 
	default:
		PRINT_DEBUG_ERROR_OSSEM(("WaitAndGetSemaphore error (%s) : %s"
			"(pSemaphore=%#x, timeout=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pSemaphore, timeout));
		return EXIT_FAILURE;
	}
#else 
	struct timespec abstime;

	// Calculate relative interval as current time plus duration given by timeout.

	if (clock_gettime(CLOCK_REALTIME, &abstime) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSSEM(("WaitAndGetSemaphore error (%s) : %s"
			"(pSemaphore=%#x, timeout=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pSemaphore, timeout));
		return EXIT_FAILURE;
	}

	abstime.tv_sec += timeout/1000; // Seconds.
	abstime.tv_nsec += (timeout%1000)*1000000; // Additional nanoseconds.
	abstime.tv_sec += abstime.tv_nsec/1000000000;
	abstime.tv_nsec = abstime.tv_nsec%1000000000;

	if (sem_timedwait(pSemaphore, &abstime) != EXIT_SUCCESS)
	{
		switch (errno)	 
		{
		case ETIMEDOUT: 
			return EXIT_TIMEOUT;
		default:
			PRINT_DEBUG_ERROR_OSSEM(("WaitAndGetSemaphore error (%s) : %s"
				"(pSemaphore=%#x, timeout=%d)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pSemaphore, timeout));
			return EXIT_FAILURE;
		}
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

#endif // OSSEM_H
