/***************************************************************************************************************:')

OSMq.h

Message queue handling (not interprocess).

Fabrice Le Bars

Created: 2009-02-10

Version status: Tested quickly

***************************************************************************************************************:)*/

// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef OSMQ_H
#define OSMQ_H

#include "OSThread.h"

/*
Debug macros specific to OSMq.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSMQ
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSMQ
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSMQ
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSMQ
#	define PRINT_DEBUG_MESSAGE_OSMQ(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSMQ(params)
#endif // _DEBUG_MESSAGES_OSMQ

#ifdef _DEBUG_WARNINGS_OSMQ
#	define PRINT_DEBUG_WARNING_OSMQ(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSMQ(params)
#endif // _DEBUG_WARNINGS_OSMQ

#ifdef _DEBUG_ERRORS_OSMQ
#	define PRINT_DEBUG_ERROR_OSMQ(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSMQ(params)
#endif // _DEBUG_ERRORS_OSMQ

#define MAX_MQ_TIMEOUT (LONG_MAX-2)
#define MAX_MQ_COUNT 2147483646

#ifdef _WIN32
#else 
#include <fcntl.h> // For O_* constants.
#include <sys/stat.h> // For mode constants.
#include <mqueue.h>
#endif // _WIN32

#ifdef _WIN32
typedef DWORD MQ;
THREAD_PROC_RETURN_VALUE MqThrProc(LPVOID lpParameter);
#else
#define MAX_MQ_NAME_LENGTH (128-8)
#define MAX_NB_MSG 10000
#define MAX_MSG_SIZE sizeof(void*)

struct MQ
{
	mqd_t mq;
	char szMqName[MAX_MQ_NAME_LENGTH+8];
};
typedef struct MQ MQ;
#endif // _WIN32

/*
Create a default mq (not interprocess). Use DeleteMq() to delete it at the end.
The maximum number of messages in the queue should be 10000.

MQ* pMq : (INOUT) Valid pointer that will receive a structure 
corresponding to a mq.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
__inline int CreateDefaultMq(MQ* pMq)
{
#ifdef _WIN32
	HANDLE hEvent = NULL; // Handle to the event that will signal that the message 
	// queue associated with the thread is ready.
	HANDLE hThread = NULL; // Handle to the thread associated with the message queue.
	DWORD dwThreadId = 0; // Identifier of the thread associated with the message queue.

	// A message queue is automatically associated with a thread, when it
	// needs it (i.e. use functions such as GetMessage(), PostMessage(),...). It seems
	// impossible to create message queues that whould not depend on a thread.
	// Therefore, we need to create a thread to create a message queue.
	// All the message queues created will have an associated thread that
	// will have the same function. In this function, there will be a call to
	// the PeekMessage() function to ensure that the thread message queue is created.

	// There is a limit of 10000 posted messages per message queue. This value can
	// be changed in the registry, but this would impact all the message queues in the
	// system, therefore it should not be changed.

	// Creates the event that will signal that the message queue associated with the thread is ready.
	hEvent = CreateEvent(
		NULL, // Default security attributes
		FALSE, // If this parameter is TRUE, the function creates a manual-reset event object, 
		// which requires the use of ResetEvent() to set the event state to nonsignaled.
		// If this parameter is FALSE, the function creates an auto-reset event object,
		// and system automatically resets the event state to nonsignaled after a single waiting thread has been released.
		// Not important in this case as we should only have one thread waiting...
		(BOOL)0, // If 0, it is initially not signaled, otherwise it is signaled.
		NULL // Name of the event.
		);

	if (hEvent == NULL)	
	{
		PRINT_DEBUG_ERROR_OSMQ(("CreateDefaultMq error (%s) : %s"
			"\n", 
			strtime_m(), 
			GetLastErrorMsg()));
		return EXIT_FAILURE;
	}

	// Creates the thread object associated with the message queue.

	hThread = (HANDLE)_beginthreadex( 
		NULL, // Default security attributes.
		32, // Stack size.
		MqThrProc, // Thread function.
		(void*)hEvent, // Argument of the thread function. It is the handle to the event.
		0, // Flags. No special flags, so it will start immediately.
		(unsigned int*)&dwThreadId); // Return the thread identifier. 

	if (hThread == NULL)	
	{
		PRINT_DEBUG_ERROR_OSMQ(("CreateDefaultMq error (%s) : %s"
			"\n", 
			strtime_m(), 
			GetLastErrorMsg()));
		CloseHandle(hEvent);
		return EXIT_FAILURE;
	}

	// With CloseHandle(), all the resources associated with the thread will
	// be released only when the thread returns. It does not terminate the thread
	// immediately. 
	if (!CloseHandle(hThread))	
	{
		PRINT_DEBUG_ERROR_OSMQ(("CreateDefaultMq error (%s) : %s"
			"\n", 
			strtime_m(), 
			GetLastErrorMsg()));
		CloseHandle(hEvent);
		return EXIT_FAILURE;
	}

	// Wait for the event that will signal that the message queue associated with the thread is ready.
	if (WaitForSingleObject((HANDLE)hEvent, INFINITE) == WAIT_FAILED)	
	{
		PRINT_DEBUG_ERROR_OSMQ(("CreateDefaultMq error (%s) : %s"
			"\n", 
			strtime_m(), 
			GetLastErrorMsg()));
		CloseHandle(hEvent);
		return EXIT_FAILURE;
	}

	if (!CloseHandle(hEvent))	
	{
		PRINT_DEBUG_ERROR_OSMQ(("CreateDefaultMq error (%s) : %s"
			"\n", 
			strtime_m(), 
			GetLastErrorMsg()));
		return EXIT_FAILURE;
	}

	*pMq = dwThreadId;
#else 
	char szMqName[MAX_MQ_NAME_LENGTH];
	struct mq_attr attr;

	// szName must be like "/nameXXX".
	// Find a better name...?
	sprintf(szMqName, "/%d_%d_%d", (int)rand(), (int)pthread_self(), (int)getpid());

	PRINT_DEBUG_MESSAGE_OSMQ(("szMqName = %s\n", szMqName));

	attr.mq_flags = 0; // Flags: 0 or O_NONBLOCK.
	attr.mq_maxmsg = MAX_NB_MSG; // Max. # of messages on queue.
	attr.mq_msgsize = MAX_MSG_SIZE; // Max. message size (bytes).
	attr.mq_curmsgs = 0; // # of messages currently in queue.

	pMq->mq = mq_open(szMqName, O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO, &attr);

	if (pMq->mq == (mqd_t)-1)
	{
		PRINT_DEBUG_ERROR_OSMQ(("CreateDefaultMq error (%s) : %s"
			"\n", 
			strtime_m(), 
			GetLastErrorMsg()));
		return EXIT_FAILURE;
	}

	sprintf(pMq->szMqName, "%s", szMqName);
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Delete a mq created by CreateDefaultMq().

MQ* pMq : (INOUT) Valid pointer to a structure corresponding to 
a mq.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
__inline int DeleteMq(MQ* pMq)
{
#ifdef _WIN32
	HANDLE hThread = NULL; // Handle to the thread associated with the message queue

	// We need a handle to the thread for the use of WaitForSingleObject().
	// We get it from its identifier mqId.
	// We must call OpenThread() while the thread is still running, as all its
	// associated resources should be freed when it returns, because we 
	// have called CloseHandle() just after its creation.
	hThread = OpenThread(THREAD_ALL_ACCESS, TRUE, *pMq);

	// Checks the return value for success 
	if (hThread == NULL)	
	{
		PRINT_DEBUG_ERROR_OSMQ(("DeleteMq error (%s) : %s"
			"(pMq=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMq));
		return EXIT_FAILURE;
	}

	// To stop the loop in the thread associated with the message queue, we
	// send a special message of type WM_USER+1 with data of 0.
	if (!PostThreadMessage( 
		*pMq, // Identifier of the message queue (i.e the WIN32 thread) to which the message is to be posted
		(UINT)WM_USER+1, // Indicates that it is a custom message (i.e. not a system predefined message)
		(WPARAM)0, // The message data
		(LPARAM)NULL // Not used
		))	
	{
		PRINT_DEBUG_ERROR_OSMQ(("DeleteMq error (%s) : %s"
			"(pMq=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMq));
		CloseHandle(hThread);
		return EXIT_FAILURE;
	}

	// Waits for the thread to return.

	// INFINITE?

	if (WaitForSingleObject(hThread, INFINITE) == WAIT_FAILED)	
	{
		PRINT_DEBUG_ERROR_OSMQ(("DeleteMq error (%s) : %s"
			"(pMq=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMq));
		CloseHandle(hThread);
		return EXIT_FAILURE;
	}

	// As we have opened a handle to the thread with OpenThread(), we should
	// close it to completely release all the resources associated with the thread.
	if (!CloseHandle(hThread))	
	{
		PRINT_DEBUG_ERROR_OSMQ(("DeleteMq error (%s) : %s"
			"(pMq=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMq));
		return EXIT_FAILURE;
	}
#else 
	if (mq_close(pMq->mq) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSMQ(("DeleteMq error (%s) : %s"
			"(pMq=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMq));
		return EXIT_FAILURE;
	}

	// The message queue name is removed immediately. The queue itself is destroyed once 
	// any other processes that have the queue open close their descriptors referring to 
	// the queue.
	//mq_unlink(pMq->szMqName);
	if (mq_unlink(pMq->szMqName) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSMQ(("DeleteMq error (%s) : %s"
			"(pMq=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMq));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Post a message in a mq.

MQ* pMq : (INOUT) Valid pointer to a structure corresponding to 
a mq.
void* msg : (IN) Valid pointer to the message data (make sure this pointer 
is still valid at time of reception, you are responsible of the memory allocation 
of the message data).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
__inline int PostMq(MQ* pMq, void* msg)
{
#ifdef _WIN32
	if (!PostThreadMessage( 
		*pMq, // Identifier of the message queue (i.e the thread) to which the message is to be posted.
		(UINT)WM_USER, // Indicates that it is a custom message (i.e. not a system predefined message).
		(WPARAM)msg, // The message data.
		(LPARAM)NULL // Not used.
		))	
	{
		PRINT_DEBUG_ERROR_OSMQ(("PostMq error (%s) : %s"
			"(pMq=%#x, msg=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMq, msg));
		return EXIT_FAILURE;
	}
#else 
	if (mq_send(pMq->mq, (char*)&msg, MAX_MSG_SIZE, 0) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSMQ(("PostMq error (%s) : %s"
			"(pMq=%#x, msg=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMq, msg));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Get a message from a mq or block until a message is available or 
a timeout elapses.
The behaviour is undefined if 2 threads are using this function 
in the same time for the same message queue.

MQ* pMq : (INOUT) Valid pointer to a structure corresponding to 
a mq.
void** pMsg : (INOUT) Valid pointer that will receive the pointer to the message data 
posted with PostMq().
int timeout : (IN) Timeout in ms (max is MAX_MQ_TIMEOUT).

Return : EXIT_SUCCESS if a message has been retrieved, 
EXIT_TIMEOUT if no message is available after the timeout elapses or EXIT_FAILURE if
there is an error.
*/
__inline int WaitAndGetMq(MQ* pMq, void** pMsg, int timeout)//PendMq
{
#ifdef _WIN32
	MSG WIN32msg;
	*pMsg = NULL;

	// It does not seem to be possible to successfully use GetMessage() to get 
	// a message from the message queue of another thread. We just can post
	// a message to the message queue of another thread. Therefore, to get a message
	// from the thread associated with the message queue, we first send a message
	// to this thread with the current thread identifier (i.e the identifier of the thread 
	// that calls phlteOs_MessageQueue_get()) as data. This will notify the thread to
	// call GetMessage() to examine its message queue and get the real message that we
	// want. This thread will then call PostThreadMessage() (using our thread identifier,
	// that we have sent before) to send us the real message.
	// 2 types of messages are used : WM_USER+1 and WM_USER. WM_USER is the type used for the real
	// message, WM_USER+1 is the type of message used to solve the problem with GetMessage().

	if (!PostThreadMessage( 
		*pMq, // Identifier of the message queue (i.e the WIN32 thread) to which the message is to be posted.
		(UINT)WM_USER+1, // Indicates that it is a custom message (i.e. not a system predefined message).
		(WPARAM)GetCurrentThreadId(), // This thread identifier as first part of the message data.
		(LPARAM)timeout // The timeout as second part of the message data.
		))	
	{
		PRINT_DEBUG_ERROR_OSMQ(("WaitAndGetMq error (%s) : %s"
			"(pMq=%#x, timeout=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMq, timeout));
		return EXIT_FAILURE;
	}

	if (!GetMessage( 
		&WIN32msg, // The message.
		(HWND)NULL, // Indicates that the message to retrieve has been posted with PostThreadMessage().
		WM_USER+1, // Lowest message value to be retrieved. In this case, it is a custom message.
		WM_USER+1 // Highest message value to be retrieved.
		))	
	{
		PRINT_DEBUG_ERROR_OSMQ(("WaitAndGetMq error (%s) : %s"
			"(pMq=%#x, timeout=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMq, timeout));
		return EXIT_FAILURE;
	}

	switch (WIN32msg.lParam)	 
	{
	case EXIT_SUCCESS: 
		*pMsg = (void*)(WIN32msg.wParam); // Retrieve the message data.
		break; 
	case EXIT_TIMEOUT: 
		return EXIT_TIMEOUT;
	default:
		PRINT_DEBUG_ERROR_OSMQ(("WaitAndGetMq error (%s) : %s"
			"(pMq=%#x, timeout=%d)\n", 
			strtime_m(), 
			"Timeout or error. ", 
			pMq, timeout));
		return EXIT_FAILURE;
	}
#else 
	int nbbytes = 0;
	struct timespec abstime;
	*pMsg = NULL;

	// Calculate relative interval as current time plus duration given by timeout.

	if (clock_gettime(CLOCK_REALTIME, &abstime) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSMQ(("WaitAndGetMq error (%s) : %s"
			"(pMq=%#x, timeout=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMq, timeout));
		return EXIT_FAILURE;
	}

	abstime.tv_sec += timeout/1000; // Seconds.
	abstime.tv_nsec += (timeout%1000)*1000000; // Additional nanoseconds.
	abstime.tv_sec += abstime.tv_nsec/1000000000;
	abstime.tv_nsec = abstime.tv_nsec%1000000000;

	// Does not work, need to solve allocation of the message buffer...

	nbbytes = mq_timedreceive(pMq->mq, (char*)pMsg, MAX_MSG_SIZE, 0, &abstime);
	if (nbbytes == -1)
	{
		switch (errno)	 
		{
		case ETIMEDOUT: 
			return EXIT_TIMEOUT;
		default:
			PRINT_DEBUG_ERROR_OSMQ(("WaitAndGetMq error (%s) : %s"
				"(pMq=%#x, timeout=%d)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pMq, timeout));
			return EXIT_FAILURE;
		}
	}
	else if (nbbytes != MAX_MSG_SIZE)
	{
		PRINT_DEBUG_ERROR_OSMQ(("WaitAndGetMq error (%s) : %s"
			"(pMq=%#x, timeout=%d)\n", 
			strtime_m(), 
			"Invalid message size. ", 
			pMq, timeout));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

#endif // OSMQ_H
