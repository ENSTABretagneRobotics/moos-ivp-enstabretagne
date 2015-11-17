/***************************************************************************************************************:')

OSIPMutex.h

Interprocess mutex handling.

Fabrice Le Bars

Created : 2011-08-14

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

#ifndef OSIPMUTEX_H
#define OSIPMUTEX_H

#include "OSThread.h"

/*
Debug macros specific to OSIPMutex.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSIPMUTEX
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSIPMUTEX
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSIPMUTEX
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSIPMUTEX
#	define PRINT_DEBUG_MESSAGE_OSIPMUTEX(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSIPMUTEX(params)
#endif // _DEBUG_MESSAGES_OSIPMUTEX

#ifdef _DEBUG_WARNINGS_OSIPMUTEX
#	define PRINT_DEBUG_WARNING_OSIPMUTEX(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSIPMUTEX(params)
#endif // _DEBUG_WARNINGS_OSIPMUTEX

#ifdef _DEBUG_ERRORS_OSIPMUTEX
#	define PRINT_DEBUG_ERROR_OSIPMUTEX(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSIPMUTEX(params)
#endif // _DEBUG_ERRORS_OSIPMUTEX

#ifdef _WIN32
#else 
#include <sys/mman.h>
#include <sys/stat.h> // For mode constants.
#include <fcntl.h> // For O_* constants.
#include <semaphore.h>
#endif // _WIN32

#define MAX_IPMUTEX_TIMEOUT (LONG_MAX-2)
#define MAX_IPMUTEX_NAME_LENGTH (128-8)

#ifdef _WIN32
typedef HANDLE IPMUTEX;
#else
struct IPMUTEX_SHARED_MEM_DATA
{
	pthread_mutex_t mutex; // Mutex stored in named interprocess shared memory.
	int nbRefs; // Reference counter to know if the mutex is used by other processes.
};
typedef struct IPMUTEX_SHARED_MEM_DATA IPMUTEX_SHARED_MEM_DATA;

struct IPMUTEX
{
	IPMUTEX_SHARED_MEM_DATA* pSharedData; // Pointer to the named interprocess shared memory data.
	char szMutexShmName[MAX_IPMUTEX_NAME_LENGTH+8]; // Name of the named interprocess shared memory, 
	// that will be considered as the name of the interprocess mutex.
	sem_t* pSemProtect; // Semaphore used to protect access to the reference counter.
	char szSemProtectName[MAX_IPMUTEX_NAME_LENGTH+8];
};
typedef struct IPMUTEX IPMUTEX;
#endif // _WIN32

/*
Create or open a named interprocess mutex. Use DeleteIPMutex() to 
delete it at the end.

IPMUTEX* pMutex : (INOUT) Valid pointer that will receive a structure 
corresponding to a mutex.
char* szName : (IN) Name of the mutex.

Return : EXIT_SUCCESS, EXIT_NAME_TOO_LONG or EXIT_FAILURE.
*/
inline int CreateOrOpenIPMutex(IPMUTEX* pMutex, char* szName)
{
#ifdef _WIN32
	HANDLE hMutex;
	char szMutexShmName[MAX_IPMUTEX_NAME_LENGTH+8];

	if (strlen(szName) > MAX_IPMUTEX_NAME_LENGTH)
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("CreateOrOpenIPMutex error (%s) : %s"
			"(szName=%s)\n", 
			strtime_m(), 
			szOSUtilsErrMsgs[EXIT_NAME_TOO_LONG], 
			szName));
		return EXIT_NAME_TOO_LONG;
	}

	// Prefixing the name with "Global\" allows processes to communicate with each 
	// other even if they are in different terminal server sessions (e.g. several 
	// remote desktop connections).
	sprintf(szMutexShmName, "Global\\%s", szName);

	hMutex = CreateMutex( 
		NULL, // Default security attributes.
		FALSE, // The calling thread does not obtain initial ownership of the mutex object.
		szMutexShmName); // Named mutex.

	if (hMutex == NULL) 
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("CreateOrOpenIPMutex error (%s) : %s"
			"(szName=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			szName));
		return EXIT_FAILURE;
	}

	*pMutex = hMutex;
#else 
	int shmfd = -1; // File descriptor of the named interprocess shared memory.
	pthread_mutexattr_t attr;

	if (strlen(szName) > MAX_IPMUTEX_NAME_LENGTH)
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("CreateOrOpenIPMutex error (%s) : %s"
			"(szName=%s)\n", 
			strtime_m(), 
			szOSUtilsErrMsgs[EXIT_NAME_TOO_LONG], 
			szName));
		return EXIT_NAME_TOO_LONG;
	}

	// szName must be like "/nameXXX".
	sprintf(pMutex->szSemProtectName, "/SP%s", szName);

	// Used as an interprocess protection semaphore.
	pMutex->pSemProtect = sem_open(pMutex->szSemProtectName, O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO, 1);

	if (pMutex->pSemProtect == SEM_FAILED)
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("CreateOrOpenIPMutex error (%s) : %s"
			"(szName=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			szName));
		return EXIT_FAILURE;
	}

	if (sem_wait(pMutex->pSemProtect) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("CreateOrOpenIPMutex error (%s) : %s"
			"(szName=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			szName));
		sem_close(pMutex->pSemProtect);
		sem_unlink(pMutex->szSemProtectName);
		return EXIT_FAILURE;
	}

	sprintf(pMutex->szMutexShmName, "/%s", szName);

	// Create and open a new, or open an existing, POSIX shared memory object.
	shmfd = shm_open(pMutex->szMutexShmName, O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);

	if (shmfd == -1)
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("CreateOrOpenIPMutex error (%s) : %s"
			"(szName=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			szName));
		sem_post(pMutex->pSemProtect);
		sem_close(pMutex->pSemProtect);
		sem_unlink(pMutex->szSemProtectName);
		return EXIT_FAILURE;
	}

	// Set the size of the shared memory space.
	if (ftruncate(shmfd, sizeof(IPMUTEX_SHARED_MEM_DATA)) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("CreateOrOpenIPMutex error (%s) : %s"
			"(szName=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			szName));
		close(shmfd);
		shm_unlink(pMutex->szMutexShmName);
		sem_post(pMutex->pSemProtect);
		sem_close(pMutex->pSemProtect);
		sem_unlink(pMutex->szSemProtectName);
		return EXIT_FAILURE;
	}

	// Map the shared memory space in the virtual address space of the calling process.
	pMutex->pSharedData = (IPMUTEX_SHARED_MEM_DATA*)mmap(NULL, sizeof(IPMUTEX_SHARED_MEM_DATA), 
		PROT_READ|PROT_WRITE, MAP_SHARED, shmfd, 0);

	if (pMutex->pSharedData == MAP_FAILED)
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("CreateOrOpenIPMutex error (%s) : %s"
			"(szName=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			szName));
		close(shmfd);
		shm_unlink(pMutex->szMutexShmName);
		sem_post(pMutex->pSemProtect);
		sem_close(pMutex->pSemProtect);
		sem_unlink(pMutex->szSemProtectName);
		return EXIT_FAILURE;
	}

	// The file descriptor of the named interprocess shared memory is no longer 
	// needed (closing it does not change the memory mapping).
	if (close(shmfd) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("CreateOrOpenIPMutex error (%s) : %s"
			"(szName=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			szName));
		munmap(pMutex->pSharedData, sizeof(IPMUTEX_SHARED_MEM_DATA));
		shm_unlink(pMutex->szMutexShmName);
		sem_post(pMutex->pSemProtect);
		sem_close(pMutex->pSemProtect);
		sem_unlink(pMutex->szSemProtectName);
		return EXIT_FAILURE;
	}

	pMutex->pSharedData->nbRefs++;

	if (pMutex->pSharedData->nbRefs <= 1)
	{
		// The mutex need to be initialized in the shared memory.

		// Initialize attributes that will be used to create the mutex.
		if (pthread_mutexattr_init(&attr) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSIPMUTEX(("CreateOrOpenIPMutex error (%s) : %s"
				"(szName=%s)\n", 
				strtime_m(), 
				"pthread_mutexattr_init failed. ", 
				szName));
			pMutex->pSharedData->nbRefs--;
			munmap(pMutex->pSharedData, sizeof(IPMUTEX_SHARED_MEM_DATA));
			shm_unlink(pMutex->szMutexShmName);
			sem_post(pMutex->pSemProtect);
			sem_close(pMutex->pSemProtect);
			sem_unlink(pMutex->szSemProtectName);
			return EXIT_FAILURE;
		}

		// A thread attempting to relock this mutex without first unlocking it shall succeed in 
		// locking the mutex. The relocking deadlock which can occur with mutex of type 
		// PTHREAD_MUTEX_NORMAL cannot occur with this type of mutex. Multiple locks of this mutex 
		// shall require the same number of unlocks to release the mutex before another thread can 
		// acquire the mutex. A thread attempting to unlock a mutex which another thread has locked 
		// shall return with an error. A thread attempting to unlock an unlocked mutex shall return 
		// with an error. 
		// This is similar to the behaviour of the Windows mutex objects.
		if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSIPMUTEX(("CreateOrOpenIPMutex error (%s) : %s"
				"(szName=%s)\n", 
				strtime_m(), 
				"pthread_mutexattr_settype failed. ", 
				szName));
			pthread_mutexattr_destroy(&attr);
			pMutex->pSharedData->nbRefs--;
			munmap(pMutex->pSharedData, sizeof(IPMUTEX_SHARED_MEM_DATA));
			shm_unlink(pMutex->szMutexShmName);
			sem_post(pMutex->pSemProtect);
			sem_close(pMutex->pSemProtect);
			sem_unlink(pMutex->szSemProtectName);
			return EXIT_FAILURE;
		}

		// The process-shared attribute is set to PTHREAD_PROCESS_SHARED to permit a mutex to be operated
		// upon by any thread that has access to the memory where the mutex is allocated, even if the mutex 
		// is allocated in memory that is shared by multiple processes.
		if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSIPMUTEX(("CreateOrOpenIPMutex error (%s) : %s"
				"(szName=%s)\n", 
				strtime_m(), 
				"pthread_mutexattr_setpshared failed. ", 
				szName));
			pthread_mutexattr_destroy(&attr);
			pMutex->pSharedData->nbRefs--;
			munmap(pMutex->pSharedData, sizeof(IPMUTEX_SHARED_MEM_DATA));
			shm_unlink(pMutex->szMutexShmName);
			sem_post(pMutex->pSemProtect);
			sem_close(pMutex->pSemProtect);
			sem_unlink(pMutex->szSemProtectName);
			return EXIT_FAILURE;
		}

		// Crezte the mutex object and store it in the shared memory.
		if (pthread_mutex_init(&pMutex->pSharedData->mutex, &attr) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSIPMUTEX(("CreateOrOpenIPMutex error (%s) : %s"
				"(szName=%s)\n", 
				strtime_m(), 
				"pthread_mutex_init failed. ", 
				szName));
			pthread_mutexattr_destroy(&attr);
			pMutex->pSharedData->nbRefs--;
			munmap(pMutex->pSharedData, sizeof(IPMUTEX_SHARED_MEM_DATA));
			shm_unlink(pMutex->szMutexShmName);
			sem_post(pMutex->pSemProtect);
			sem_close(pMutex->pSemProtect);
			sem_unlink(pMutex->szSemProtectName);
			return EXIT_FAILURE;
		}

		if (pthread_mutexattr_destroy(&attr) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSIPMUTEX(("CreateOrOpenIPMutex error (%s) : %s"
				"(szName=%s)\n", 
				strtime_m(), 
				"pthread_mutexattr_destroy failed. ", 
				szName));
			pthread_mutex_destroy(&pMutex->pSharedData->mutex);
			pMutex->pSharedData->nbRefs--;
			munmap(pMutex->pSharedData, sizeof(IPMUTEX_SHARED_MEM_DATA));
			shm_unlink(pMutex->szMutexShmName);
			sem_post(pMutex->pSemProtect);
			sem_close(pMutex->pSemProtect);
			sem_unlink(pMutex->szSemProtectName);
			return EXIT_FAILURE;
		}
	}

	if (sem_post(pMutex->pSemProtect) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("CreateOrOpenIPMutex error (%s) : %s"
			"(szName=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			szName));
		pthread_mutex_destroy(&pMutex->pSharedData->mutex);
		pMutex->pSharedData->nbRefs--;
		munmap(pMutex->pSharedData, sizeof(IPMUTEX_SHARED_MEM_DATA));
		shm_unlink(pMutex->szMutexShmName);
		sem_close(pMutex->pSemProtect);
		sem_unlink(pMutex->szSemProtectName);
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Delete an interprocess mutex created by CreateOrOpenIPMutex().

IPMUTEX* pMutex : (INOUT) Valid pointer to a structure corresponding to 
a mutex.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int DeleteIPMutex(IPMUTEX* pMutex)
{
#ifdef _WIN32
	if (!CloseHandle(*pMutex))
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("DeleteIPMutex error (%s) : %s"
			"(pMutex=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMutex));
		return EXIT_FAILURE;
	}
#else
	if (sem_wait(pMutex->pSemProtect) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("DeleteIPMutex error (%s) : %s"
			"(pMutex=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMutex));
		return EXIT_FAILURE;
	}

	pMutex->pSharedData->nbRefs--;

	if (pMutex->pSharedData->nbRefs < 1)
	{
		// Everything must be destroyed.

		if (pthread_mutex_destroy(&pMutex->pSharedData->mutex) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSIPMUTEX(("DeleteIPMutex error (%s) : %s"
				"(pMutex=%#x)\n", 
				strtime_m(), 
				"pthread_mutex_destroy failed. ", 
				pMutex));
			sem_post(pMutex->pSemProtect);
			return EXIT_FAILURE;
		}

		// Remove a shared memory object name, and, once all processes have unmapped the 
		// object, deallocate and destroy the contents of the associated memory region.
		if (shm_unlink(pMutex->szMutexShmName) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSIPMUTEX(("DeleteIPMutex error (%s) : %s"
				"(pMutex=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pMutex));
			sem_post(pMutex->pSemProtect);
			return EXIT_FAILURE;
		}

		// The semaphore name is removed immediately. The semaphore is destroyed
		// once all other processes that have the semaphore open close it.
		if (sem_unlink(pMutex->szSemProtectName) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSIPMUTEX(("DeleteIPMutex error (%s) : %s"
				"(pMutex=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pMutex));
			sem_post(pMutex->pSemProtect);
			return EXIT_FAILURE;
		}
	}

	if (munmap(pMutex->pSharedData, sizeof(IPMUTEX_SHARED_MEM_DATA)) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("DeleteIPMutex error (%s) : %s"
			"(pMutex=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMutex));
		sem_post(pMutex->pSemProtect);
		return EXIT_FAILURE;
	}

	if (sem_post(pMutex->pSemProtect) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("DeleteIPMutex error (%s) : %s"
			"(pMutex=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMutex));
		return EXIT_FAILURE;
	}

	if (sem_close(pMutex->pSemProtect) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("DeleteIPMutex error (%s) : %s"
			"(pMutex=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMutex));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Release ownership of the specified mutex object.
After a thread has ownership of a mutex, it can make additional calls to WaitAndGetIPMutex()
or TryGetIPMutex() without blocking its execution, but it must call ReleaseIPMutex() once 
for each time that it took ownership of the mutex. A thread that currently do not have 
ownership of the mutex should not call ReleaseIPMutex().

IPMUTEX* pMutex : (INOUT) Valid pointer to a structure corresponding to a mutex.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int ReleaseIPMutex(IPMUTEX* pMutex)
{
#ifdef _WIN32
	if (!ReleaseMutex(*pMutex))	
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("ReleaseIPMutex error (%s) : %s"
			"(pMutex=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMutex));
		return EXIT_FAILURE;
	}
#else 
	if (pthread_mutex_unlock(&pMutex->pSharedData->mutex) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("ReleaseIPMutex error (%s) : %s"
			"(pMutex=%#x)\n", 
			strtime_m(), 
			"pthread_mutex_unlock failed. ", 
			pMutex));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Attempt to take ownership of a mutex without blocking. If the call is successful, the 
calling thread takes ownership of the mutex.

IPMUTEX* pMutex : (INOUT) Valid pointer to a structure corresponding to 
a mutex.

Return : EXIT_SUCCESS if the mutex is successfully taken, EXIT_OBJECT_NONSIGNALED if it 
is not available or EXIT_FAILURE if there is an error.
*/
inline int TryAndGetIPMutex(IPMUTEX* pMutex)
{
#ifdef _WIN32
	DWORD dwWaitResult = WAIT_FAILED; 

	// Return when the specified object is in the signaled state or the time-out interval elapses.
	// In this case, we just check if the mutex is available.
	dwWaitResult = WaitForSingleObject(
		*pMutex, // Handle to the mutex.
		(DWORD)0); // Time-out interval (ms).

	switch (dwWaitResult)	 
	{
	case WAIT_OBJECT_0: 
		// The mutex object was signaled.
		break; 
	case WAIT_TIMEOUT: 
		// The mutex was nonsignaled, so a time-out occurred.
		return EXIT_OBJECT_NONSIGNALED; 
	default:
		PRINT_DEBUG_ERROR_OSIPMUTEX(("TryAndGetIPMutex error (%s) : %s"
			"(pMutex=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMutex));
		return EXIT_FAILURE;
	}
#else 
	int iResult = pthread_mutex_trylock(&pMutex->pSharedData->mutex);
	switch (iResult)	 
	{
	case EXIT_SUCCESS: 
		break;
	case EBUSY: 
		return EXIT_OBJECT_NONSIGNALED;
	default:
		PRINT_DEBUG_ERROR_OSIPMUTEX(("TryAndGetIPMutex error (%s) : %s"
			"(pMutex=%#x)\n", 
			strtime_m(), 
			"pthread_mutex_trylock failed. ", 
			pMutex));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Wait for ownership of the specified mutex object. The function returns when the 
calling thread is granted ownership or a timeout elapses.

IPMUTEX* pMutex : (INOUT) Valid pointer to a structure corresponding to 
a mutex.
int timeout : (IN) Timeout in ms (max is MAX_IPMUTEX_TIMEOUT).

Return : EXIT_SUCCESS if the mutex is successfully taken, EXIT_TIMEOUT if it is not 
available after the timeout elapses or EXIT_FAILURE if there is an error.
*/
inline int WaitAndGetIPMutex(IPMUTEX* pMutex, int timeout)
{
#ifdef _WIN32
	DWORD dwWaitResult = WAIT_FAILED; 

	// Returns when the specified object is in the signaled state or the time-out interval elapses.
	dwWaitResult = WaitForSingleObject(
		*pMutex, // Handle to the mutex.
		(DWORD)timeout); // Time-out interval (ms).

	switch (dwWaitResult)	 
	{
	case WAIT_OBJECT_0: 
		// The mutex object was signaled.
		break; 
	case WAIT_TIMEOUT: 
		// The mutex was nonsignaled, so a time-out occurred.
		return EXIT_TIMEOUT; 
	default:
		PRINT_DEBUG_ERROR_OSIPMUTEX(("WaitAndGetIPMutex error (%s) : %s"
			"(pMutex=%#x, timeout=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMutex, timeout));
		return EXIT_FAILURE;
	}
#else 
	int iResult = EXIT_FAILURE;
	struct timespec abstime;

	// Calculate relative interval as current time plus duration given by timeout.

	if (clock_gettime(CLOCK_REALTIME, &abstime) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSIPMUTEX(("WaitAndGetIPMutex error (%s) : %s"
			"(pMutex=%#x, timeout=%d)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			pMutex, timeout));
		return EXIT_FAILURE;
	}

	abstime.tv_sec += timeout/1000; // Seconds.
	abstime.tv_nsec += (timeout%1000)*1000000; // Additional nanoseconds.
	abstime.tv_sec += abstime.tv_nsec/1000000000;
	abstime.tv_nsec = abstime.tv_nsec%1000000000;

	iResult = pthread_mutex_timedlock(&pMutex->pSharedData->mutex, &abstime);
	switch (iResult)	 
	{
	case EXIT_SUCCESS: 
		break;
	case ETIMEDOUT: 
		return EXIT_TIMEOUT;
	default:
		PRINT_DEBUG_ERROR_OSIPMUTEX(("WaitAndGetIPMutex error (%s) : %s"
			"(pMutex=%#x, timeout=%d)\n", 
			strtime_m(), 
			"pthread_mutex_timedlock failed. ", 
			pMutex, timeout));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

#endif // OSIPMUTEX_H
