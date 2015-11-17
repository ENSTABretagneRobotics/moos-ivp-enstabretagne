/***************************************************************************************************************:')

GPSoEMgr.h

GPS over Ethernet handling.

Fabrice Le Bars

Created: 2012-05-15

Version status: Not finished

***************************************************************************************************************:)*/

#ifndef GPSOEMGR_H
#define GPSOEMGR_H

#include "OSThread.h"
#include "OSCriticalSection.h"
#include "GPSoECfg.h"

/*
Debug macros specific to GPSoEMgr.
*/
#ifdef _DEBUG_MESSAGES_GPSOEUTILS
#	define _DEBUG_MESSAGES_GPSOEMGR
#endif // _DEBUG_MESSAGES_GPSOEUTILS

#ifdef _DEBUG_WARNINGS_GPSOEUTILS
#	define _DEBUG_WARNINGS_GPSOEMGR
#endif // _DEBUG_WARNINGS_GPSOEUTILS

#ifdef _DEBUG_ERRORS_GPSOEUTILS
#	define _DEBUG_ERRORS_GPSOEMGR
#endif // _DEBUG_ERRORS_GPSOEUTILS

#ifdef _DEBUG_MESSAGES_GPSOEMGR
#	define PRINT_DEBUG_MESSAGE_GPSOEMGR(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_GPSOEMGR(params)
#endif // _DEBUG_MESSAGES_GPSOEMGR

#ifdef _DEBUG_WARNINGS_GPSOEMGR
#	define PRINT_DEBUG_WARNING_GPSOEMGR(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_GPSOEMGR(params)
#endif // _DEBUG_WARNINGS_GPSOEMGR

#ifdef _DEBUG_ERRORS_GPSOEMGR
#	define PRINT_DEBUG_ERROR_GPSOEMGR(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_GPSOEMGR(params)
#endif // _DEBUG_ERRORS_GPSOEMGR

#define DATA_POLLING_MODE_GPSOE 0x00000001

struct GPSOEMGR
{
	HGPSOE hGPSoE;
	char szDevice[256];
	THREAD_IDENTIFIER ThreadId;
	int PollingTimeInterval;
	int PollingMode;
	BOOL bRunThread;
	BOOL bConnected;
	CRITICAL_SECTION GPSoECS;
	CRITICAL_SECTION dataCS;
	UINT datacounter;
	double utc;
	double latitude;
	double longitude;
	BOOL bWaitForFirstData;
};
typedef struct GPSOEMGR* HGPSOEMGR;

#define INVALID_HGPSOEMGR_VALUE NULL

/*
Private function.
Thread function that handles the polling and automatic reconnection of a GPS over Ethernet.

LPVOID lpParam : (INOUT) Parameter of the thread.

Return : 0.
*/
EXTERN_C THREAD_PROC_RETURN_VALUE GPSoEMgrThreadProc(void* lpParam);

/*
Private function.
Open a GPS over Ethernet and get data from it.

HES hGPSoE : (IN) Identifier of the GPS over Ethernet.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int ConnectToGPSoE(HGPSOEMGR hGPSoEMgr)
{
	// No critical section is needed here because the other functions 
	// should not try to access the device or data while the device is
	// not connected (bConnected = FALSE).

	if (OpenGPSoE(&hGPSoEMgr->hGPSoE, hGPSoEMgr->szDevice) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_MESSAGE_GPSOEMGR(("Unable to connect to a GPS over Ethernet\n"));
		return EXIT_FAILURE;
	}

	// Get data before setting bConnected to TRUE to be sure that
	// the other functions will get valid data.
	if (hGPSoEMgr->bWaitForFirstData)
	{
		if (hGPSoEMgr->PollingMode & DATA_POLLING_MODE_GPSOE)
		{
			if (GetDataGPSoE(hGPSoEMgr->hGPSoE, &hGPSoEMgr->utc, &hGPSoEMgr->latitude, &hGPSoEMgr->longitude) != EXIT_SUCCESS)
			{
				PRINT_DEBUG_MESSAGE_GPSOEMGR(("Unable to connect to a GPS over Ethernet\n"));
				return EXIT_FAILURE;
			}
		}
	}

	hGPSoEMgr->bConnected = TRUE;
	PRINT_DEBUG_MESSAGE_GPSOEMGR(("GPS over Ethernet connected\n"));

	return EXIT_SUCCESS;
}

/*
Initialize a structure that manage the polling and the reconnection of a GPS over Ethernet.

HGPSOEMGR* phGPSoEMgr : (INOUT) Valid pointer that will receive an identifier of
the structure.
char* szDevice : (IN) GPS over Ethernet serial port to open.
int PollingTimeInterval : (IN) Time interval between 2 requests of data (in ms).
int PollingMode : (IN) DATA_POLLING_MODE_GPSOE.
int Priority : (IN) Priority of the thread that manages the polling and the 
reconnection of the device. See SetThreadDefaultPriority().
BOOL bWaitForFirstData : (IN) TRUE to wait for the first entire data in 
InitGPSOEMgr(), FALSE if it does not matter if the first data after the 
connection to the device is invalid during some seconds.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int InitGPSoEMgr(HGPSOEMGR* phGPSoEMgr, char* szDevice, 
						 int PollingTimeInterval,
						 int PollingMode,
						 int Priority,
						 BOOL bWaitForFirstData)
{
	*phGPSoEMgr = (HGPSOEMGR)calloc(1, sizeof(struct GPSOEMGR));

	if (*phGPSoEMgr == NULL)
	{
		PRINT_DEBUG_ERROR_GPSOEMGR(("InitGPSoEMgr error (%s) : %s"
			"(szDevice=%s)\n", 
			strtime_m(), 
			"Out of memory. ", 
			szDevice));
		return EXIT_FAILURE;
	}

	sprintf((*phGPSoEMgr)->szDevice, "%s", szDevice);

	(*phGPSoEMgr)->PollingTimeInterval = PollingTimeInterval;
	(*phGPSoEMgr)->PollingMode = PollingMode;
	(*phGPSoEMgr)->bConnected = FALSE;
	(*phGPSoEMgr)->bRunThread = TRUE;
	(*phGPSoEMgr)->datacounter = 0;
	(*phGPSoEMgr)->utc = 0;
	(*phGPSoEMgr)->latitude = 0;
	(*phGPSoEMgr)->longitude = 0;
	(*phGPSoEMgr)->bWaitForFirstData = bWaitForFirstData;

	if (
		(InitCriticalSection(&(*phGPSoEMgr)->GPSoECS) != EXIT_SUCCESS)||
		(InitCriticalSection(&(*phGPSoEMgr)->dataCS) != EXIT_SUCCESS)
		)
	{
		PRINT_DEBUG_ERROR_GPSOEMGR(("InitGPSoEMgr error (%s) : %s"
			"(szDevice=%s)\n", 
			strtime_m(), 
			"Error creating a critical section. ", 
			szDevice));
		DeleteCriticalSection(&(*phGPSoEMgr)->dataCS);
		DeleteCriticalSection(&(*phGPSoEMgr)->GPSoECS);
		free(*phGPSoEMgr);
		return EXIT_FAILURE;
	}

	// First attempt to connect to the device.
	ConnectToGPSoE(*phGPSoEMgr);

	if (CreateDefaultThread(GPSoEMgrThreadProc, (void*)*phGPSoEMgr, &(*phGPSoEMgr)->ThreadId) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_GPSOEMGR(("InitGPSoEMgr error (%s) : %s"
			"(szDevice=%s)\n", 
			strtime_m(), 
			"Error creating a thread. ", 
			szDevice));
		if ((*phGPSoEMgr)->bConnected)
		{
			CloseGPSoE(&(*phGPSoEMgr)->hGPSoE);
		}
		DeleteCriticalSection(&(*phGPSoEMgr)->dataCS);
		DeleteCriticalSection(&(*phGPSoEMgr)->GPSoECS);
		free(*phGPSoEMgr);
		return EXIT_FAILURE;
	}

#ifdef ENABLE_PRIORITY_HANDLING
	if (SetThreadDefaultPriority((*phGPSoEMgr)->ThreadId, Priority) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_WARNING_GPSOEMGR(("InitGPSoEMgr error (%s) : %s"
			"(szDevice=%s)\n", 
			strtime_m(), 
			"Error setting the priority of a thread. ", 
			szDevice));
	}
#else
	UNREFERENCED_PARAMETER(Priority);
#endif // ENABLE_PRIORITY_HANDLING

	return EXIT_SUCCESS;
}

/*
Get the distance from a GPS over Ethernet (thread safe).
If PollingMode have the DATA_POLLING_MODE_GPSOE flag set, get the last data 
polled in the last PollingTimeInterval, otherwise a direct thread safe request 
is made to the device.

HGPSOEMGR hGPSoEMgr : (IN) Identifier of the structure that manages the polling 
and the reconnection of a GPS over Ethernet.
double* pUTC : (INOUT) Valid pointer receiving the UTC time.
double* pLatitude : (INOUT) Valid pointer receiving the latitude.
double* pLongitude : (INOUT) Valid pointer receiving the longitude.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int GetDataGPSoEMgr(HGPSOEMGR hGPSoEMgr, double* pUTC, double* pLatitude, double* pLongitude)
{
	if (hGPSoEMgr->PollingMode & DATA_POLLING_MODE_GPSOE)
	{
		EnterCriticalSection(&hGPSoEMgr->dataCS);

		if (!hGPSoEMgr->bConnected)
		{ 
			PRINT_DEBUG_ERROR_GPSOEMGR(("GetDataGPSoEMgr error (%s) : %s"
				"(hGPSoEMgr=%#x)\n", 
				strtime_m(), 
				"Device not currently connected. ", 
				hGPSoEMgr));

			LeaveCriticalSection(&hGPSoEMgr->dataCS);
			return EXIT_FAILURE;
		}

		// Get the last value stored.
		*pUTC = hGPSoEMgr->utc;
		*pLatitude = hGPSoEMgr->latitude;
		*pLongitude = hGPSoEMgr->longitude;

		LeaveCriticalSection(&hGPSoEMgr->dataCS);
	}
	else
	{
		EnterCriticalSection(&hGPSoEMgr->GPSoECS);

		if (!hGPSoEMgr->bConnected)
		{ 
			PRINT_DEBUG_ERROR_GPSOEMGR(("GetDataGPSoEMgr error (%s) : %s"
				"(hGPSoEMgr=%#x)\n", 
				strtime_m(), 
				"Device not currently connected. ", 
				hGPSoEMgr));

			LeaveCriticalSection(&hGPSoEMgr->GPSoECS);
			return EXIT_FAILURE;
		}

		// Get the value directly from the device.
		if (GetDataGPSoE(hGPSoEMgr->hGPSoE, pUTC, pLatitude, pLongitude) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_MESSAGE_GPSOEMGR(("Connection to a GPS over Ethernet lost\n"));
			PRINT_DEBUG_ERROR_GPSOEMGR(("GetDataGPSoEMgr error (%s) : %s"
				"(hGPSoEMgr=%#x)\n", 
				strtime_m(), 
				"Error getting data from the device. ", 
				hGPSoEMgr));

			// The device was supposed to be connected before.
			// Notify the thread to try to reconnect the device.
			hGPSoEMgr->bConnected = FALSE; 
			CloseGPSoE(&hGPSoEMgr->hGPSoE);

			LeaveCriticalSection(&hGPSoEMgr->GPSoECS);
			return EXIT_FAILURE;
		}
		
		LeaveCriticalSection(&hGPSoEMgr->GPSoECS);
	}

	return EXIT_SUCCESS;
}

/*
Release a structure that manage the polling and the reconnection of a GPS over Ethernet.

HGPSOEMGR* phGPSoEMgr : (INOUT) Valid pointer to the identifier of the structure  
that manages the polling and the reconnection of a GPS over Ethernet.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int ReleaseGPSoEMgr(HGPSOEMGR* phGPSoEMgr)
{
	BOOL bError = FALSE;

	// Indicate to the thread that it should terminate its loop.
	(*phGPSoEMgr)->bRunThread = FALSE;

	if (WaitForThread((*phGPSoEMgr)->ThreadId) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_GPSOEMGR(("ReleaseGPSoEMgr error (%s) : %s"
			"(*phGPSoEMgr=%#x)\n", 
			strtime_m(), 
			"Error waiting for the thread. ", 
			*phGPSoEMgr));
		bError = TRUE;
	}

	DeleteCriticalSection(&(*phGPSoEMgr)->dataCS);
	DeleteCriticalSection(&(*phGPSoEMgr)->GPSoECS);

	free(*phGPSoEMgr);

	*phGPSoEMgr = INVALID_HGPSOEMGR_VALUE;

	if (bError)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

#endif // GPSOEMGR_H
