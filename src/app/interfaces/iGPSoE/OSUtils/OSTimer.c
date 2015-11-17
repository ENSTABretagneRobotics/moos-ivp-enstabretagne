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

#include "OSTimer.h"

#ifndef _WIN32
void* _TimerCallbackThreadProc(void* pParam)
{
	TIMERTHREADPARAM* pTimerThreadParam = (TIMERTHREADPARAM*)pParam;

	if (pthread_detach(pthread_self()) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTIMER(("_TimerCallbackThreadProc error (%s) : %s"
			"(pParam=%#x)\n", 
			strtime_m(), 
			"pthread_detach failed. ", 
			pParam));
		return 0;
	}

	pTimerThreadParam->CallbackFunction(pTimerThreadParam->pCallbackParam, TRUE);

	if (pthread_mutex_lock(&pTimerThreadParam->mutex) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTIMER(("_TimerCallbackThreadProc error (%s) : %s"
			"(pParam=%#x)\n", 
			strtime_m(), 
			"pthread_mutex_lock failed. ", 
			pParam));
		return 0;
	}
	//PRINT_DEBUG_MESSAGE_OSTIMER(("nbTimerCallbackThreads=%d\n", pTimerThreadParam->nbTimerCallbackThreads));
	pTimerThreadParam->nbTimerCallbackThreads--;
	if (pTimerThreadParam->nbTimerCallbackThreads <= 0)
	{
		if (pthread_cond_broadcast(&pTimerThreadParam->cv) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("_TimerCallbackThreadProc error (%s) : %s"
				"(pParam=%#x)\n", 
				strtime_m(), 
				"pthread_cond_broadcast failed. ", 
				pParam));
			pthread_mutex_unlock(&pTimerThreadParam->mutex);
			return 0;
		}
	}
	if (pthread_mutex_unlock(&pTimerThreadParam->mutex) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTIMER(("_TimerCallbackThreadProc error (%s) : %s"
			"(pParam=%#x)\n", 
			strtime_m(), 
			"pthread_mutex_unlock failed. ", 
			pParam));
		return 0;
	}

	return 0;
}

#ifndef USE_OLD_TIMER
void* _TimerThreadProc(void* pParam)
{
	TIMERTHREADPARAM* pTimerThreadParam = (TIMERTHREADPARAM*)pParam;
	pthread_t TimerCallbackThreadId;
	uint64_t exp = 0;
	ssize_t s = 0;

	for (;;)
	{
		exp = 0;
		// Wait for timer expirations.
		s = read(pTimerThreadParam->timerfd, &exp, sizeof(uint64_t));
		if (s != sizeof(uint64_t))
		{	
			PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
				"(pParam=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pParam));
		}
		else
		{
			// exp should contain the number of expirations that have occurred.
			while (exp--)
			{
				//pTimerThreadParam->CallbackFunction(pTimerThreadParam->pCallbackParam, TRUE);
				if (pTimerThreadParam->nbTimerCallbackThreads < MAX_NB_TIMER_CALLBACK_THREADS-1)
				{
					if (pthread_mutex_lock(&pTimerThreadParam->mutex) != EXIT_SUCCESS)
					{
						PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
							"(pParam=%#x)\n", 
							strtime_m(), 
							"pthread_mutex_lock failed. ", 
							pParam));
					}
					pTimerThreadParam->nbTimerCallbackThreads++;
					if (pthread_mutex_unlock(&pTimerThreadParam->mutex) != EXIT_SUCCESS)
					{
						PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
							"(pParam=%#x)\n", 
							strtime_m(), 
							"pthread_mutex_unlock failed. ", 
							pParam));
					}
					if (pthread_create(&TimerCallbackThreadId, NULL, _TimerCallbackThreadProc, pTimerThreadParam) != EXIT_SUCCESS)
					{
#ifdef TIMER_THREAD_PROC_DEBUG
						// Because printf() may be a cancellation point, there might be a deadlock here
						// if DeleteTimer() is called in the same time (nbTimerCallbackThreads will never be 0).
						PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
							"(pParam=%#x)\n", 
							strtime_m(), 
							"pthread_create failed. ", 
							pParam));
#endif // TIMER_THREAD_PROC_DEBUG
						if (pthread_mutex_lock(&pTimerThreadParam->mutex) != EXIT_SUCCESS)
						{
							PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
								"(pParam=%#x)\n", 
								strtime_m(), 
								"pthread_mutex_lock failed. ", 
								pParam));
						}
						pTimerThreadParam->nbTimerCallbackThreads--;
						if (pTimerThreadParam->nbTimerCallbackThreads <= 0)
						{
							if (pthread_cond_broadcast(&pTimerThreadParam->cv) != EXIT_SUCCESS)
							{
								PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
									"(pParam=%#x)\n", 
									strtime_m(), 
									"pthread_cond_broadcast failed. ", 
									pParam));
								pthread_mutex_unlock(&pTimerThreadParam->mutex);
							}
						}
						if (pthread_mutex_unlock(&pTimerThreadParam->mutex) != EXIT_SUCCESS)
						{
							PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
								"(pParam=%#x)\n", 
								strtime_m(), 
								"pthread_mutex_unlock failed. ", 
								pParam));
						}
					}		
				}
			}
		}
	} // for (;;)

	return 0;
}
#else
void* _TimerThreadProc(void* pParam)
{
	TIMERTHREADPARAM* pTimerThreadParam = (TIMERTHREADPARAM*)pParam;
	pthread_t TimerCallbackThreadId;
	struct timespec req;

	// usleep() is considered as obsolete.
	//usleep(pTimerThreadParam->DueTime*1000);
	req.tv_sec = pTimerThreadParam->DueTime/1000; // Seconds.
	req.tv_nsec = (pTimerThreadParam->DueTime%1000)*1000000; // Additional nanoseconds.
	nanosleep(&req, NULL);

	//pTimerThreadParam->CallbackFunction(pTimerThreadParam->pCallbackParam, TRUE);
	if (pTimerThreadParam->nbTimerCallbackThreads < MAX_NB_TIMER_CALLBACK_THREADS-1)
	{
		if (pthread_mutex_lock(&pTimerThreadParam->mutex) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
				"(pParam=%#x)\n", 
				strtime_m(), 
				"pthread_mutex_lock failed. ", 
				pParam));
		}
		pTimerThreadParam->nbTimerCallbackThreads++;
		if (pthread_mutex_unlock(&pTimerThreadParam->mutex) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
				"(pParam=%#x)\n", 
				strtime_m(), 
				"pthread_mutex_unlock failed. ", 
				pParam));
		}
		if (pthread_create(&TimerCallbackThreadId, NULL, _TimerCallbackThreadProc, pTimerThreadParam) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
				"(pParam=%#x)\n", 
				strtime_m(), 
				"pthread_create failed. ", 
				pParam));
			if (pthread_mutex_lock(&pTimerThreadParam->mutex) != EXIT_SUCCESS)
			{
				PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
					"(pParam=%#x)\n", 
					strtime_m(), 
					"pthread_mutex_lock failed. ", 
					pParam));
			}
			pTimerThreadParam->nbTimerCallbackThreads--;
			if (pTimerThreadParam->nbTimerCallbackThreads <= 0)
			{
				if (pthread_cond_broadcast(&pTimerThreadParam->cv) != EXIT_SUCCESS)
				{
					PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
						"(pParam=%#x)\n", 
						strtime_m(), 
						"pthread_cond_broadcast failed. ", 
						pParam));
					pthread_mutex_unlock(&pTimerThreadParam->mutex);
				}
			}
			if (pthread_mutex_unlock(&pTimerThreadParam->mutex) != EXIT_SUCCESS)
			{
				PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
					"(pParam=%#x)\n", 
					strtime_m(), 
					"pthread_mutex_unlock failed. ", 
					pParam));
			}
		}
	}

	// If the period of the timer is 0, the timer is signaled once. 
	// If >0, the timer is periodic.	
	if (pTimerThreadParam->Period == 0)
	{
		return 0;
	}

	for (;;)
	{
		// usleep() is considered as obsolete.
		//usleep(pTimerThreadParam->Period*1000);
		req.tv_sec = pTimerThreadParam->Period/1000; // Seconds.
		req.tv_nsec = (pTimerThreadParam->Period%1000)*1000000; // Additional nanoseconds.
		nanosleep(&req, NULL);

		//pTimerThreadParam->CallbackFunction(pTimerThreadParam->pCallbackParam, TRUE);
		if (pTimerThreadParam->nbTimerCallbackThreads < MAX_NB_TIMER_CALLBACK_THREADS-1)
		{
			if (pthread_mutex_lock(&pTimerThreadParam->mutex) != EXIT_SUCCESS)
			{
				PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
					"(pParam=%#x)\n", 
					strtime_m(), 
					"pthread_mutex_lock failed. ", 
					pParam));
			}
			pTimerThreadParam->nbTimerCallbackThreads++;
			if (pthread_mutex_unlock(&pTimerThreadParam->mutex) != EXIT_SUCCESS)
			{
				PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
					"(pParam=%#x)\n", 
					strtime_m(), 
					"pthread_mutex_unlock failed. ", 
					pParam));
			}
			if (pthread_create(&TimerCallbackThreadId, NULL, _TimerCallbackThreadProc, pTimerThreadParam) != EXIT_SUCCESS)
			{
#ifdef TIMER_THREAD_PROC_DEBUG
				// Because printf() may be a cancellation point, there might be a deadlock here
				// if DeleteTimer() is called in the same time (nbTimerCallbackThreads will never be 0).
				PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
					"(pParam=%#x)\n", 
					strtime_m(), 
					"pthread_create failed. ", 
					pParam));
#endif // TIMER_THREAD_PROC_DEBUG
				if (pthread_mutex_lock(&pTimerThreadParam->mutex) != EXIT_SUCCESS)
				{
					PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
						"(pParam=%#x)\n", 
						strtime_m(), 
						"pthread_mutex_lock failed. ", 
						pParam));
				}
				pTimerThreadParam->nbTimerCallbackThreads--;
				if (pTimerThreadParam->nbTimerCallbackThreads <= 0)
				{
					if (pthread_cond_broadcast(&pTimerThreadParam->cv) != EXIT_SUCCESS)
					{
						PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
							"(pParam=%#x)\n", 
							strtime_m(), 
							"pthread_cond_broadcast failed. ", 
							pParam));
						pthread_mutex_unlock(&pTimerThreadParam->mutex);
					}
				}
				if (pthread_mutex_unlock(&pTimerThreadParam->mutex) != EXIT_SUCCESS)
				{
					PRINT_DEBUG_ERROR_OSTIMER(("_TimerThreadProc error (%s) : %s"
						"(pParam=%#x)\n", 
						strtime_m(), 
						"pthread_mutex_unlock failed. ", 
						pParam));
				}
			}
		}
	}

	return 0;
}
#endif // USE_OLD_TIMER
#endif // _WIN32
