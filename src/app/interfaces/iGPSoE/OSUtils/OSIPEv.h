/***************************************************************************************************************:')

OSIPEv.c

Interprocess events handling.

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

#ifndef OSIPEV_H
#define OSIPEV_H

#include "OSTime.h"

/*
Debug macros specific to OSIPEv.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSIPEV
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSIPEV
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSIPEV
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSIPEV
#	define PRINT_DEBUG_MESSAGE_OSIPEV(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSIPEV(params)
#endif // _DEBUG_MESSAGES_OSIPEV

#ifdef _DEBUG_WARNINGS_OSIPEV
#	define PRINT_DEBUG_WARNING_OSIPEV(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSIPEV(params)
#endif // _DEBUG_WARNINGS_OSIPEV

#ifdef _DEBUG_ERRORS_OSIPEV
#	define PRINT_DEBUG_ERROR_OSIPEV(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSIPEV(params)
#endif // _DEBUG_ERRORS_OSIPEV

#ifdef _WIN32
#else 
#include <fcntl.h> // For O_* constants.
#include <sys/stat.h> // For mode constants.
#include <semaphore.h>
#endif // _WIN32

#define MAX_IPEV_TIMEOUT (LONG_MAX-2)
#define MAX_IPEV_NAME_LENGTH (128-8)

#ifdef _WIN32
typedef HANDLE IPEVENT;
#else
// This might need to be in an interprocess shared memeory.
struct IPEVENT
{
	sem_t* pSemNbWaitingTasks;
	sem_t* pSemBroadcast;
	sem_t* pSemProtect;
	char szSemNbWaitingTasksName[MAX_IPEV_NAME_LENGTH+8];
	char szSemBroadcastName[MAX_IPEV_NAME_LENGTH+8];
	char szSemProtectName[MAX_IPEV_NAME_LENGTH+8];
	char szEvName[MAX_IPEV_NAME_LENGTH+8];
};
typedef struct IPEVENT IPEVENT;
#endif // _WIN32

/*
Create or open a named interprocess event. Use DeleteIPEvent() to 
delete it at the end.

IPEVENT* pEvent : (INOUT) Valid pointer that will receive a structure 
corresponding to an event.
BOOL bInitialStateSignaled : (IN) TRUE if the initial state of the event must be 
signaled, FALSE otherwise.
char* szName : (IN) Name of the event.

Return : EXIT_SUCCESS, EXIT_NAME_TOO_LONG or EXIT_FAILURE.
*/
inline int CreateOrOpenIPEvent(IPEVENT* pEvent, BOOL bInitialStateSignaled, char* szName)
{
#ifdef _WIN32
	HANDLE hEvent;
	char szEvName[MAX_IPEV_NAME_LENGTH+8];

	if (strlen(szName) > MAX_IPEV_NAME_LENGTH)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("CreateOrOpenIPEvent error (%s) : %s"
			"(bInitialStateSignaled=%d, szName=%s)\n", 
			strtime_m(), 
			szOSUtilsErrMsgs[EXIT_NAME_TOO_LONG], 
			(int)bInitialStateSignaled, szName));
		return EXIT_NAME_TOO_LONG;
	}

	// Prefixing the name with "Global\" allows processes to communicate with each 
	// other even if they are in different terminal server sessions (e.g. several 
	// remote desktop connections).
	sprintf(szEvName, "Global\\%s", szName);

	hEvent = CreateEvent( 
		NULL, // Default security attributes.
		TRUE, // If this parameter is TRUE, the function creates a manual-reset event object, 
		// which requires the use of ResetEvent() to set the event state to nonsignaled.
		// If this parameter is FALSE, the function creates an auto-reset event object,
		// and system automatically resets the event state to nonsignaled after a single waiting
		// thread has been released.
		bInitialStateSignaled, // If FALSE, it is initially not signaled, otherwise it is signaled.
		szEvName); // Named event.

	if (hEvent == NULL) 
	{
		PRINT_DEBUG_ERROR_OSIPEV(("CreateOrOpenIPEvent error (%s) : %s"
			"(bInitialStateSignaled=%d, szName=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			(int)bInitialStateSignaled, szName));
		return EXIT_FAILURE;
	}

	*pEvent = hEvent;
#else 
	if (strlen(szName) > MAX_IPEV_NAME_LENGTH)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("CreateOrOpenIPEvent error (%s) : %s"
			"(bInitialStateSignaled=%d, szName=%s)\n", 
			strtime_m(), 
			szOSUtilsErrMsgs[EXIT_NAME_TOO_LONG], 
			(int)bInitialStateSignaled, szName));
		return EXIT_NAME_TOO_LONG;
	}

	// szName must be like "/nameXXX".
	sprintf(pEvent->szSemNbWaitingTasksName, "/SN%s", szName);
	sprintf(pEvent->szSemBroadcastName, "/SB%s", szName);
	sprintf(pEvent->szSemProtectName, "/SP%s", szName);

	// Counter used to store between processes the number of tasks that are waiting for the event.
	pEvent->pSemNbWaitingTasks = sem_open(pEvent->szSemNbWaitingTasksName, O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO, 0);

	if (pEvent->pSemNbWaitingTasks == SEM_FAILED)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("CreateOrOpenIPEvent error (%s) : %s"
			"(bInitialStateSignaled=%d, szName=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			(int)bInitialStateSignaled, szName));
		return EXIT_FAILURE;
	}

	// Used to signal event to waiting tasks.
	pEvent->pSemBroadcast = sem_open(pEvent->szSemBroadcastName, O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO, bInitialStateSignaled);

	if (pEvent->pSemBroadcast == SEM_FAILED)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("CreateOrOpenIPEvent error (%s) : %s"
			"(bInitialStateSignaled=%d, szName=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			(int)bInitialStateSignaled, szName));
		sem_close(pEvent->pSemNbWaitingTasks);
		sem_unlink(pEvent->szSemNbWaitingTasksName);
		return EXIT_FAILURE;
	}

	// Used as an interprocess protection semaphore.
	pEvent->pSemProtect = sem_open(pEvent->szSemProtectName, O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO, 1);

	if (pEvent->pSemProtect == SEM_FAILED)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("CreateOrOpenIPEvent error (%s) : %s"
			"(bInitialStateSignaled=%d, szName=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			(int)bInitialStateSignaled, szName));
		sem_close(pEvent->pSemBroadcast);
		sem_close(pEvent->pSemNbWaitingTasks);
		sem_unlink(pEvent->szSemBroadcastName);
		sem_unlink(pEvent->szSemNbWaitingTasksName);
		return EXIT_FAILURE;
	}

	sprintf(pEvent->szEvName, "%s", szName);
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Delete an interprocess event created by CreateOrOpenIPEvent().

IPEVENT* pEvent : (INOUT) Valid pointer to a structure corresponding to 
an event.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int DeleteIPEvent(IPEVENT* pEvent)
{
#ifdef _WIN32
	if (!CloseHandle(*pEvent))
	{
		PRINT_DEBUG_ERROR_OSIPEV(("DeleteIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}
#else 
	if (sem_close(pEvent->pSemProtect) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("DeleteIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}

	if (sem_close(pEvent->pSemBroadcast) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("DeleteIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}

	if (sem_close(pEvent->pSemNbWaitingTasks) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("DeleteIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}

	// The semaphore name is removed immediately. The semaphore is destroyed
	// once all other processes that have the semaphore open close it.
	sem_unlink(pEvent->szSemProtectName);
	//if (sem_unlink(pEvent->szSemProtectName) != EXIT_SUCCESS)
	//{
	//	PRINT_DEBUG_ERROR_OSIPEV(("DeleteIPEvent error (%s) : %s"
	//		"(pEvent=%#x)\n", 
	//		strtime_m(), 
	//		GetLastErrorMsg(), 
	//		pEvent));
	//	return EXIT_FAILURE;
	//}

	sem_unlink(pEvent->szSemBroadcastName);
	//if (sem_unlink(pEvent->szSemBroadcastName) != EXIT_SUCCESS)
	//{
	//	PRINT_DEBUG_ERROR_OSIPEV(("DeleteIPEvent error (%s) : %s"
	//		"(pEvent=%#x)\n", 
	//		strtime_m(), 
	//		GetLastErrorMsg(), 
	//		pEvent));
	//	return EXIT_FAILURE;
	//}

	sem_unlink(pEvent->szSemNbWaitingTasksName);
	//if (sem_unlink(pEvent->szSemNbWaitingTasksName) != EXIT_SUCCESS)
	//{
	//	PRINT_DEBUG_ERROR_OSIPEV(("DeleteIPEvent error (%s) : %s"
	//		"(pEvent=%#x)\n", 
	//		strtime_m(), 
	//		GetLastErrorMsg(), 
	//		pEvent));
	//	return EXIT_FAILURE;
	//}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Set an event to a signaled state.

IPEVENT* pEvent : (INOUT) Valid pointer to a structure corresponding to 
an event.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SignalIPEvent(IPEVENT* pEvent)
{
#ifdef _WIN32
	if (!SetEvent(*pEvent))	
	{
		PRINT_DEBUG_ERROR_OSIPEV(("SignalIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}
#else 
	int flag = 0;
	int nbWaitingTasks = 0;

	// Lock the protection semaphore before accessing the flag value.
	if (sem_wait(pEvent->pSemProtect) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("SignalIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}

	// Get the event state.
	if (sem_getvalue(pEvent->pSemBroadcast, &flag) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("SignalIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}

	if (flag <= 0)
	{
		int i = 0;

		// Get the current number of waiting tasks.
		if (sem_getvalue(pEvent->pSemNbWaitingTasks, &nbWaitingTasks) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSIPEV(("SignalIPEvent error (%s) : %s"
				"(pEvent=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pEvent));
			return EXIT_FAILURE;
		}

		// Set the event to the signaled state.
		//flag = TRUE;
		// Signal to each waiting task that the event is now signaled.
		// +1 to signal it even if there are currently no waiting tasks.
		i = nbWaitingTasks+1;
		while (i > 0)
		{
			if (sem_post(pEvent->pSemBroadcast) != EXIT_SUCCESS)
			{
				PRINT_DEBUG_ERROR_OSIPEV(("SignalIPEvent error (%s) : %s"
					"(pEvent=%#x)\n", 
					strtime_m(), 
					GetLastErrorMsg(), 
					pEvent));
				return EXIT_FAILURE;
			}
			i--;
		}
	}

	if (sem_post(pEvent->pSemProtect) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("SignalIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Set an event to a nonsignaled state.

IPEVENT* pEvent : (INOUT) Valid pointer to a structure corresponding to 
an event.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int UnSignalIPEvent(IPEVENT* pEvent)
{
#ifdef _WIN32
	if (!ResetEvent(*pEvent))	
	{
		PRINT_DEBUG_ERROR_OSIPEV(("UnSignalIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}
#else 
	int flag = 0;

	// Lock the protection semaphore before accessing the flag value.
	if (sem_wait(pEvent->pSemProtect) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("UnSignalIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}

	// Get the event state.
	if (sem_getvalue(pEvent->pSemBroadcast, &flag) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("UnSignalIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}

	if (flag > 0)
	{
		// Set the event to the nonsignaled state.
		//flag = 0;
		// Reset the broadcasting semaphore (might be several times in case it was still set 
		// because of wait timeouts...).
		while (sem_trywait(pEvent->pSemBroadcast) == EXIT_SUCCESS);
	}

	if (sem_post(pEvent->pSemProtect) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("UnSignalIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Check if an event is signaled.

IPEVENT* pEvent : (INOUT) Valid pointer to a structure corresponding to 
an event.

Return : EXIT_SUCCESS if the event is signaled, EXIT_OBJECT_NONSIGNALED if it 
is not signaled or EXIT_FAILURE if there is an error.
*/
inline int CheckIPEvent(IPEVENT* pEvent)
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
		PRINT_DEBUG_ERROR_OSIPEV(("CheckIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}
#else 
	int flag = 0;

	// Lock the protection semaphore before accessing the flag value.
	if (sem_wait(pEvent->pSemProtect) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("CheckIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}

	// Get the event state.
	if (sem_getvalue(pEvent->pSemBroadcast, &flag) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("CheckIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}

	if (flag <= 0)
	{
		// The flag is clear. Unlock the protection semaphore and return that 
		// the event is nonsignaled.
		if (sem_post(pEvent->pSemProtect) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSIPEV(("CheckIPEvent error (%s) : %s"
				"(pEvent=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pEvent));
			return EXIT_FAILURE;
		}
		return EXIT_OBJECT_NONSIGNALED;
	}

	if (sem_post(pEvent->pSemProtect) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("CheckIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Wait for an event to be signaled or a timeout elapses.

IPEVENT* pEvent : (INOUT) Valid pointer to a structure corresponding to 
an event.
int timeout : (IN) Timeout in ms (max is MAX_IPEV_TIMEOUT).

Return : EXIT_SUCCESS if the event is signaled, EXIT_TIMEOUT if it is not 
signaled after the timeout elapses or EXIT_FAILURE if there is an error.
*/
inline int WaitForIPEvent(IPEVENT* pEvent, int timeout)
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
		PRINT_DEBUG_ERROR_OSIPEV(("WaitForIPEvent error (%s) : %s"
			"(pEvent=%#x, timeout=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent, timeout));
		return EXIT_FAILURE;
	}
#else 
	struct timespec abstime;
	int flag = 0;
	//int nbWaitingTasks = 0;

	// Calculate relative interval as current time plus duration given by timeout.

	if (clock_gettime(CLOCK_REALTIME, &abstime) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("WaitForIPEvent error (%s) : %s"
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

	// Lock the protection semaphore before accessing the flag value.
	if (sem_wait(pEvent->pSemProtect) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("WaitForIPEvent error (%s) : %s"
			"(pEvent=%#x, timeout=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent, timeout));
		return EXIT_FAILURE;
	}

	// Get the event state.
	if (sem_getvalue(pEvent->pSemBroadcast, &flag) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("WaitForIPEvent error (%s) : %s"
			"(pEvent=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent));
		return EXIT_FAILURE;
	}

	if (flag <= 0) 
	{
		// The event is nonsignaled, we will have to wait for it to be signaled or a timeout.

		// Set the current number of waiting tasks.
		//nbWaitingTasks++;
		if (sem_post(pEvent->pSemNbWaitingTasks) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSIPEV(("WaitForIPEvent error (%s) : %s"
				"(pEvent=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pEvent));
			return EXIT_FAILURE;
		}

		if (sem_post(pEvent->pSemProtect) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSIPEV(("WaitForIPEvent error (%s) : %s"
				"(pEvent=%#x, timeout=%d)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pEvent, timeout));
			return EXIT_FAILURE;
		}

		// Wait for a signal indicating that the flag value has changed. When the
		// signal arrives and this process unblocks, indicate that we are not waiting any more.
		if (sem_timedwait(pEvent->pSemBroadcast, &abstime) != EXIT_SUCCESS)
		{
			switch (errno)	 
			{
			case ETIMEDOUT: 
				if (sem_wait(pEvent->pSemProtect) != EXIT_SUCCESS)
				{
					PRINT_DEBUG_ERROR_OSIPEV(("WaitForIPEvent error (%s) : %s"
						"(pEvent=%#x, timeout=%d)\n", 
						strtime_m(), 
						GetLastErrorMsg(), 
						pEvent, timeout));
					return EXIT_FAILURE;
				}
				// Set the current number of waiting tasks.
				//nbWaitingTasks--;
				if (sem_trywait(pEvent->pSemNbWaitingTasks) != EXIT_SUCCESS)
				{
					PRINT_DEBUG_ERROR_OSIPEV(("WaitForIPEvent error (%s) : %s"
						"(pEvent=%#x)\n", 
						strtime_m(), 
						GetLastErrorMsg(), 
						pEvent));
					return EXIT_FAILURE;
				}
				if (sem_post(pEvent->pSemProtect) != EXIT_SUCCESS)
				{
					PRINT_DEBUG_ERROR_OSIPEV(("WaitForIPEvent error (%s) : %s"
						"(pEvent=%#x, timeout=%d)\n", 
						strtime_m(), 
						GetLastErrorMsg(), 
						pEvent, timeout));
					return EXIT_FAILURE;
				}
				return EXIT_TIMEOUT;
			default:
				PRINT_DEBUG_ERROR_OSIPEV(("WaitForIPEvent error (%s) : %s"
					"(pEvent=%#x, timeout=%d)\n", 
					strtime_m(), 
					GetLastErrorMsg(), 
					pEvent, timeout));
				sem_wait(pEvent->pSemProtect);
				//nbWaitingTasks--;
				sem_trywait(pEvent->pSemNbWaitingTasks);
				sem_post(pEvent->pSemProtect);
				return EXIT_FAILURE;
			}
		}
		else
		{
			if (sem_wait(pEvent->pSemProtect) != EXIT_SUCCESS)
			{
				PRINT_DEBUG_ERROR_OSIPEV(("WaitForIPEvent error (%s) : %s"
					"(pEvent=%#x, timeout=%d)\n", 
					strtime_m(), 
					GetLastErrorMsg(), 
					pEvent, timeout));
				return EXIT_FAILURE;
			}
			// Set the current number of waiting tasks.
			//nbWaitingTasks--;
			if (sem_trywait(pEvent->pSemNbWaitingTasks) != EXIT_SUCCESS)
			{
				PRINT_DEBUG_ERROR_OSIPEV(("WaitForIPEvent error (%s) : %s"
					"(pEvent=%#x)\n", 
					strtime_m(), 
					GetLastErrorMsg(), 
					pEvent));
				return EXIT_FAILURE;
			}
		}
	}

	// Here the event is signaled, unlock the protection semaphore.
	if (sem_post(pEvent->pSemProtect) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPEV(("WaitForIPEvent error (%s) : %s"
			"(pEvent=%#x, timeout=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pEvent, timeout));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

#endif // OSIPEV_H
