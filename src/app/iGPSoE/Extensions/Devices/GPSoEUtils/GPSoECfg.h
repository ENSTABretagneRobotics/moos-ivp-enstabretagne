/***************************************************************************************************************:')

GPSoECfg.h

GPS over Ethernet handling.

Fabrice Le Bars

Created: 2012-05-15

Version status: Not finished

***************************************************************************************************************:)*/

#ifndef GPSOECFG_H
#define GPSOECFG_H

#include "GPSoECore.h"

/*
Debug macros specific to GPSoECfg.
*/
#ifdef _DEBUG_MESSAGES_GPSOEUTILS
#	define _DEBUG_MESSAGES_GPSOECFG
#endif // _DEBUG_MESSAGES_GPSOEUTILS

#ifdef _DEBUG_WARNINGS_GPSOEUTILS
#	define _DEBUG_WARNINGS_GPSOECFG
#endif // _DEBUG_WARNINGS_GPSOEUTILS

#ifdef _DEBUG_ERRORS_GPSOEUTILS
#	define _DEBUG_ERRORS_GPSOECFG
#endif // _DEBUG_ERRORS_GPSOEUTILS

#ifdef _DEBUG_MESSAGES_GPSOECFG
#	define PRINT_DEBUG_MESSAGE_GPSOECFG(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_GPSOECFG(params)
#endif // _DEBUG_MESSAGES_GPSOECFG

#ifdef _DEBUG_WARNINGS_GPSOECFG
#	define PRINT_DEBUG_WARNING_GPSOECFG(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_GPSOECFG(params)
#endif // _DEBUG_WARNINGS_GPSOECFG

#ifdef _DEBUG_ERRORS_GPSOECFG
#	define PRINT_DEBUG_ERROR_GPSOECFG(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_GPSOECFG(params)
#endif // _DEBUG_ERRORS_GPSOECFG

struct GPSOE
{
	SOCKET s;
	char address[MAX_BUF_LEN];
	char port[MAX_BUF_LEN];
};
typedef struct GPSOE* HGPSOE;

#define INVALID_HGPSOE_VALUE NULL

/*
Open a GPS over Ethernet.

HGPSOE* phGPSoE : (INOUT) Valid pointer that will receive an identifier of the
device opened.
char* szDevice : (IN) IP address:port.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int OpenGPSoE(HGPSOE* phGPSoE, char* szDevice)
{
	char* ptr = NULL;

	*phGPSoE = (HGPSOE)calloc(1, sizeof(struct GPSOE));

	if (*phGPSoE == NULL)
	{
		PRINT_DEBUG_ERROR_GPSOECFG(("OpenGPSoE error (%s) : %s"
			"(szDevice=%s)\n", 
			strtime_m(), 
			"Out of memory. ", 
			szDevice));
		return EXIT_FAILURE;
	}

	memset(&(*phGPSoE)->address, 0, sizeof(&(*phGPSoE)->address));
	memset(&(*phGPSoE)->port, 0, sizeof(&(*phGPSoE)->port));

	ptr = strchr(szDevice, ':');
	if (ptr == NULL)
	{
		PRINT_DEBUG_ERROR_GPSOECFG(("OpenGPSoE error (%s) : %s"
			"(szDevice=%s)\n", 
			strtime_m(), 
			"Invalid TCP/IP address or port. ", 
			szDevice));
		free(*phGPSoE);
		return EXIT_FAILURE;
	}

	memcpy((*phGPSoE)->address, szDevice, ptr-szDevice);
	strcpy((*phGPSoE)->port, ptr+1);

	//if (sscanf(szDevice, "%s:%s", (*phGPSoE)->address, (*phGPSoE)->port) != 2)
	//{
	//	PRINT_DEBUG_ERROR_GPSOECFG(("OpenGPSoE error (%s) : %s"
	//		"(szDevice=%s)\n", 
	//		strtime_m(), 
	//		"Invalid TCP/IP address or port. ", 
	//		szDevice));
	//	free(*phGPSoE);
	//	return EXIT_FAILURE;
	//}

	// Connect to server.
	if (inittcpcli(&(*phGPSoE)->s, (*phGPSoE)->address, (*phGPSoE)->port) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_GPSOECFG(("OpenGPSoE error (%s) : %s"
			"(szDevice=%s)\n", 
			strtime_m(), 
			"Cannot open the TCP/IP address and port. ", 
			szDevice));
		free(*phGPSoE);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Get data from a GPS over Ethernet.

HGPSoE hGPSoE : (IN) Identifier of the device.
double* pUTC : (INOUT) Valid pointer receiving the UTC time.
double* pLatitude : (INOUT) Valid pointer receiving the latitude.
double* pLongitude : (INOUT) Valid pointer receiving the longitude.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int GetDataGPSoE(HGPSOE hGPSoE, double* pUTC, double* pLatitude, double* pLongitude)
{
	if (ReadDataGPSoE(hGPSoE->s, pUTC, pLatitude, pLongitude) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_GPSOECFG(("GetDataGPSoE error (%s) : %s"
			"(hGPSoE=%#x)\n", 
			strtime_m(), 
			"Error getting data from the device. ", 
			hGPSoE));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Close a GPS over Ethernet.

HES* phES : (INOUT) Valid pointer to the identifier of the device.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int CloseGPSoE(HGPSOE* phGPSoE)
{
	BOOL bError = FALSE;

	// Disconnect from server.
	if (releasetcpcli((*phGPSoE)->s) != EXIT_SUCCESS)
	{
		bError = TRUE;
		PRINT_DEBUG_ERROR_GPSOECFG(("CloseGPSOE error (%s) : %s"
			"(*phGPSoE=%#x)\n", 
			strtime_m(), 
			"Error closing the TCP/IP address and port. ", 
			*phGPSoE));
	}

	free(*phGPSoE);

	*phGPSoE = INVALID_HGPSOE_VALUE;

	if (bError)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

#endif // GPSOECFG_H
