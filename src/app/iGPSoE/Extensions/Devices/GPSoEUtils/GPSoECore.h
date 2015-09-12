/***************************************************************************************************************:')

GPSoECore.h

GPS over Ethernet handling.

Fabrice Le Bars

Created: 2012-05-15

Version status: Not finished

***************************************************************************************************************:)*/

#ifndef GPSOECORE_H
#define GPSOECORE_H

#include "OSNet.h"
#include "OSMisc.h"

/*
Debug macros specific to GPSoEUtils.
*/
#ifdef _DEBUG_MESSAGES
#	define _DEBUG_MESSAGES_GPSOEUTILS
#endif // _DEBUG_MESSAGES

#ifdef _DEBUG_WARNINGS
#	define _DEBUG_WARNINGS_GPSOEUTILS
#endif // _DEBUG_WARNINGS

#ifdef _DEBUG_ERRORS
#	define _DEBUG_ERRORS_GPSOEUTILS
#endif // _DEBUG_ERRORS

#ifdef _DEBUG_MESSAGES_GPSOEUTILS
#	define PRINT_DEBUG_MESSAGE_GPSOEUTILS(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_GPSOEUTILS(params)
#endif // _DEBUG_MESSAGES_GPSOEUTILS

#ifdef _DEBUG_WARNINGS_GPSOEUTILS
#	define PRINT_DEBUG_WARNING_GPSOEUTILS(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_GPSOEUTILS(params)
#endif // _DEBUG_WARNINGS_GPSOEUTILS

#ifdef _DEBUG_ERRORS_GPSOEUTILS
#	define PRINT_DEBUG_ERROR_GPSOEUTILS(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_GPSOEUTILS(params)
#endif // _DEBUG_ERRORS_GPSOEUTILS

/*
Debug macros specific to GPSoECore.
*/
#ifdef _DEBUG_MESSAGES_GPSOEUTILS
#	define _DEBUG_MESSAGES_GPSOECORE
#endif // _DEBUG_MESSAGES_GPSOEUTILS

#ifdef _DEBUG_WARNINGS_GPSOEUTILS
#	define _DEBUG_WARNINGS_GPSOECORE
#endif // _DEBUG_WARNINGS_GPSOEUTILS

#ifdef _DEBUG_ERRORS_GPSOEUTILS
#	define _DEBUG_ERRORS_GPSOECORE
#endif // _DEBUG_ERRORS_GPSOEUTILS

#ifdef _DEBUG_MESSAGES_GPSOECORE
#	define PRINT_DEBUG_MESSAGE_GPSOECORE(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_GPSOECORE(params)
#endif // _DEBUG_MESSAGES_GPSOECORE

#ifdef _DEBUG_WARNINGS_GPSOECORE
#	define PRINT_DEBUG_WARNING_GPSOECORE(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_GPSOECORE(params)
#endif // _DEBUG_WARNINGS_GPSOECORE

#ifdef _DEBUG_ERRORS_GPSOECORE
#	define PRINT_DEBUG_ERROR_GPSOECORE(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_GPSOECORE(params)
#endif // _DEBUG_ERRORS_GPSOECORE

#define MAX_NB_BYTES_GPSOE 2048

/*
Read the value at a specific channel of a GPS over Ethernet.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int ReadDataGPSoE(SOCKET s, double* pUTC, double* pLatitude, double* pLongitude)
{
	char recvbuf[MAX_NB_BYTES_GPSOE];
	char savebuf[MAX_NB_BYTES_GPSOE];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	char* ptr_GPGGA = NULL;

	double utc = 0;
	int latdeg = 0, longdeg = 0;
	double latmin = 0, longmin = 0;
	char szlatdeg[3];
	char szlongdeg[4];
	char north = 0, east = 0;
	int GPS_quality_indicator = 0;

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_GPSOE-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	// As the sensor always sends data, any old data must be discarded.
	Bytes = recv(s, recvbuf, recvbuflen, 0);
	if (Bytes <= 0)
	{
		printf("recv() failed or timed out.\n");
		return EXIT_FAILURE;
	}
	BytesReceived += Bytes;

	while (Bytes == recvbuflen)
	{
		memcpy(savebuf, recvbuf, Bytes);

		Bytes = recv(s, recvbuf, recvbuflen, 0);
		if (Bytes <= 0)
		{
			printf("recv() failed or timed out.\n");
			return EXIT_FAILURE;
		}
		BytesReceived += Bytes;
	}

	if (BytesReceived < recvbuflen)
	{
		ptr_GPGGA = FindLatestNMEASentence("$GPGGA", recvbuf);

		while (!ptr_GPGGA)
		{
			if (BytesReceived >= recvbuflen) return EXIT_FAILURE;

			Bytes = recv(s, recvbuf+BytesReceived, recvbuflen-BytesReceived, 0);
			if (Bytes <= 0)
			{
				printf("recv() failed or timed out.\n");
				return EXIT_FAILURE;
			}
			BytesReceived += Bytes;

			ptr_GPGGA = FindLatestNMEASentence("$GPGGA", recvbuf);
		}
	}
	else
	{
		memmove(recvbuf+recvbuflen-Bytes, recvbuf, Bytes);
		memcpy(recvbuf, savebuf+Bytes, recvbuflen-Bytes);

		ptr_GPGGA = FindLatestNMEASentence("$GPGGA", recvbuf);

		if (!ptr_GPGGA ) return EXIT_FAILURE;
	}

	// Display and analyse received data.
	//printf("Received : \"%s\"\n", recvbuf);

	// GPS data.
	memset(szlatdeg, 0, sizeof(szlatdeg));
	memset(szlongdeg, 0, sizeof(szlongdeg));

	if (sscanf(ptr_GPGGA, "$GPGGA,%lf,%c%c%lf,%c,%c%c%c%lf,%c,%d", &utc, 
		&szlatdeg[0], &szlatdeg[1], &latmin, &north, 
		&szlongdeg[0], &szlongdeg[1], &szlongdeg[2], &longmin, &east,
		&GPS_quality_indicator) != 11)
	{
		printf("Error reading data from the GPS over Ethernet.\n");
		return EXIT_FAILURE;
	}

	if (GPS_quality_indicator == 0) 
	{
		printf("GPS fix not available or invalid.\n");
		return EXIT_FAILURE;
	}

	latdeg = atoi(szlatdeg);
	longdeg = atoi(szlongdeg);

	// Convert GPS latitude and longitude in decimal.
	*pLatitude = (north == 'N')?(latdeg+latmin/60.0):-(latdeg+latmin/60.0);
	*pLongitude = (east == 'E')?(longdeg+longmin/60.0):-(longdeg+longmin/60.0);

	*pUTC = utc;

	PRINT_DEBUG_MESSAGE_GPSOECORE(("UTC : %f, Lat : %f, Long : %f\n", *pUTC, *pLatitude, *pLongitude));

	return EXIT_SUCCESS;
}

#endif // GPSOECORE_H
