/***************************************************************************************************************:')

OSIPSem.h

Interprocess semaphores handling.

Fabrice Le Bars

Created : 2010-05-23

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

#ifndef OSIPSEM_H
#define OSIPSEM_H

#include "OSTime.h"

/*
Debug macros specific to OSIPSem.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSIPSEM
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSIPSEM
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSIPSEM
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSIPSEM
#	define PRINT_DEBUG_MESSAGE_OSIPSEM(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSIPSEM(params)
#endif // _DEBUG_MESSAGES_OSIPSEM

#ifdef _DEBUG_WARNINGS_OSIPSEM
#	define PRINT_DEBUG_WARNING_OSIPSEM(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSIPSEM(params)
#endif // _DEBUG_WARNINGS_OSIPSEM

#ifdef _DEBUG_ERRORS_OSIPSEM
#	define PRINT_DEBUG_ERROR_OSIPSEM(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSIPSEM(params)
#endif // _DEBUG_ERRORS_OSIPSEM

#ifdef _WIN32
#else 
#include <fcntl.h> // For O_* constants.
#include <sys/stat.h> // For mode constants.
#include <semaphore.h>
#endif // _WIN32

#define MAX_IPSEM_TIMEOUT (LONG_MAX-2)
#define MAX_IPSEM_COUNT 2147483646
#define MAX_IPSEM_NAME_LENGTH (128-8)

#ifdef _WIN32
typedef HANDLE IPSEMAPHORE;
#else
struct IPSEMAPHORE
{
	sem_t* pSem;
	char szSemName[MAX_IPSEM_NAME_LENGTH+8];
};
typedef struct IPSEMAPHORE IPSEMAPHORE;
#endif // _WIN32

/*
Create or open a named interprocess semaphore. Use DeleteIPSemaphore() to 
delete it at the end.

IPSEMAPHORE* pSemaphore : (INOUT) Valid pointer that will receive a structure 
corresponding to a semaphore.
int InitialCount : (IN) Initial count for the semaphore.
char* szName : (IN) Name of the semaphore.

Return : EXIT_SUCCESS, EXIT_NAME_TOO_LONG or EXIT_FAILURE.
*/
inline int CreateOrOpenIPSemaphore(IPSEMAPHORE* pSemaphore, int InitialCount, char* szName)
{
#ifdef _WIN32
	HANDLE hSemaphore = INVALID_HANDLE_VALUE;
	char szSemName[MAX_IPSEM_NAME_LENGTH+8];

	if (strlen(szName) > MAX_IPSEM_NAME_LENGTH)
	{
		PRINT_DEBUG_ERROR_OSIPSEM(("CreateOrOpenIPSemaphore error (%s) : %s"
			"(InitialCount=%d, szName=%s)\n", 
			strtime_m(), 
			szOSUtilsErrMsgs[EXIT_NAME_TOO_LONG], 
			InitialCount, szName));
		return EXIT_NAME_TOO_LONG;
	}

	// Prefixing the name with "Global\" allows processes to communicate with each 
	// other even if they are in different terminal server sessions (e.g. several 
	// remote desktop connections).
	sprintf(szSemName, "Global\\%s", szName);

	hSemaphore = CreateSemaphore( 
		NULL, // Default security attributes.
		InitialCount, // Initial count.
		MAX_IPSEM_COUNT, // Maximum count.
		szSemName); // Named semaphore.

	if (hSemaphore == NULL) 
	{
		PRINT_DEBUG_ERROR_OSIPSEM(("CreateOrOpenIPSemaphore error (%s) : %s"
			"(InitialCount=%d, szName=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			InitialCount, szName));
		return EXIT_FAILURE;
	}

	*pSemaphore = hSemaphore;
#else 
	if (strlen(szName) > MAX_IPSEM_NAME_LENGTH)
	{
		PRINT_DEBUG_ERROR_OSIPSEM(("CreateOrOpenIPSemaphore error (%s) : %s"
			"(InitialCount=%d, szName=%s)\n", 
			strtime_m(), 
			szOSUtilsErrMsgs[EXIT_NAME_TOO_LONG], 
			InitialCount, szName));
		return EXIT_NAME_TOO_LONG;
	}

	// szName must be like "/nameXXX".
	sprintf(pSemaphore->szSemName, "/%s", szName);

	pSemaphore->pSem = sem_open(pSemaphore->szSemName, O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO, InitialCount);

	if (pSemaphore->pSem == SEM_FAILED)
	{
		PRINT_DEBUG_ERROR_OSIPSEM(("CreateOrOpenIPSemaphore error (%s) : %s"
			"(InitialCount=%d, szName=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			InitialCount, szName));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Delete an interprocess semaphore created by CreateOrOpenIPSemaphore().

IPSEMAPHORE* pSemaphore : (INOUT) Valid pointer to a structure corresponding to 
a semaphore.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int DeleteIPSemaphore(IPSEMAPHORE* pSemaphore)
{
#ifdef _WIN32
	if (!CloseHandle(*pSemaphore))
	{
		PRINT_DEBUG_ERROR_OSIPSEM(("DeleteIPSemaphore error (%s) : %s"
			"(pSemaphore=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pSemaphore));
		return EXIT_FAILURE;
	}
#else 
	// Close the named semaphore, allowing any resources that the system has
	// allocated to the calling process for this semaphore to be freed.
	if (sem_close(pSemaphore->pSem) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPSEM(("DeleteIPSemaphore error (%s) : %s"
			"(pSemaphore=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pSemaphore));
		return EXIT_FAILURE;
	}

	// The semaphore name is removed immediately. The semaphore is destroyed
	// once all other processes that have the semaphore open close it.
	sem_unlink(pSemaphore->szSemName);
	//if (sem_unlink(pSemaphore->szSemName) != EXIT_SUCCESS)
	//{
	//	PRINT_DEBUG_ERROR_OSIPSEM(("DeleteIPSemaphore error (%s) : %s"
	//		"(pSemaphore=%#x)\n", 
	//		strtime_m(), 
	//		GetLastErrorMsg(), 
	//		pSemaphore));
	//	return EXIT_FAILURE;
	//}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Increase the count of a semaphore.

IPSEMAPHORE* pSemaphore : (INOUT) Valid pointer to a structure corresponding to 
a semaphore.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int PostIPSemaphore(IPSEMAPHORE* pSemaphore)
{
#ifdef _WIN32
	if (!ReleaseSemaphore( 
		*pSemaphore, // Handle to semaphore.
		1, // Increase count by 1.
		NULL)) // Not interested in previous count.	
	{
		PRINT_DEBUG_ERROR_OSIPSEM(("PostIPSemaphore error (%s) : %s"
			"(pSemaphore=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pSemaphore));
		return EXIT_FAILURE;
	}
#else 
	if (sem_post(pSemaphore->pSem) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPSEM(("PostIPSemaphore error (%s) : %s"
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

IPSEMAPHORE* pSemaphore : (INOUT) Valid pointer to a structure corresponding to 
a semaphore.

Return : EXIT_SUCCESS if the semaphore is successfully decreased, 
EXIT_OBJECT_NONSIGNALED if it is not available or EXIT_FAILURE if there is 
an error.
*/
inline int TryAndGetIPSemaphore(IPSEMAPHORE* pSemaphore)
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
		PRINT_DEBUG_ERROR_OSIPSEM(("TryAndGetIPSemaphore error (%s) : %s"
			"(pSemaphore=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pSemaphore));
		return EXIT_FAILURE;
	}
#else 
	if (sem_trywait(pSemaphore->pSem) != EXIT_SUCCESS)
	{
		switch (errno)	 
		{
		case EAGAIN: 
			return EXIT_OBJECT_NONSIGNALED;
		default:
			PRINT_DEBUG_ERROR_OSIPSEM(("TryAndGetIPSemaphore error (%s) : %s"
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

IPSEMAPHORE* pSemaphore : (INOUT) Valid pointer to a structure corresponding to 
a semaphore.
int timeout : (IN) Timeout in ms (max is MAX_IPSEMAPHORE_TIMEOUT).

Return : EXIT_SUCCESS if the semaphore is successfully decreased, 
EXIT_TIMEOUT if it is not available after the timeout elapses or EXIT_FAILURE if
there is an error.
*/
inline int WaitAndGetIPSemaphore(IPSEMAPHORE* pSemaphore, int timeout)
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
		PRINT_DEBUG_ERROR_OSIPSEM(("WaitAndGetIPSemaphore error (%s) : %s"
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
		PRINT_DEBUG_ERROR_OSIPSEM(("WaitAndGetIPSemaphore error (%s) : %s"
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

	if (sem_timedwait(pSemaphore->pSem, &abstime) != EXIT_SUCCESS)
	{
		switch (errno)	 
		{
		case ETIMEDOUT: 
			return EXIT_TIMEOUT;
		default:
			PRINT_DEBUG_ERROR_OSIPSEM(("WaitAndGetIPSemaphore error (%s) : %s"
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

#endif // OSIPSEM_H
