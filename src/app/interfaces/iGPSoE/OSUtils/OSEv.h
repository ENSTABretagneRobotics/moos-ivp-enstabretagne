/***************************************************************************************************************:')

OSEv.c

Events handling (not interprocess).

Fabrice Le Bars

Created : 2009-04-09

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

#ifndef OSEV_H
#define OSEV_H

#include "OSThread.h"

/*
Debug macros specific to OSEv.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSEV
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSEV
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSEV
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSEV
#	define PRINT_DEBUG_MESSAGE_OSEV(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSEV(params)
#endif // _DEBUG_MESSAGES_OSEV

#ifdef _DEBUG_WARNINGS_OSEV
#	define PRINT_DEBUG_WARNING_OSEV(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSEV(params)
#endif // _DEBUG_WARNINGS_OSEV

#ifdef _DEBUG_ERRORS_OSEV
#	define PRINT_DEBUG_ERROR_OSEV(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSEV(params)
#endif // _DEBUG_ERRORS_OSEV

#define MAX_EV_TIMEOUT (LONG_MAX-2)

#ifdef _WIN32
typedef HANDLE EVENT;
#else
struct EVENT
{
	BOOL flag;
	pthread_cond_t cv;
	pthread_mutex_t mutex;
};
typedef struct EVENT EVENT;
#endif // _WIN32

/*
Create a default event (not interprocess). Use DeleteEvent() to 
delete it at the end.

EVENT* pEvent : (INOUT) Valid pointer that will receive a structure 
corresponding to an event.
BOOL bInitialStateSignaled : (IN) TRUE if the initial state of the event must be 
signaled, FALSE otherwise.

Return : EXIT_SUCCESS or EXIT_FAILURE.
*/
inline int CreateDefaultEvent(EVENT* pEvent, BOOL bInitialStateSignaled)
{
#ifdef _WIN32
	HANDLE hEvent;

	hEvent = CreateEvent( 
		NULL, // Default security attributes.
		TRUE, // If this parameter is TRUE, the function creates a manual-reset event object, 
		// which requires the use of ResetEvent() to set the event state to nonsignaled.
		// If this parameter is FALSE, the function creates an auto-reset event object,
		// and system automatically resets the event state to nonsignaled after a single waiting
		// thread has been released.
		bInitialStateSignaled, // If FALSE, it is initially not signaled, otherwise it is signaled.
		NULL); // Unnamed event.

	if (hEvent == NULL) 
	{
		PRINT_DEBUG_ERROR_OSEV(("CreateDefaultEvent error (%s) : %s"
			"(bInitialStateSignaled=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			(int)bInitialStateSignaled));
		return EXIT_FAILURE;
	}

	*pEvent = hEvent;
#else 
	if (pthread_mutex_init(&pEvent->mutex, NULL) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSEV(("CreateDefaultEvent error (%s) : %s"
			"(bInitialStateSignaled=%d)\n", 
			strtime_m(), 
			"pthread_mutex_init failed. ", 
			(int)bInitialStateSignaled));
		return EXIT_FAILURE;
	}

	if (pthread_cond_init(&pEvent->cv, NULL) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSEV(("CreateDefaultEvent error (%s) : %s"
			"(bInitialStateSignaled=%d)\n", 
			strtime_m(), 
			"pthread_cond_init failed. ", 
			(int)bInitialStateSignaled));
		pthread_mutex_destroy(&pEvent->mutex);
		return EXIT_FAILURE;
	}

	pEvent->flag = bInitialStateSignaled;
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Delete an event created by CreateDefaultEvent().

EVENT* pEvent : (INOUT) Valid pointer to a structure corresponding to 
an event.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int DeleteEvent(EVENT* pEvent)
{
#ifdef _WIN32
	if (!CloseHandle(*pEvent))
	{
		PRINT_DEBUG_ERROR_OSEV(("DeleteEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}
#else 
	if (pthread_cond_destroy(&pEvent->cv) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSEV(("DeleteEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			"pthread_cond_destroy failed. ", 
			pEvent));
		return EXIT_FAILURE;
	}

	if (pthread_mutex_destroy(&pEvent->mutex) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSEV(("DeleteEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			"pthread_mutex_destroy failed. ", 
			pEvent));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Set an event to a signaled state. All threads that were waiting or 
will try to wait or check for the event using CheckEvent() or WaitForEvent()
should be released unless UnSignalEvent() is called between SignalEvent() 
and CheckEvent() or WaitForEvent().

EVENT* pEvent : (INOUT) Valid pointer to a structure corresponding to 
an event.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SignalEvent(EVENT* pEvent)
{
#ifdef _WIN32
	if (!SetEvent(*pEvent))	
	{
		PRINT_DEBUG_ERROR_OSEV(("SignalEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}
#else 
	// Lock the mutex before accessing the flag value.
	if (pthread_mutex_lock(&pEvent->mutex) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSEV(("SignalEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			"pthread_mutex_lock failed. ", 
			pEvent));
		return EXIT_FAILURE;
	}

	if (!pEvent->flag)
	{
		// Set the flag value, and then signal in case a thread is
		// blocked, waiting for the flag to become set.
		pEvent->flag = TRUE;

		if (pthread_cond_broadcast(&pEvent->cv) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSEV(("SignalEvent error (%s) : %s"
				"(pEvent=%#x)\n", 
				strtime_m(), 
				"pthread_cond_broadcast failed. ", 
				pEvent));
			pthread_mutex_unlock(&pEvent->mutex);
			return EXIT_FAILURE;
		}
	}

	// Unlock the mutex.
	if (pthread_mutex_unlock(&pEvent->mutex) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSEV(("SignalEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			"pthread_mutex_unlock failed. ", 
			pEvent));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Set an event to a nonsignaled state.

EVENT* pEvent : (INOUT) Valid pointer to a structure corresponding to 
an event.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int UnSignalEvent(EVENT* pEvent)
{
#ifdef _WIN32
	if (!ResetEvent(*pEvent))	
	{
		PRINT_DEBUG_ERROR_OSEV(("UnSignalEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}
#else 
	// Lock the mutex before accessing the flag value.
	if (pthread_mutex_lock(&pEvent->mutex) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSEV(("UnSignalEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			"pthread_mutex_lock failed. ", 
			pEvent));
		return EXIT_FAILURE;
	}

	// Set the flag value. If it was previously signaled, no thread should 
	// be waiting and if it was nonsignaled, it should not change anything 
	// so there is no need to call pthread_cond_broadcast().
	pEvent->flag = FALSE;

	// Unlock the mutex.
	if (pthread_mutex_unlock(&pEvent->mutex) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSEV(("UnSignalEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			"pthread_mutex_unlock failed. ", 
			pEvent));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Check if an event is signaled.

EVENT* pEvent : (INOUT) Valid pointer to a structure corresponding to 
an event.

Return : EXIT_SUCCESS if the event is signaled, EXIT_OBJECT_NONSIGNALED if it 
is not signaled or EXIT_FAILURE if there is an error.
*/
inline int CheckEvent(EVENT* pEvent)
{
#ifdef _WIN32
	DWORD dwWaitResult = WAIT_FAILED; 

	// Return when the specified object is in the signaled state or the time-out interval elapses.
	// In this case, we just check if the event is signaled.
	dwWaitResult = WaitForSingleObject(
		*pEvent, // Handle to the event.
		(DWORD)0); // Time-out interval (ms).

	switch (dwWaitResult)	 
	{
	case WAIT_OBJECT_0: 
		// The event object was signaled.
		break; 
	case WAIT_TIMEOUT: 
		// The event was nonsignaled, so a time-out occurred.
		return EXIT_OBJECT_NONSIGNALED; 
	default:
		PRINT_DEBUG_ERROR_OSEV(("CheckEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}
#else 
	// Lock the mutex before accessing the flag value.
	if (pthread_mutex_lock(&pEvent->mutex) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSEV(("CheckEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			"pthread_mutex_lock failed. ", 
			pEvent));
		return EXIT_FAILURE;
	}

	if (!pEvent->flag)
	{
		// The flag is clear. Unlock the mutex and return that the event is nonsignaled.
		if (pthread_mutex_unlock(&pEvent->mutex) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSEV(("CheckEvent error (%s) : %s"
				"(pEvent=%#x)\n", 
				strtime_m(), 
				"pthread_mutex_unlock failed. ", 
				pEvent));
			return EXIT_FAILURE;
		}
		return EXIT_OBJECT_NONSIGNALED;
	}

	// When we have gotten here, we know the flag must be set. Unlock the mutex.
	if (pthread_mutex_unlock(&pEvent->mutex) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSEV(("CheckEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			"pthread_mutex_unlock failed. ", 
			pEvent));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Wait for an event to be signaled or a timeout elapse.

EVENT* pEvent : (INOUT) Valid pointer to a structure corresponding to 
an event.
int timeout : (IN) Timeout in ms (max is MAX_EV_TIMEOUT).

Return : EXIT_SUCCESS if the event is signaled, EXIT_TIMEOUT if it is not 
signaled after the timeout elapses or EXIT_FAILURE if there is an error.
*/
inline int WaitForEvent(EVENT* pEvent, int timeout)
{
#ifdef _WIN32
	DWORD dwWaitResult = WAIT_FAILED; 

	// Returns when the specified object is in the signaled state or the time-out interval elapses.
	dwWaitResult = WaitForSingleObject(
		*pEvent, // Handle to the event.
		(DWORD)timeout); // Time-out interval (ms).

	switch (dwWaitResult)	 
	{
	case WAIT_OBJECT_0: 
		// The event object was signaled.
		break; 
	case WAIT_TIMEOUT: 
		// The event was nonsignaled, so a time-out occurred.
		return EXIT_TIMEOUT; 
	default:
		PRINT_DEBUG_ERROR_OSEV(("WaitForEvent error (%s) : %s"
			"(pEvent=%#x, timeout=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent, timeout));
		return EXIT_FAILURE;
	}
#else 
	struct timespec abstime;

	// Calculate relative interval as current time plus duration given by timeout.

	if (clock_gettime(CLOCK_REALTIME, &abstime) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSEV(("WaitForEvent error (%s) : %s"
			"(pEvent=%#x, timeout=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent, timeout));
		return EXIT_FAILURE;
	}

	abstime.tv_sec += timeout/1000; // Seconds.
	abstime.tv_nsec += (timeout%1000)*1000000; // Additional nanoseconds.
	abstime.tv_sec += abstime.tv_nsec/1000000000;
	abstime.tv_nsec = abstime.tv_nsec%1000000000;

	// Lock the mutex before accessing the flag value.
	if (pthread_mutex_lock(&pEvent->mutex) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSEV(("WaitForEvent error (%s) : %s"
			"(pEvent=%#x, timeout=%d)\n", 
			strtime_m(), 
			"pthread_mutex_lock failed. ", 
			pEvent, timeout));
		return EXIT_FAILURE;
	}

	if (!pEvent->flag)
	{
		// Wait for a signal on the condition variable, indicating that the flag
		// value has changed.
		int iResult = pthread_cond_timedwait(&pEvent->cv, &pEvent->mutex, &abstime);
		switch (iResult)
		{
		case EXIT_SUCCESS: 
			break;
		case ETIMEDOUT: 
			if (pthread_mutex_unlock(&pEvent->mutex) != EXIT_SUCCESS)
			{
				PRINT_DEBUG_ERROR_OSEV(("WaitForEvent error (%s) : %s"
					"(pEvent=%#x, timeout=%d)\n", 
					strtime_m(), 
					"pthread_mutex_unlock failed. ", 
					pEvent, timeout));
				return EXIT_FAILURE;
			}
			return EXIT_TIMEOUT;
		default:
			PRINT_DEBUG_ERROR_OSEV(("WaitForEvent error (%s) : %s"
				"(pEvent=%#x, timeout=%d)\n", 
				strtime_m(), 
				"pthread_cond_timedwait failed. ", 
				pEvent, timeout));
			pthread_mutex_unlock(&pEvent->mutex);
			return EXIT_FAILURE;
		}
	}

	// When we have gotten here, we know the flag is set. Unlock the mutex.
	if (pthread_mutex_unlock(&pEvent->mutex) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSEV(("WaitForEvent error (%s) : %s"
			"(pEvent=%#x, timeout=%d)\n", 
			strtime_m(), 
			"pthread_mutex_unlock failed. ", 
			pEvent, timeout));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

#endif // OSEV_H
