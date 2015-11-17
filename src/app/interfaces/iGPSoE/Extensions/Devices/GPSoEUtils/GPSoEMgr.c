/***************************************************************************************************************:')

GPSoEMgr.c

GPS over Ethernet handling.

Fabrice Le Bars

Created: 2012-05-15

Version status: Not finished

***************************************************************************************************************:)*/

#include "GPSoEMgr.h"

THREAD_PROC_RETURN_VALUE GPSoEMgrThreadProc(void* lpParam)	
{
	HGPSOEMGR hGPSoEMgr = (HGPSOEMGR)lpParam;
	double utc = 0;
	double latitude = 0;
	double longitude = 0;

	do
	{
		mSleep(hGPSoEMgr->PollingTimeInterval);

		if (!hGPSoEMgr->bConnected)
		{ 
			// The device is not connected, trying to connect or reconnect.
			ConnectToGPSoE(hGPSoEMgr);
		}
		else	
		{
			// Wait for new data.
			if (hGPSoEMgr->PollingMode & DATA_POLLING_MODE_GPSOE)
			{
				EnterCriticalSection(&hGPSoEMgr->GPSoECS);

				if (!hGPSoEMgr->bConnected)
				{ 
					LeaveCriticalSection(&hGPSoEMgr->GPSoECS);
					continue;
				}

				if (GetDataGPSoE(hGPSoEMgr->hGPSoE, &utc, &latitude, &longitude) != EXIT_SUCCESS)
				{
					PRINT_DEBUG_MESSAGE_GPSOEMGR(("Connection to a GPS over Ethernet lost\n"));

					// The device was supposed to be connected before.
					// Notify the thread to try to reconnect the device.
					hGPSoEMgr->bConnected = FALSE; 
					CloseGPSoE(&hGPSoEMgr->hGPSoE);

					LeaveCriticalSection(&hGPSoEMgr->GPSoECS);
					continue;
				}

				LeaveCriticalSection(&hGPSoEMgr->GPSoECS);

				EnterCriticalSection(&hGPSoEMgr->dataCS);
				hGPSoEMgr->datacounter++;
				hGPSoEMgr->utc = utc;
				hGPSoEMgr->latitude = latitude;
				hGPSoEMgr->longitude = longitude;
				LeaveCriticalSection(&hGPSoEMgr->dataCS);
			}
		}
	} 
	while (hGPSoEMgr->bRunThread);

	EnterCriticalSection(&hGPSoEMgr->GPSoECS);

	if (hGPSoEMgr->bConnected)
	{ 
		hGPSoEMgr->bConnected = FALSE; 

		if (CloseGPSoE(&hGPSoEMgr->hGPSoE) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_GPSOEMGR(("GPSoEMgrThreadProc error (%s) : %s"
				"\n", 
				strtime_m(), 
				". "));
		}
	}
	
	LeaveCriticalSection(&hGPSoEMgr->GPSoECS);

	return 0; 
} 
