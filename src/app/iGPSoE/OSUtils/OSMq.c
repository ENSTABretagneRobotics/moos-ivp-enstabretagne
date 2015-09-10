/***************************************************************************************************************:')

OSMq.c

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

#include "OSMq.h"

#ifdef _WIN32
THREAD_PROC_RETURN_VALUE MqThrProc(LPVOID lpParameter)	
{
	MSG WIN32msg;
	DWORD thrId = 0;
	DWORD timeout = 0;
	DWORD dwWaitResult = WAIT_FAILED; 
	int status = EXIT_FAILURE;

	// This is just to force the creation of the message queue associated with the
	// thread. When it is ready, an event is signaled. Therefore, we are sure that
	// subsequent call to PostThreadMessage() to post a message in the
	// message queue associated with this thread, will not fail just because there is no 
	// message queue.
	PeekMessage(&WIN32msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	if (!SetEvent((HANDLE)lpParameter))	
	{
		PRINT_DEBUG_ERROR_OSMQ(("MqThrProc error (%s) : %s"
			"(lpParameter=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			lpParameter));
	}

	// Loop that ends when a specific message of type WM_USER+1 with data of 0 is sent.
	for (;;)	
	{
		// Wait for a request to check this thread message queue.
		// This request is a message of type WM_USER+1 with the sender thread identifier as data.
		// If the data is 0, it is a request to stop the thread.
		if (!GetMessage( 
			&WIN32msg, // The message.
			(HWND)NULL, // Indicate that the message to retrieve has been posted with PostThreadMessage().
			WM_USER+1, // Lowest message value to be retrieved. In this case, it is a custom message.
			WM_USER+1 // Highest message value to be retrieved.
			))	
		{
			PRINT_DEBUG_ERROR_OSMQ(("MqThrProc error (%s) : %s"
				"(lpParameter=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				lpParameter));
		}

		// The first part of the message data is the thread identifier of the thread which wants to get 
		// the message from this message queue.
		// If it is 0 it is a request to stop.
		thrId = (DWORD)(WIN32msg.wParam);
		if (thrId == 0)	
		{
			break;
		}
		// The second part of the message data is the timeout.
		timeout = (DWORD)(WIN32msg.lParam);

		// Wait for a posted message in this thread's message queue.
		dwWaitResult = MsgWaitForMultipleObjectsEx(
			1, // Number of object handles to check. In this case, it is only the current thread.
			NULL, // Indicates that it is only the current thread that has to be checked.
			(DWORD)timeout, // Time-out interval, in ms.
			QS_POSTMESSAGE, // Indicates to wait for posted messages in the thread's message queue.
			MWMO_INPUTAVAILABLE // Indicates that it returns if input exists for the queue, even if the input has been seen (but not removed) using a call to another function, such as PeekMessage().
			);

		switch (dwWaitResult)	 
		{
		case WAIT_OBJECT_0: 
		case WAIT_OBJECT_0+1: 
			// Retrieve the message that caused MsgWaitForMultipleObjectsEx() to return.
			ZeroMemory(&WIN32msg, sizeof(MSG));
#if _WIN32_WINNT>=0x0500
			if (!PeekMessage( 
				&WIN32msg, // The message.
				(HWND)NULL, // Indicates that the message to retrieve has been posted with PostThreadMessage().
				WM_USER, // Lowest message value to be retrieved. In this case, it is a custom message.
				WM_USER, // Highest message value to be retrieved.
				PM_REMOVE | PM_QS_POSTMESSAGE // Posted messages are removed from the queue.
				))	
#else
			if (!PeekMessage( 
				&WIN32msg, // The message
				(HWND)NULL, // Indicates that the message to retrieve has been posted with PostThreadMessage()
				WM_USER, // Lowest message value to be retrieved. In this case, it is a custom message.
				WM_USER, // Highest message value to be retrieved.
				PM_REMOVE // PM_QS_POSTMESSAGE is not supported under earlier versions.
				))	
#endif // _WIN32_WINNT>=0x0500
			{
				PRINT_DEBUG_ERROR_OSMQ(("MqThrProc error (%s) : %s"
					"(lpParameter=%#x)\n", 
					strtime_m(), 
					"There was an invalid message in the message queue specified. ", 
					lpParameter));
				status = EXIT_FAILURE;
				break;
			}		
			status = EXIT_SUCCESS;
			break; 
		case WAIT_TIMEOUT: 
			status = EXIT_TIMEOUT;
			break; 
		default:
			PRINT_DEBUG_ERROR_OSMQ(("MqThrProc error (%s) : %s"
				"(lpParameter=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				lpParameter));
			status = EXIT_FAILURE;
			break;
		}

		// We have no control on the target thread, we only know its identifier.
		// Therefore, this function could fail if the target thread does not have 
		// a message queue. However, in this case, it should not fail as the target
		// thread should have created its message queue because it has sent
		// to this thread a message, and therefore made a call to PostThreadMessage()
		// that forces the creation of its message queue if it did not have already created it.
		if (!PostThreadMessage( 
			(DWORD)thrId, // Identifier of the message queue (i.e the WIN32 thread) to which the message is to be posted.
			(UINT)WM_USER+1, // Indicates that it is a custom message.
			(WPARAM)WIN32msg.wParam, // The message data.
			(LPARAM)status // Used to indicate the result of the wait.
			))	
		{
			PRINT_DEBUG_ERROR_OSMQ(("MqThrProc error (%s) : %s"
				"(lpParameter=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				lpParameter));
		}
	}

	return 0;
}
#endif // _WIN32
