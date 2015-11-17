/***************************************************************************************************************:')

OSCriticalSection.h

Critical sections handling (not interprocess).

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

#ifndef OSCRITICALSECTION_H
#define OSCRITICALSECTION_H

#include "OSThread.h"

/*
Debug macros specific to OSCriticalSection.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSCRITICALSECTION
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSCRITICALSECTION
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSCRITICALSECTION
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSCRITICALSECTION
#	define PRINT_DEBUG_MESSAGE_OSCRITICALSECTION(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSCRITICALSECTION(params)
#endif // _DEBUG_MESSAGES_OSCRITICALSECTION

#ifdef _DEBUG_WARNINGS_OSCRITICALSECTION
#	define PRINT_DEBUG_WARNING_OSCRITICALSECTION(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSCRITICALSECTION(params)
#endif // _DEBUG_WARNINGS_OSCRITICALSECTION

#ifdef _DEBUG_ERRORS_OSCRITICALSECTION
#	define PRINT_DEBUG_ERROR_OSCRITICALSECTION(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSCRITICALSECTION(params)
#endif // _DEBUG_ERRORS_OSCRITICALSECTION

#ifdef _WIN32
#else
typedef pthread_mutex_t CRITICAL_SECTION;
#endif // _WIN32

/*
Initialize a critical section (not interprocess).

CRITICAL_SECTION* pCriticalSection : (INOUT) Pointer to the critical section object.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int InitCriticalSection(CRITICAL_SECTION* pCriticalSection)
{
#ifdef _WIN32
	if (!InitializeCriticalSectionAndSpinCount(pCriticalSection, 32))
	{
		PRINT_DEBUG_ERROR_OSCRITICALSECTION(("InitCriticalSection error (%s) : %s\n", 
			strtime_m(), GetLastErrorMsg()));
		return EXIT_FAILURE;
	}
#else 
	pthread_mutexattr_t attr;

	if (pthread_mutexattr_init(&attr) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSCRITICALSECTION(("InitCriticalSection error (%s) : %s\n", 
			strtime_m(), "pthread_mutexattr_init failed. "));
		return EXIT_FAILURE;
	}

	// A thread attempting to relock this mutex without first unlocking it shall succeed in 
	// locking the mutex. The relocking deadlock which can occur with mutexes of type 
	// PTHREAD_MUTEX_NORMAL cannot occur with this type of mutex. Multiple locks of this mutex 
	// shall require the same number of unlocks to release the mutex before another thread can 
	// acquire the mutex. A thread attempting to unlock a mutex which another thread has locked 
	// shall return with an error. A thread attempting to unlock an unlocked mutex shall return 
	// with an error. 
	// This is similar to the behaviour of the Windows critical section objects.
	if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSCRITICALSECTION(("InitCriticalSection error (%s) : %s\n", 
			strtime_m(), "pthread_mutexattr_settype failed. "));
		pthread_mutexattr_destroy(&attr);
		return EXIT_FAILURE;
	}

	if (pthread_mutex_init(pCriticalSection, &attr) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSCRITICALSECTION(("InitCriticalSection error (%s) : %s\n", 
			strtime_m(), "pthread_mutex_init failed. "));
		pthread_mutexattr_destroy(&attr);
		return EXIT_FAILURE;
	}

	if (pthread_mutexattr_destroy(&attr) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSCRITICALSECTION(("InitCriticalSection error (%s) : %s\n", 
			strtime_m(), "pthread_mutexattr_destroy failed. "));
		pthread_mutex_destroy(pCriticalSection);
		return EXIT_FAILURE;
	}
#endif // _WIN32
	return EXIT_SUCCESS;
}

#ifndef _WIN32
/*
Release resources used by a critical section.
Be sure that no thread has ownership of the specified critical section object before 
calling DeleteCriticalSection(), otherwise the behaviour is undefined.

CRITICAL_SECTION* pCriticalSection : (INOUT) Pointer to the critical section object.

Return : Nothing.
*/
inline void DeleteCriticalSection(CRITICAL_SECTION* pCriticalSection)
{
	if (pthread_mutex_destroy(pCriticalSection) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_WARNING_OSCRITICALSECTION(("DeleteCriticalSection warning (%s) : %s"
			"(pCriticalSection=%#x)\n", 
			strtime_m(), 
			"pthread_mutex_destroy failed. ", 
			pCriticalSection));
	}
}
#endif // _WIN32

#ifndef _WIN32
/*
Wait for ownership of the specified critical section object. The function returns when the 
calling thread is granted ownership.

CRITICAL_SECTION* pCriticalSection : (INOUT) Pointer to the critical section object.

Return : Nothing.
*/
inline void EnterCriticalSection(CRITICAL_SECTION* pCriticalSection)
{
	if (pthread_mutex_lock(pCriticalSection) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_WARNING_OSCRITICALSECTION(("EnterCriticalSection warning (%s) : %s"
			"(pCriticalSection=%#x)\n", 
			strtime_m(), 
			"pthread_mutex_lock failed. ", 
			pCriticalSection));
	}
}
#endif // _WIN32

#ifndef _WIN32
/*
Attempt to enter a critical section without blocking. If the call is successful, the calling 
thread takes ownership of the critical section.

CRITICAL_SECTION* pCriticalSection : (INOUT) Pointer to the critical section object.

Return : If the critical section is successfully entered or the current thread already owns the 
critical section, the return value is !=0. If another thread already owns the critical section, 
the return value is 0.
*/
inline BOOL TryEnterCriticalSection(CRITICAL_SECTION* pCriticalSection)
{
	int iResult = pthread_mutex_trylock(pCriticalSection);
	switch (iResult)	 
	{
	case EXIT_SUCCESS: 
		break;
	case EBUSY: 
		return FALSE;
	default:
		PRINT_DEBUG_WARNING_OSCRITICALSECTION(("TryEnterCriticalSection warning (%s) : %s"
			"(pCriticalSection=%#x)\n", 
			strtime_m(), 
			"pthread_mutex_trylock failed. ", 
			pCriticalSection));
		return FALSE;
	}

	return TRUE;
}
#endif // _WIN32

#ifndef _WIN32
/*
Release ownership of the specified critical section object.
After a thread has ownership of a critical section, it can make additional calls to 
EnterCriticalSection() or TryEnterCriticalSection() without blocking its execution,
but it must call LeaveCriticalSection() once for each time that it entered the critical 
section. A thread that currently do not have ownership of the critical section object 
should not call LeaveCriticalSection().

CRITICAL_SECTION* pCriticalSection : (INOUT) Pointer to the critical section object.

Return : Nothing.
*/
inline void LeaveCriticalSection(CRITICAL_SECTION* pCriticalSection)
{
	if (pthread_mutex_unlock(pCriticalSection) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_WARNING_OSCRITICALSECTION(("LeaveCriticalSection warning (%s) : %s"
			"(pCriticalSection=%#x)\n", 
			strtime_m(), 
			"pthread_mutex_unlock failed. ", 
			pCriticalSection));
	}
}
#endif // _WIN32

/*
Thread-safe version of memcpy. Wait for ownership of the specified critical section object 
and copy the first Size bytes from pSrc to pDest. pDest or pSrc should be the shared resource 
protected by the critical section. Release the ownership of the critical section object when
it returns.

void* pDest : (OUT) New buffer.
void* pSrc : (IN) Buffer to copy from.
size_t Size : (IN) Number of bytes to copy.
CRITICAL_SECTION* pCriticalSection : (INOUT) Pointer to the critical section object.

Return : Nothing.
*/
inline void memcpy_ts(void* pDest, void* pSrc, size_t Size, CRITICAL_SECTION* pCriticalSection)	
{
	// Request ownership of the critical section.
	EnterCriticalSection(pCriticalSection);

	// Access the shared resource.
	memcpy(pDest, pSrc, Size);

	// Release ownership of the critical section.
	LeaveCriticalSection(pCriticalSection);
}

#endif // OSCRITICALSECTION_H
