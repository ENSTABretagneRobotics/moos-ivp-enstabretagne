/***************************************************************************************************************:')

OSTimer.c

Timers.

Fabrice Le Bars

Created : 2011-07-30

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

#ifndef OSTIMER_H
#define OSTIMER_H

#include "OSEv.h"

/*
Debug macros specific to OSTimer.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSTIMER
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSTIMER
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSTIMER
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSTIMER
#	define PRINT_DEBUG_MESSAGE_OSTIMER(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSTIMER(params)
#endif // _DEBUG_MESSAGES_OSTIMER

#ifdef _DEBUG_WARNINGS_OSTIMER
#	define PRINT_DEBUG_WARNING_OSTIMER(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSTIMER(params)
#endif // _DEBUG_WARNINGS_OSTIMER

#ifdef _DEBUG_ERRORS_OSTIMER
#	define PRINT_DEBUG_ERROR_OSTIMER(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSTIMER(params)
#endif // _DEBUG_ERRORS_OSTIMER

#define MAX_TIMER_DUETIME (LONG_MAX-2)
#define MAX_TIMER_PERIOD (LONG_MAX-2)

#ifdef _WIN32
#else 
#include <sys/timerfd.h>
#endif // _WIN32

#ifdef _WIN32
#define TIMERCALLBACK_RETURN_VALUE void CALLBACK
typedef WAITORTIMERCALLBACK TIMERCALLBACK;

typedef HANDLE TIMER;
#else
#define MAX_NB_TIMER_CALLBACK_THREADS 500

EXTERN_C void* _TimerThreadProc(void* pParam);

#define TIMERCALLBACK_RETURN_VALUE void
typedef void (*TIMERCALLBACK)(void*, BOOLEAN);   

struct TIMERTHREADPARAM
{
	TIMERCALLBACK CallbackFunction;
	void* pCallbackParam;
	UINT DueTime;
	UINT Period;
	BOOL bIOPending;
	int nbTimerCallbackThreads;
	pthread_cond_t cv;
	pthread_mutex_t mutex;
#ifndef USE_OLD_TIMER
	int timerfd;
#endif // USE_OLD_TIMER
};
typedef struct TIMERTHREADPARAM TIMERTHREADPARAM;

struct TIMER
{
	pthread_t TimerThreadId;
	TIMERTHREADPARAM TimerThreadParam;
};
typedef struct TIMER TIMER;
#endif // _WIN32

/*
Create a timer. Use DeleteTimer() to delete it at the end.

TIMER* pTimer : (INOUT) Valid pointer that will receive a structure 
corresponding to a timer.
TIMERCALLBACK Callback : (IN) Pointer to the application-defined 
function to be executed by the timer. This function should have this prototype : 
TIMERCALLBACK_RETURN_VALUE function(void* pParam, BOOLEAN b);
Note that the BOOLEAN b parameter is currently not used.
void* pParam : (IN) Pointer to a variable to be passed to the thread.
UINT DueTime : (IN) Amount of time to elapse before the timer executes the callback 
function for the first time (in ms, max is MAX_TIMER_DUETIME).
UINT Period : (IN) Period of the timer (in ms, max is MAX_TIMER_PERIOD). If 0, the 
timer is not periodic and the callback function is only executed once.

Return : EXIT_SUCCESS or EXIT_FAILURE.
*/
inline int CreateTimer(TIMER* pTimer, TIMERCALLBACK Callback, void* pParam, UINT DueTime, UINT Period)
{
#ifdef _WIN32
	HANDLE hTimer = INVALID_HANDLE_VALUE;

	if (!CreateTimerQueueTimer( 
		&hTimer, // Pointer to a buffer that receives a handle to the timer-queue timer on return.
		NULL, // Handle to the timer queue. If NULL, the timer is associated with the default timer queue.
		Callback, 
		pParam,
		(DWORD)DueTime, // The amount of time to elapse before the timer is to be set to the signaled state for the first time, in milliseconds.
		(DWORD)Period, // The period of the timer, in milliseconds. If 0, the timer is signaled once. 
		// If >0, the timer is periodic. 
		WT_EXECUTELONGFUNCTION)) // By default, the callback function is queued to a non-I/O worker thread.
		// Here indicate that the callback function can perform a long wait. This flag helps the system to decide if it should create a new thread.
	{
		PRINT_DEBUG_ERROR_OSTIMER(("CreateTimer error (%s) : %s"
			"(Callback=%#x, pParam=%#x, DueTime=%u, Period=%u)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			Callback, pParam, DueTime, Period));
		return EXIT_FAILURE;
	}

	*pTimer = hTimer;
#else 
#ifndef USE_OLD_TIMER
	struct itimerspec its, old;
#endif // USE_OLD_TIMER

	pTimer->TimerThreadParam.CallbackFunction = Callback;
	pTimer->TimerThreadParam.pCallbackParam = pParam;
	pTimer->TimerThreadParam.DueTime = DueTime;
	pTimer->TimerThreadParam.Period = Period;
	pTimer->TimerThreadParam.bIOPending = FALSE;
	pTimer->TimerThreadParam.nbTimerCallbackThreads = 0;

	if (pthread_mutex_init(&pTimer->TimerThreadParam.mutex, NULL) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTIMER(("CreateTimer error (%s) : %s"
			"(Callback=%#x, pParam=%#x, DueTime=%u, Period=%u)\n", 
			strtime_m(), 
			"pthread_mutex_init failed. ", 
			Callback, pParam, DueTime, Period));
		return EXIT_FAILURE;
	}

	if (pthread_cond_init(&pTimer->TimerThreadParam.cv, NULL) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTIMER(("CreateTimer error (%s) : %s"
			"(Callback=%#x, pParam=%#x, DueTime=%u, Period=%u)\n", 
			strtime_m(), 
			"pthread_cond_init failed. ", 
			Callback, pParam, DueTime, Period));
		pthread_mutex_destroy(&pTimer->TimerThreadParam.mutex);
		return EXIT_FAILURE;
	}

#ifndef USE_OLD_TIMER
	pTimer->TimerThreadParam.timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
	if (pTimer->TimerThreadParam.timerfd == -1)
	{
		PRINT_DEBUG_ERROR_OSTIMER(("CreateTimer error (%s) : %s"
			"(Callback=%#x, pParam=%#x, DueTime=%u, Period=%u)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			Callback, pParam, DueTime, Period));
		pthread_cond_destroy(&pTimer->TimerThreadParam.cv);
		pthread_mutex_destroy(&pTimer->TimerThreadParam.mutex);
		return EXIT_FAILURE;
	}

	its.it_value.tv_sec = DueTime/1000; // Seconds.
	its.it_value.tv_nsec = (DueTime%1000)*1000000; // Additional nanoseconds.
	its.it_interval.tv_sec = Period/1000; // Seconds.
	its.it_interval.tv_nsec = (Period%1000)*1000000; // Additional nanoseconds.

	if (timerfd_settime(pTimer->TimerThreadParam.timerfd, 0, &its, &old) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTIMER(("CreateTimer error (%s) : %s"
			"(Callback=%#x, pParam=%#x, DueTime=%u, Period=%u)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			Callback, pParam, DueTime, Period));
		close(pTimer->TimerThreadParam.timerfd);
		pthread_cond_destroy(&pTimer->TimerThreadParam.cv);
		pthread_mutex_destroy(&pTimer->TimerThreadParam.mutex);
		return EXIT_FAILURE;
	}
#endif // USE_OLD_TIMER

	// Create the timer thread.
	if (pthread_create(&pTimer->TimerThreadId, NULL, _TimerThreadProc, &pTimer->TimerThreadParam) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTIMER(("CreateTimer error (%s) : %s"
			"(Callback=%#x, pParam=%#x, DueTime=%u, Period=%u)\n", 
			strtime_m(), 
			"pthread_create failed. ", 
			Callback, pParam, DueTime, Period));
#ifndef USE_OLD_TIMER
		close(pTimer->TimerThreadParam.timerfd);
#endif // USE_OLD_TIMER
		pthread_cond_destroy(&pTimer->TimerThreadParam.cv);
		pthread_mutex_destroy(&pTimer->TimerThreadParam.mutex);
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Delete a timer created by CreateTimer().

TIMER* pTimer : (INOUT) Valid pointer to a structure corresponding to 
a timer.
BOOL bReturnImmediately : (IN) If TRUE, mark the timer for deletion and return immediately.
If FALSE, wait for the timer callback function to complete before returning. 
ONLY FALSE IS CURRENTLY IMPLEMENTED FOR LINUX, the behaviour is unspecified if there are 
outstanding callback functions when this function is called.

Return : EXIT_SUCCESS if the timer was successfully deleted, EXIT_IO_PENDING if there are
outstanding callback functions that will cause the timer to be deleted (automatically) only when  
these callback functions are finished or EXIT_FAILURE if there is an error.
*/
inline int DeleteTimer(TIMER* pTimer, BOOL bReturnImmediately)
{
#ifdef _WIN32
	HANDLE CompletionEvent = bReturnImmediately?NULL:INVALID_HANDLE_VALUE;

	if (!DeleteTimerQueueTimer(
		NULL, // Handle to the timer queue. If NULL, the timer is associated with the default timer queue.
		*pTimer,
		CompletionEvent // If INVALID_HANDLE_VALUE, wait for the timer callback function to complete before returning.
		// If NULL, mark the timer for deletion and return immediately.
		))
	{
		if (GetLastError() == ERROR_IO_PENDING)
		{
			return EXIT_IO_PENDING;
		}
		else
		{
			PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
				"(pTimer=%#x, bReturnImmediately=%d)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pTimer, (int)bReturnImmediately));
			return EXIT_FAILURE;
		}
	}
#else 
#ifndef USE_OLD_TIMER
	//struct itimerspec its, old;

	//// Thread-safe?

	//// Stop the timer.
	//its.it_value.tv_sec = 0; // Seconds.
	//its.it_value.tv_nsec = 0; // Additional nanoseconds.
	//its.it_interval.tv_sec = 0; // Seconds.
	//its.it_interval.tv_nsec = 0; // Additional nanoseconds.
	//if (timerfd_settime(pTimer->TimerThreadParam.timerfd, 0, &its, &old) != EXIT_SUCCESS)
	//{
	//	PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
	//		"(pTimer=%#x, bReturnImmediately=%d)\n", 
	//		strtime_m(), 
	//		GetLastErrorMsg(), 
	//		pTimer, (int)bReturnImmediately));
	//	return EXIT_FAILURE;
	//}
#endif // USE_OLD_TIMER

	pTimer->TimerThreadParam.bIOPending = FALSE;

	// If TRUE, mark the timer for deletion and return immediately (TimerThread is 
	// detachable).
	// If FALSE, wait for the timer callback function to complete before returning 
	// (TimerThread is joinable).
	if (bReturnImmediately)
	{
		// Indicate that storage for the thread can be reclaimed when it terminates.
		if (pthread_detach(pTimer->TimerThreadId) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
				"(pTimer=%#x, bReturnImmediately=%d)\n", 
				strtime_m(), 
				"pthread_detach failed. ", 
				pTimer, (int)bReturnImmediately));
			return EXIT_FAILURE;
		}

		// When initially created, a thread is synchronously cancelable.
		// The thread should only exit when it calls a function that is a 
		// cancellation point (here nanosleep() or read()). 
		if (pthread_cancel(pTimer->TimerThreadId) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
				"(pTimer=%#x, bReturnImmediately=%d)\n", 
				strtime_m(), 
				"pthread_cancel failed. ", 
				pTimer, (int)bReturnImmediately));
			return EXIT_FAILURE;
		}

#ifndef USE_OLD_TIMER
		if (close(pTimer->TimerThreadParam.timerfd) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
				"(pTimer=%#x, bReturnImmediately=%d)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pTimer, (int)bReturnImmediately));
			return EXIT_FAILURE;
		}
#endif // USE_OLD_TIMER

		if (pthread_mutex_lock(&pTimer->TimerThreadParam.mutex) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
				"(pTimer=%#x, bReturnImmediately=%d)\n", 
				strtime_m(), 
				"pthread_mutex_lock failed. ", 
				pTimer, (int)bReturnImmediately));
			return EXIT_FAILURE;
		}

		// If there are outstanding callback functions, the function will return EXIT_IO_PENDING. 
		// Those callbacks either will execute or are in the middle of executing. The timer is 
		// cleaned up when the callback function is finished executing.
		if  (pTimer->TimerThreadParam.nbTimerCallbackThreads > 0)
		{
			pTimer->TimerThreadParam.bIOPending = TRUE;
		}

		if (pthread_mutex_unlock(&pTimer->TimerThreadParam.mutex) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
				"(pTimer=%#x, bReturnImmediately=%d)\n", 
				strtime_m(), 
				"pthread_mutex_unlock failed. ", 
				pTimer, (int)bReturnImmediately));
			return EXIT_FAILURE;
		}

		if (pthread_cond_destroy(&pTimer->TimerThreadParam.cv) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
				"(pTimer=%#x, bReturnImmediately=%d)\n", 
				strtime_m(), 
				"pthread_cond_destroy failed. ", 
				pTimer, (int)bReturnImmediately));
			return EXIT_FAILURE;
		}

		if (pthread_mutex_destroy(&pTimer->TimerThreadParam.mutex) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
				"(pTimer=%#x, bReturnImmediately=%d)\n", 
				strtime_m(), 
				"pthread_mutex_destroy failed. ", 
				pTimer, (int)bReturnImmediately));
			return EXIT_FAILURE;
		}

		if (pTimer->TimerThreadParam.bIOPending)
		{
			// There will be a problem in this case as the TIMER structure might be still used 
			// by the callback threads after exiting this function.
			return EXIT_IO_PENDING;
		}
	}
	else
	{
		// When initially created, a thread is synchronously cancelable.
		// The thread should only exit when it calls a function that is a 
		// cancellation point (here nanosleep() or read()). 
		if (pthread_cancel(pTimer->TimerThreadId) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
				"(pTimer=%#x, bReturnImmediately=%d)\n", 
				strtime_m(), 
				"pthread_cancel failed. ", 
				pTimer, (int)bReturnImmediately));
			return EXIT_FAILURE;
		}

		// Wait until the timer thread has terminated.
		if (pthread_join(pTimer->TimerThreadId, NULL) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
				"(pTimer=%#x, bReturnImmediately=%d)\n", 
				strtime_m(), 
				"pthread_join failed. ", 
				pTimer, (int)bReturnImmediately));
			return EXIT_FAILURE;
		}

#ifndef USE_OLD_TIMER
		if (close(pTimer->TimerThreadParam.timerfd) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
				"(pTimer=%#x, bReturnImmediately=%d)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pTimer, (int)bReturnImmediately));
			return EXIT_FAILURE;
		}
#endif // USE_OLD_TIMER

		if (pthread_mutex_lock(&pTimer->TimerThreadParam.mutex) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
				"(pTimer=%#x, bReturnImmediately=%d)\n", 
				strtime_m(), 
				"pthread_mutex_lock failed. ", 
				pTimer, (int)bReturnImmediately));
			return EXIT_FAILURE;
		}

		while (pTimer->TimerThreadParam.nbTimerCallbackThreads > 0)
		{
			// There are outstanding callback functions. Wait for a signal on the condition
			// variable, indicating that the number of running callback functions has changed. When the
			// signal arrives and this thread unblocks, loop and check again.
			if (pthread_cond_wait(&pTimer->TimerThreadParam.cv, &pTimer->TimerThreadParam.mutex) != EXIT_SUCCESS)
			{
				PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
					"(pTimer=%#x, bReturnImmediately=%d)\n", 
					strtime_m(), 
					"pthread_cond_timedwait failed. ", 
					pTimer, (int)bReturnImmediately));
				pthread_mutex_unlock(&pTimer->TimerThreadParam.mutex);
				return EXIT_FAILURE;
			}
		}

		// When we are here, we know there is no more running callback functions. Unlock the mutex.
		if (pthread_mutex_unlock(&pTimer->TimerThreadParam.mutex) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
				"(pTimer=%#x, bReturnImmediately=%d)\n", 
				strtime_m(), 
				"pthread_mutex_unlock failed. ", 
				pTimer, (int)bReturnImmediately));
			return EXIT_FAILURE;
		}

		if (pthread_cond_destroy(&pTimer->TimerThreadParam.cv) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
				"(pTimer=%#x, bReturnImmediately=%d)\n", 
				strtime_m(), 
				"pthread_cond_destroy failed. ", 
				pTimer, (int)bReturnImmediately));
			return EXIT_FAILURE;
		}

		if (pthread_mutex_destroy(&pTimer->TimerThreadParam.mutex) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("DeleteTimer error (%s) : %s"
				"(pTimer=%#x, bReturnImmediately=%d)\n", 
				strtime_m(), 
				"pthread_mutex_destroy failed. ", 
				pTimer, (int)bReturnImmediately));
			return EXIT_FAILURE;
		}
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

#endif // OSTIMER_H
