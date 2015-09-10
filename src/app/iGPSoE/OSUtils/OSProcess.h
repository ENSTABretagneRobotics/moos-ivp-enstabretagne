/***************************************************************************************************************:')

OSProcess.c

Processes handling.

Fabrice Le Bars

Created : 2010-05-24

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

#ifndef OSPROCESS_H
#define OSPROCESS_H

#include "OSTime.h"

/*
Debug macros specific to OSProcess.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSPROCESS
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSPROCESS
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSPROCESS
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSPROCESS
#	define PRINT_DEBUG_MESSAGE_OSPROCESS(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSPROCESS(params)
#endif // _DEBUG_MESSAGES_OSPROCESS

#ifdef _DEBUG_WARNINGS_OSPROCESS
#	define PRINT_DEBUG_WARNING_OSPROCESS(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSPROCESS(params)
#endif // _DEBUG_WARNINGS_OSPROCESS

#ifdef _DEBUG_ERRORS_OSPROCESS
#	define PRINT_DEBUG_ERROR_OSPROCESS(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSPROCESS(params)
#endif // _DEBUG_ERRORS_OSPROCESS

#ifdef _WIN32
#else 
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <signal.h>
#endif // _WIN32

#ifdef _WIN32
typedef DWORD PROCESS_IDENTIFIER;
#else
typedef pid_t PROCESS_IDENTIFIER;
#endif // _WIN32

#ifndef _WIN32
/*
Retrieve the process identifier of the calling process.

Return : The process identifier.
*/
inline PROCESS_IDENTIFIER GetCurrentProcessId(void)
{
	return getpid();
}
#endif // _WIN32

//#ifndef _WIN32
//EXTERN_C void _ChildEndHandler(int sig);
//#endif // _WIN32

/*
Create a process with default attributes.

char* szCommandLine : (IN) Command line to execute.
PROCESS_IDENTIFIER* pProcessId : (INOUT) Valid pointer that will receive the 
process identifier.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int CreateDefaultProcess(char* szCommandLine, PROCESS_IDENTIFIER* pProcessId)
{
#ifdef _WIN32
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process. 
	if (!CreateProcess(NULL,   // No module name (use command line)
		szCommandLine,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi            // Pointer to PROCESS_INFORMATION structure
		)) 
	{
		PRINT_DEBUG_ERROR_OSPROCESS(("CreateDefaultProcess error (%s) : %s"
			"(szCommandLine=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			szCommandLine));
		return EXIT_FAILURE;
	}

	*pProcessId = pi.dwProcessId;

	// Close the process and primary thread handles (this does not terminate the process and makes the handle 
	// returned by CreateProcess() be closed automatically and invalid when the process terminates,
	// if no other handles to it are opened. Anyway, all the resources used by the process will
	// be released when it is terminated and all the handles to it are closed).
	if (!CloseHandle(pi.hProcess))
	{
		PRINT_DEBUG_ERROR_OSPROCESS(("CreateDefaultProcess error (%s) : %s"
			"(szCommandLine=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			szCommandLine));
		return EXIT_FAILURE;
	}
	if (!CloseHandle(pi.hThread))
	{
		PRINT_DEBUG_ERROR_OSPROCESS(("CreateDefaultProcess error (%s) : %s"
			"(szCommandLine=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			szCommandLine));
		return EXIT_FAILURE;
	}
#else
	struct sigaction act;

	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;//SA_RESTART|SA_RESETHAND|SA_NOCLDWAIT;
	act.sa_handler = SIG_IGN;//_ChildEndHandler;

	// To prevent the child to be in a zombie state when it exits normally.
	// SIGCHLD is fired when it goes in a zombie state.
	// Equivalent to make a thread detached...
	if (sigaction(SIGCHLD, &act, NULL) == -1)
	{
		PRINT_DEBUG_ERROR_OSPROCESS(("CreateDefaultProcess error (%s) : %s"
			"(szCommandLine=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			szCommandLine));
		*pProcessId = 0;
		return EXIT_FAILURE;
	}

	//PRINT_DEBUG_MESSAGE_OSPROCESS(("Trying to fork...\n"));

	*pProcessId = fork(); // Create child.

	if (*pProcessId == 0)		
	{
		char* new_argv[8];
		new_argv[0] = (char*)"sh";
		new_argv[1] = (char*)"-c";
		new_argv[2] = (char*)calloc(1024,1);
		sprintf(new_argv[2], "exec %s", szCommandLine);
		new_argv[3] = NULL;

		//PRINT_DEBUG_MESSAGE_OSPROCESS(("Child...\n"));

		if (execve("/bin/sh", (char *const *)new_argv, __environ) == -1)
		{	
			PRINT_DEBUG_ERROR_OSPROCESS(("CreateDefaultProcess error (%s) : %s"
				"(szCommandLine=%s)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				szCommandLine));
			*pProcessId = 0;
			free(new_argv[2]);
			return EXIT_FAILURE;
		}
		free(new_argv[2]);
		exit(127); // Like in the code of the system function...
	}
	else if (*pProcessId < 0)
	{	
		PRINT_DEBUG_ERROR_OSPROCESS(("CreateDefaultProcess error (%s) : %s"
			"(szCommandLine=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			szCommandLine));
		*pProcessId = 0;
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Wait until the process identified by ProcessId has terminated (should 
be a child process).

PROCESS_IDENTIFIER ProcessId : (IN) The process identifier.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int WaitForProcess(PROCESS_IDENTIFIER ProcessId)
{
#ifdef _WIN32
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, ProcessId);

	// Check the return value for success. 
	if (hProcess != NULL)
	{ 
		// The process is not already terminated.
		// Wait until the process has terminated.
		if (WaitForSingleObject(hProcess, INFINITE) == WAIT_FAILED)
		{
			PRINT_DEBUG_ERROR_OSPROCESS(("WaitForProcess error (%s) : %s"
				"(ProcessId=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				ProcessId));
			CloseHandle(hProcess);
			return EXIT_FAILURE;
		}
		// Close the process handle.
		if (!CloseHandle(hProcess))
		{
			PRINT_DEBUG_ERROR_OSPROCESS(("WaitForProcess error (%s) : %s"
				"(ProcessId=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				ProcessId));
			return EXIT_FAILURE;
		}
	}
#else 
	//struct sigaction act;

	//// Equivalent to make a thread joinable...
	//sigemptyset(&act.sa_mask);
	//act.sa_flags = 0;
	//act.sa_handler = 0;
	//if (sigaction(SIGCHLD, &act, NULL) == -1)
	//{
	//	PRINT_DEBUG_ERROR_OSPROCESS(("WaitForProcess error (%s) : %s"
	//		"(ProcessId=%#x)\n", 
	//		strtime_m(), 
	//		GetLastErrorMsg(), 
	//		ProcessId));
	//	return EXIT_FAILURE;
	//}

	// In fact it is considered as success if we do not find the process...
	
	//// To get error report if ProcessId is invalid.
	//if (waitpid(ProcessId, NULL, WNOHANG) == -1)
	//{
	//	PRINT_DEBUG_ERROR_OSPROCESS(("WaitForProcess error (%s) : %s"
	//		"(ProcessId=%#x)\n", 
	//		strtime_m(), 
	//		GetLastErrorMsg(), 
	//		ProcessId));
	//	return EXIT_FAILURE;
	//}

	if (waitpid(ProcessId, NULL, 0) == -1)
	{	
		if (errno != ECHILD)
		{
			PRINT_DEBUG_ERROR_OSPROCESS(("WaitForProcess error (%s) : %s"
				"(ProcessId=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				ProcessId));
			return EXIT_FAILURE;
		}
		//// Reset to detached...
		//sigemptyset(&act.sa_mask);
		//act.sa_flags = 0;
		//act.sa_handler = SIG_IGN;
		//sigaction(SIGCHLD, &act, NULL);
		//return EXIT_FAILURE;
	}

	//// Reset to detached...
	//sigemptyset(&act.sa_mask);
	//act.sa_flags = 0;
	//act.sa_handler = SIG_IGN;
	//if (sigaction(SIGCHLD, &act, NULL) == -1)
	//{
	//	PRINT_DEBUG_ERROR_OSPROCESS(("WaitForProcess error (%s) : %s"
	//		"(ProcessId=%#x)\n", 
	//		strtime_m(), 
	//		GetLastErrorMsg(), 
	//		ProcessId));
	//	return EXIT_FAILURE;
	//}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Terminate a process immediately. 

PROCESS_IDENTIFIER ProcessId : (IN) The process identifier.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int KillProcess(PROCESS_IDENTIFIER ProcessId)
{
#ifdef _WIN32

	// Should we also terminate the process main thread...?

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, ProcessId);

	if (hProcess == NULL)
	{
		PRINT_DEBUG_ERROR_OSPROCESS(("KillProcess error (%s) : %s"
			"(ProcessId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ProcessId));
		return EXIT_FAILURE;
	}

	if (!TerminateProcess(hProcess, EXIT_KILLED_PROCESS))
	{
		PRINT_DEBUG_ERROR_OSPROCESS(("KillProcess error (%s) : %s"
			"(ProcessId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ProcessId));
		CloseHandle(hProcess);
		return EXIT_FAILURE;
	}

	if (!CloseHandle(hProcess))
	{
		PRINT_DEBUG_ERROR_OSPROCESS(("KillProcess error (%s) : %s"
			"(ProcessId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ProcessId));
		return EXIT_FAILURE;
	}
#else 
	if (kill(ProcessId, SIGTERM) == -1)
	{
		PRINT_DEBUG_ERROR_OSPROCESS(("KillProcess error (%s) : %s"
			"(ProcessId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ProcessId));
		return EXIT_FAILURE;
	}

	//if (waitpid(ProcessId, NULL, WNOHANG) == -1)
	//{
	//	PRINT_DEBUG_ERROR_OSPROCESS(("KillProcess error (%s) : %s"
	//		"(ProcessId=%#x)\n", 
	//		strtime_m(), 
	//		GetLastErrorMsg(), 
	//		ProcessId));
	//	return EXIT_FAILURE;
	//}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Cause the calling process to yield execution to another process that is ready to 
run.

Return : Nothing.
*/
inline void ProcessYield(void)
{
#ifdef _WIN32
	SleepEx(0,0);
#else 
	sched_yield();
#endif // _WIN32
}

/*
Check if a process exists. 

PROCESS_IDENTIFIER ProcessId : (IN) The process identifier.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int CheckProcess(PROCESS_IDENTIFIER ProcessId)
{
#ifdef _WIN32
	DWORD dwExitCode = 0;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, ProcessId);

	if (hProcess == NULL)
	{
		PRINT_DEBUG_ERROR_OSPROCESS(("CheckProcess error (%s) : %s"
			"(ProcessId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ProcessId));
		return EXIT_FAILURE;
	}

	if (!GetExitCodeProcess(hProcess, &dwExitCode))
	{
		PRINT_DEBUG_ERROR_OSPROCESS(("CheckProcess error (%s) : %s"
			"(ProcessId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ProcessId));
		CloseHandle(hProcess);
		return EXIT_FAILURE;
	}

	if (!CloseHandle(hProcess))
	{
		PRINT_DEBUG_ERROR_OSPROCESS(("CheckProcess error (%s) : %s"
			"(ProcessId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ProcessId));
		return EXIT_FAILURE;
	}

	if (dwExitCode != STILL_ACTIVE)
	{
		PRINT_DEBUG_ERROR_OSPROCESS(("CheckProcess error (%s) : %s"
			"(ProcessId=%#x)\n", 
			strtime_m(), 
			"The process has already exited. ", 
			ProcessId));
		return EXIT_FAILURE;
	}
#else 
	if (kill(ProcessId, 0) == -1)
	{
		PRINT_DEBUG_ERROR_OSPROCESS(("CheckProcess error (%s) : %s"
			"(ProcessId=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			ProcessId));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}








#ifdef TEST

#ifdef _WIN32
/*s
truct PROCESS_IDENTIFIER
{
STARTUPINFO si;
PROCESS_INFORMATION pi;
};
typedef struct PROCESS_IDENTIFIER PROCESS_IDENTIFIER;
*/
#else
extern int REALTIME_PRIORITY_CLASS;
extern int HIGH_PRIORITY_CLASS;
extern int ABOVE_NORMAL_PRIORITY_CLASS;
extern int NORMAL_PRIORITY_CLASS;
extern int BELOW_NORMAL_PRIORITY_CLASS;
extern int IDLE_PRIORITY_CLASS;
/*
#define REALTIME_PRIORITY_CLASS 99
#define HIGH_PRIORITY_CLASS 70
#define ABOVE_NORMAL_PRIORITY_CLASS 60
#define NORMAL_PRIORITY_CLASS 50
#define BELOW_NORMAL_PRIORITY_CLASS 40
#define IDLE_PRIORITY_CLASS 0
*/
#endif // _WIN32

/*
Set the priority of the process identified by ProcessId. Priority should be one 
of the following constants : 
REALTIME_PRIORITY_CLASS, HIGH_PRIORITY_CLASS, ABOVE_NORMAL_PRIORITY_CLASS, 
NORMAL_PRIORITY_CLASS, BELOW_NORMAL_PRIORITY_CLASS, IDLE_PRIORITY_CLASS.
This function should be called when the process is not already terminated.

PROCESS_IDENTIFIER ProcessId : (IN) The process identifier.
int Priority : (IN) Integer representing the process priority.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SetProcessDefaultPriority(PROCESS_IDENTIFIER ProcessId, int Priority)
{
#ifdef _WIN32
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, ProcessId);

	// Checks the return value for success. 
	if (hProcess == NULL)
	{
		return EXIT_FAILURE;
	}

	// Set the process priority.
	if (!SetPriorityClass(hProcess, Priority))
	{
		CloseHandle(hProcess);
		return EXIT_FAILURE;
	}

	// Close the process handle (this does not terminate the process).
	if (!CloseHandle(hProcess))
	{
		return EXIT_FAILURE;
	}
#else 
	//setpriority, nice
	return EXIT_FAILURE;
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Get the priority of the process identified by ProcessId. *pPriority should be 
one of the following constants :
REALTIME_PRIORITY_CLASS, HIGH_PRIORITY_CLASS, ABOVE_NORMAL_PRIORITY_CLASS, 
NORMAL_PRIORITY_CLASS, BELOW_NORMAL_PRIORITY_CLASS, IDLE_PRIORITY_CLASS.
This function should be called when the process is not already terminated.

PROCESS_IDENTIFIER ProcessId : (IN) The process identifier.
int* pPriority : (INOUT) Valid pointer that will receive an integer 
representing the process priority.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int GetProcessDefaultPriority(PROCESS_IDENTIFIER ProcessId, int* pPriority)
{
#ifdef _WIN32
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, ProcessId);

	// Check the return value for success. 
	if (hProcess == NULL)
	{
		return EXIT_FAILURE;
	}

	// Get the process priority.
	*pPriority = GetPriorityClass(hProcess);
	if (*pPriority == 0)
	{
		CloseHandle(hProcess);
		return EXIT_FAILURE;
	}

	// Close the process handle (this does not terminate the process).
	if (!CloseHandle(hProcess))
	{
		return EXIT_FAILURE;
	}
#else 
	//getpriority, nice
	return EXIT_FAILURE;
#endif // _WIN32

	return EXIT_SUCCESS;
}

// TerminateProcess, kill

#ifdef USELESS
/*
Compare 2 process identifiers.

PROCESS_IDENTIFIER ProcessId1 : (IN) First process identifier.
PROCESS_IDENTIFIER ProcessId2 : (IN) Second process identifier.

Return : TRUE or FALSE.
*/
inline BOOL CompareProcessId(PROCESS_IDENTIFIER ProcessId1, PROCESS_IDENTIFIER ProcessId2)
{
	if (ProcessId1 == ProcessId2)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
#endif // USELESS

#endif // TEST

#endif // OSPROCESS_H
