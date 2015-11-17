/***************************************************************************************************************:')

OSTime.h

Functions related to the time.

Fabrice Le Bars
GetTickCount() from the file ue9.c provided by www.labjack.com
strtime_m() and strtime_fns() are based on a part of an example in the MSDN library

Created : 2009-01-28

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

#ifndef OSTIME_H
#define OSTIME_H

#include "OSCore.h"

/*
Debug macros specific to OSTime.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSTIME
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSTIME
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSTIME
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSTIME
#	define PRINT_DEBUG_MESSAGE_OSTIME(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSTIME(params)
#endif // _DEBUG_MESSAGES_OSTIME

#ifdef _DEBUG_WARNINGS_OSTIME
#	define PRINT_DEBUG_WARNING_OSTIME(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSTIME(params)
#endif // _DEBUG_WARNINGS_OSTIME

#ifdef _DEBUG_ERRORS_OSTIME
#	define PRINT_DEBUG_ERROR_OSTIME(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSTIME(params)
#endif // _DEBUG_ERRORS_OSTIME

#ifdef _WIN32
#ifdef ENABLE_GETTIMEOFDAY_WIN32
// To get struct timeval.
#include <winsock2.h>
// To get struct _timeb.
//#include <sys/timeb.h>
#endif // ENABLE_GETTIMEOFDAY_WIN32
#else 
// CLOCK_MONOTONIC is a clock that cannot be set and represents monotonic time since 
// some unspecified starting point.
// CLOCK_MONOTONIC_RAW is available since Linux 2.6.28 and provides access to a raw 
// hardware-based time that is not subject to NTP adjustments.
#ifndef CLOCK_MONOTONIC_RAW 
#define CLOCK_MONOTONIC_RAW CLOCK_MONOTONIC
#endif // CLOCK_MONOTONIC_RAW 
#endif // _WIN32

EXTERN_C char strftime_m_tmpbuf[32]; // Used to store the string returned by strtime_m().
EXTERN_C char strftime_fns_tmpbuf[32]; // Used to store the string returned by strtime_fns().

#ifndef USE_OLD_CHRONO
#ifdef _WIN32
struct CHRONO
{
	LARGE_INTEGER Start;
	LARGE_INTEGER Finish;
	LARGE_INTEGER Duration;
	LARGE_INTEGER Frequency;
	BOOL Suspended; // Used to know if the chronometer is currently suspended.
};
typedef struct CHRONO CHRONO;
#else 
struct CHRONO
{
	struct timespec Start;
	struct timespec Finish;
	struct timespec Duration;
	struct timespec Frequency;
	BOOL Suspended; // Used to know if the chronometer is currently suspended.
};
typedef struct CHRONO CHRONO;
#endif // _WIN32
#else
#ifdef ENABLE_GETTIMEOFDAY_WIN32
/*
Structure for a basic chronometer.
Might not work correctly if used during more than approximately 68 years. 
The accuracy should be around 15 ms, but it is subject to system time changes 
(automatic network time synchronization, user changing the system time...).
*/
struct CHRONO
{
	struct timeval Start;
	struct timeval Finish;
	struct timeval Duration;
	BOOL Suspended; // Used to know if the chronometer is currently suspended.
};
typedef struct CHRONO CHRONO;
#else
/*
Structure for a basic chronometer.
Might not work correctly if used during more than approximately 72 min for Linux 
or 50 days for Windows.
*/
struct CHRONO
{
	clock_t Start;
	clock_t Finish;
	double Duration;
	BOOL Suspended; // Used to know if the chronometer is currently suspended.
};
typedef struct CHRONO CHRONO;
#endif // ENABLE_GETTIMEOFDAY_WIN32
#endif // USE_OLD_CHRONO

#ifdef _WIN32
#ifdef ENABLE_GETTIMEOFDAY_WIN32
//#define DELTA_EPOCH_IN_MICROSECS 11644473600000000ui64
//#define DELTA_EPOCH_IN_MICROSECS 11644473600000000ULL
#define DELTA_EPOCH_IN_MICROSECS_LOW 1216757760
#define DELTA_EPOCH_IN_MICROSECS_HIGH 2711190

#ifdef _MSC_VER
// Disable Visual Studio warnings about timezone declaration.
#pragma warning(disable : 6244) 
#endif // _MSC_VER

// Local declaration of timezone hides previous declaration in time.h.
struct timezone 
{
	int tz_minuteswest; // Minutes W of Greenwich.
	int tz_dsttime; // Type of DST correction.
};

#ifdef _MSC_VER
// Restore the Visual Studio warnings previously disabled.
#pragma warning(default : 6244) 
#endif // _MSC_VER

/*
Obtain the current time, expressed as seconds and microseconds since the Epoch, 
and store it in the timeval structure pointed to by tv (the accuracy should be
around 15 ms).

struct timeval* tv : (INOUT) Valid pointer to the structure that will receive 
the current time.
struct timezone* tz : (INOUT) Usually set to NULL.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int gettimeofday(struct timeval* tv, struct timezone* tz)
{
	FILETIME ft; // Will contain a 64-bit value representing the number of 100-nanosecond 
	// intervals since January 1, 1601 (UTC).
	ULONGLONG tmpres = 0;
	ULARGE_INTEGER li;
	ULARGE_INTEGER epoch;
#ifdef USE__TZSET
	static int tzflag;
#else
	TIME_ZONE_INFORMATION tz_winapi;
	int rez = 0; 
#endif // USE__TZSET

	if (tv)
	{
		GetSystemTimeAsFileTime(&ft);
		li.LowPart = ft.dwLowDateTime;
		li.HighPart = ft.dwHighDateTime;

		// Converting file time to UNIX Epoch.
		tmpres = li.QuadPart/(ULONGLONG)10; // Convert into microseconds.
		//tmpres -= DELTA_EPOCH_IN_MICROSECS;
		epoch.LowPart = DELTA_EPOCH_IN_MICROSECS_LOW;
		epoch.HighPart = DELTA_EPOCH_IN_MICROSECS_HIGH;
		tmpres -= epoch.QuadPart;

		tv->tv_sec = (long)(tmpres/(ULONGLONG)1000000);
		tv->tv_usec = (long)(tmpres%(ULONGLONG)1000000);
	}

#ifdef USE__TZSET
	if (tz)
	{
		if (!tzflag)
		{
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone/60;
		tz->tz_dsttime = _daylight;
	}
#else
	if (tz)
	{
		// _tzset(), do not work properly, so we use GetTimeZoneInformation.   
		rez = GetTimeZoneInformation(&tz_winapi);     
		tz->tz_dsttime = (rez == 2)?TRUE:FALSE;     
		tz->tz_minuteswest = tz_winapi.Bias + ((rez == 2)?tz_winapi.DaylightBias:0); 
	}
#endif // USE__TZSET

	return EXIT_SUCCESS;
}
//inline int gettimeofday(struct timeval* tp, void* tz)
//{
//	struct _timeb timebuffer;
//
//	UNREFERENCED_PARAMETER(tz);
//
//	_ftime(&timebuffer);
//	tp->tv_sec = (long)timebuffer.time;
//	tp->tv_usec = timebuffer.millitm*1000;
//	return 0;
//}
#endif // ENABLE_GETTIMEOFDAY_WIN32
#endif // _WIN32

#ifndef _WIN32
/* 
Retrieve the number of milliseconds that has elasped since the system was started (Windows) 
or the time since the Epoch (Linux). The accuracy should be around 15 ms and the value will 
wrap around to 0 after 50 days for Windows.

Return : This number.
*/
inline DWORD GetTickCount(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (DWORD)((tv.tv_sec*1000)+(tv.tv_usec/1000));
}
#endif // _WIN32

/*
Return a string like ctime() but in this format :

Mon Aug 27 19:28:04 2007\0

(without the "\n" of ctime()).
Should not be used directly in concurrent threads as the string value returned might 
be changed by another thread.

Return : This string.
*/
EXTERN_C char* strtime_m(void);

/*
Return a string like ctime() but in this format :

Mon_Aug_27_19h28min04sec_2007\0

(without the "\n", ":", " " of ctime() in order to be safely used in file names).
Should not be used directly in concurrent threads as the string value returned might 
be changed by another thread.

Return : This string.
*/
EXTERN_C char* strtime_fns(void);

/*
Wait some time...

long Milliseconds : (IN) Time to wait in ms.

Return : Nothing.
*/
inline void mSleep(long Milliseconds)
{
#ifdef _WIN32
	Sleep(Milliseconds);
#else 
	// usleep() is considered as obsolete.
	//usleep(Milliseconds*1000);
	struct timespec req;

	req.tv_sec = Milliseconds/1000; // Seconds.
	req.tv_nsec = (Milliseconds%1000)*1000000; // Additional nanoseconds.
	nanosleep(&req, NULL);
#endif // _WIN32
}

inline void DecSec2DaysHoursMinSec(double decsec, int* pDays, int* pHours, int* pMin, int* pSec, double* pDeccsec)
{
	int hours = 0, minutes = 0, seconds = 0; 

	seconds = (int)decsec;
	*pSec = seconds%60;
	minutes = seconds/60;
	*pMin = minutes%60;
	hours = minutes/60;
	*pHours = hours%24;
	*pDays = hours/24;

	*pDeccsec = (decsec-seconds)*100.0;
}

#ifndef USE_OLD_CHRONO
#ifdef _WIN32
/*
Start a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int StartChrono(CHRONO* pChrono)
{
	if (!QueryPerformanceFrequency(&pChrono->Frequency))
	{
		PRINT_DEBUG_ERROR_OSTIME(("StartChrono error (%s) : %s\n", 
			strtime_m(), 
			GetLastErrorMsg()));
		return EXIT_FAILURE;
	}

	if (!QueryPerformanceCounter(&pChrono->Start))
	{
		PRINT_DEBUG_ERROR_OSTIME(("StartChrono error (%s) : %s\n", 
			strtime_m(), 
			GetLastErrorMsg()));
		return EXIT_FAILURE;
	}

	pChrono->Finish.QuadPart = 0;
	pChrono->Duration.QuadPart = 0;
	pChrono->Suspended = FALSE;

	return EXIT_SUCCESS;
}

/*
Suspend a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.
double* pDuration : (INOUT) Duration (in s).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SuspendChrono(CHRONO* pChrono, double* pDuration)
{
	LARGE_INTEGER temp;

	if (!pChrono->Suspended)
	{
		pChrono->Suspended = TRUE;
		if (!QueryPerformanceCounter(&temp))
		{
			PRINT_DEBUG_ERROR_OSTIME(("SuspendChrono error (%s) : %s"
				"(pChrono=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pChrono));
			return EXIT_FAILURE;
		}

		//pChrono->Duration += pChrono->Finish-pChrono->Start
		pChrono->Finish = temp;
		temp.QuadPart += pChrono->Duration.QuadPart-pChrono->Start.QuadPart;
		pChrono->Duration = temp;
		*pDuration = (double)pChrono->Duration.QuadPart/(double)pChrono->Frequency.QuadPart;
	}
	else
	{
		PRINT_DEBUG_ERROR_OSTIME(("SuspendChrono error (%s) : %s"
			"(pChrono=%#x)\n", 
			strtime_m(), 
			"Chrono already suspended. ", 
			pChrono));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Suspend a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : Duration (in s).
*/
inline double SuspendChronoQuick(CHRONO* pChrono)
{
	LARGE_INTEGER temp;

	if (!pChrono->Suspended)
	{
		pChrono->Suspended = TRUE;
		QueryPerformanceCounter(&temp);

		//pChrono->Duration += pChrono->Finish-pChrono->Start
		pChrono->Finish = temp;
		temp.QuadPart += pChrono->Duration.QuadPart-pChrono->Start.QuadPart;
		pChrono->Duration = temp;
		return (double)pChrono->Duration.QuadPart/(double)pChrono->Frequency.QuadPart;
	}
	else
	{
		PRINT_DEBUG_ERROR_OSTIME(("SuspendChronoQuick error (%s) : %s"
			"(pChrono=%#x)\n", 
			strtime_m(), 
			"Chrono already suspended. ", 
			pChrono));
		return -1;
	}
}

/*
Resume a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int ResumeChrono(CHRONO* pChrono)
{
	if (pChrono->Suspended)
	{
		pChrono->Suspended = FALSE;
		if (!QueryPerformanceCounter(&pChrono->Start))
		{
			PRINT_DEBUG_ERROR_OSTIME(("ResumeChrono error (%s) : %s"
				"(pChrono=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pChrono));
			return EXIT_FAILURE;
		}
	}
	else
	{
		PRINT_DEBUG_ERROR_OSTIME(("ResumeChrono error (%s) : %s"
			"(pChrono=%#x)\n", 
			strtime_m(), 
			"Chrono not suspended. ", 
			pChrono));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Get the time elapsed of a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.
double* pDuration : (INOUT) Duration (in s).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int GetTimeElapsedChrono(CHRONO* pChrono, double* pDuration)
{
	LARGE_INTEGER temp;

	if (pChrono->Suspended)
	{
		*pDuration = (double)pChrono->Duration.QuadPart/(double)pChrono->Frequency.QuadPart;
	}
	else
	{
		if (!QueryPerformanceCounter(&temp))
		{
			PRINT_DEBUG_ERROR_OSTIME(("GetTimeElapsedChrono error (%s) : %s"
				"(pChrono=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pChrono));
			return EXIT_FAILURE;
		}

		//*pDuration = pChrono->Duration+temp-pChrono->Start
		temp.QuadPart += pChrono->Duration.QuadPart-pChrono->Start.QuadPart;
		*pDuration = (double)temp.QuadPart/(double)pChrono->Frequency.QuadPart;
	}

	return EXIT_SUCCESS;
}

/*
Get the time elapsed of a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : Duration (in s).
*/
inline double GetTimeElapsedChronoQuick(CHRONO* pChrono)
{
	LARGE_INTEGER temp;

	if (pChrono->Suspended)
	{
		return (double)pChrono->Duration.QuadPart/(double)pChrono->Frequency.QuadPart;
	}
	else
	{
		QueryPerformanceCounter(&temp);

		//*pDuration = pChrono->Duration+temp-pChrono->Start
		temp.QuadPart += pChrono->Duration.QuadPart-pChrono->Start.QuadPart;
		return (double)temp.QuadPart/(double)pChrono->Frequency.QuadPart;
	}
}

/*
Stop a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.
double* pDuration : (INOUT) Duration (in s).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int StopChrono(CHRONO* pChrono, double* pDuration)
{
	LARGE_INTEGER temp;

	if (pChrono->Suspended)
	{
		*pDuration = (double)pChrono->Duration.QuadPart/(double)pChrono->Frequency.QuadPart;
	}
	else
	{
		if (!QueryPerformanceCounter(&temp))
		{
			PRINT_DEBUG_ERROR_OSTIME(("StopChrono error (%s) : %s"
				"(pChrono=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pChrono));
			return EXIT_FAILURE;
		}

		//pChrono->Duration += pChrono->Finish-pChrono->Start
		pChrono->Finish = temp;
		temp.QuadPart += pChrono->Duration.QuadPart-pChrono->Start.QuadPart;
		pChrono->Duration = temp;
		*pDuration = (double)pChrono->Duration.QuadPart/(double)pChrono->Frequency.QuadPart;
	}

	return EXIT_SUCCESS;
}

/*
Stop a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : Duration (in s).
*/
inline double StopChronoQuick(CHRONO* pChrono)
{
	LARGE_INTEGER temp;

	if (pChrono->Suspended)
	{
		return (double)pChrono->Duration.QuadPart/(double)pChrono->Frequency.QuadPart;
	}
	else
	{
		QueryPerformanceCounter(&temp);

		//pChrono->Duration += pChrono->Finish-pChrono->Start
		pChrono->Finish = temp;
		temp.QuadPart += pChrono->Duration.QuadPart-pChrono->Start.QuadPart;
		pChrono->Duration = temp;
		return (double)pChrono->Duration.QuadPart/(double)pChrono->Frequency.QuadPart;
	}
}
#else 
/*
Start a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int StartChrono(CHRONO* pChrono)
{
	if (clock_getres(CLOCK_MONOTONIC_RAW, &pChrono->Frequency) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTIME(("StartChrono error (%s) : %s\n", 
			strtime_m(), 
			GetLastErrorMsg()));
		return EXIT_FAILURE;
	}

	if (clock_gettime(CLOCK_MONOTONIC_RAW, &pChrono->Start) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSTIME(("StartChrono error (%s) : %s\n", 
			strtime_m(), 
			GetLastErrorMsg()));
		return EXIT_FAILURE;
	}

	pChrono->Finish.tv_sec = 0;
	pChrono->Finish.tv_nsec = 0;
	pChrono->Duration.tv_sec = 0;
	pChrono->Duration.tv_nsec = 0;
	pChrono->Suspended = FALSE;

	return EXIT_SUCCESS;
}

/*
Suspend a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.
double* pDuration : (INOUT) Duration (in s).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SuspendChrono(CHRONO* pChrono, double* pDuration)
{
	struct timespec temp;

	if (!pChrono->Suspended)
	{
		pChrono->Suspended = TRUE;
		if (clock_gettime(CLOCK_MONOTONIC_RAW, &temp) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIME(("SuspendChrono error (%s) : %s"
				"(pChrono=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pChrono));
			return EXIT_FAILURE;
		}

		//pChrono->Duration += pChrono->Finish-pChrono->Start

		pChrono->Finish = temp;

		temp.tv_sec += pChrono->Duration.tv_sec; // Seconds.
		temp.tv_nsec += pChrono->Duration.tv_nsec; // Additional nanoseconds.
		// tv_nsec must stay in [0,1000000000].
		temp.tv_sec += temp.tv_nsec/1000000000;
		temp.tv_nsec = temp.tv_nsec%1000000000;

		temp.tv_sec -= pChrono->Start.tv_sec;
		temp.tv_nsec -= pChrono->Start.tv_nsec;
		// tv_nsec must stay in [0,1000000000].
		if (temp.tv_nsec < 0)
		{
			temp.tv_sec += -1+temp.tv_nsec/1000000000;
			temp.tv_nsec = 1000000000+temp.tv_nsec%1000000000;
		}

		pChrono->Duration = temp;

		*pDuration = pChrono->Duration.tv_sec+(double)pChrono->Duration.tv_nsec/1000000000.0;
	}
	else
	{
		PRINT_DEBUG_ERROR_OSTIME(("SuspendChrono error (%s) : %s"
			"(pChrono=%#x)\n", 
			strtime_m(), 
			"Chrono already suspended. ", 
			pChrono));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Suspend a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : Duration (in s).
*/
inline double SuspendChronoQuick(CHRONO* pChrono)
{
	struct timespec temp;

	if (!pChrono->Suspended)
	{
		pChrono->Suspended = TRUE;
		clock_gettime(CLOCK_MONOTONIC_RAW, &temp);

		//pChrono->Duration += pChrono->Finish-pChrono->Start

		pChrono->Finish = temp;

		temp.tv_sec += pChrono->Duration.tv_sec; // Seconds.
		temp.tv_nsec += pChrono->Duration.tv_nsec; // Additional nanoseconds.
		// tv_nsec must stay in [0,1000000000].
		temp.tv_sec += temp.tv_nsec/1000000000;
		temp.tv_nsec = temp.tv_nsec%1000000000;

		temp.tv_sec -= pChrono->Start.tv_sec;
		temp.tv_nsec -= pChrono->Start.tv_nsec;
		// tv_nsec must stay in [0,1000000000].
		if (temp.tv_nsec < 0)
		{
			temp.tv_sec += -1+temp.tv_nsec/1000000000;
			temp.tv_nsec = 1000000000+temp.tv_nsec%1000000000;
		}

		pChrono->Duration = temp;

		return pChrono->Duration.tv_sec+(double)pChrono->Duration.tv_nsec/1000000000.0;
	}
	else
	{
		PRINT_DEBUG_ERROR_OSTIME(("SuspendChronoQuick error (%s) : %s"
			"(pChrono=%#x)\n", 
			strtime_m(), 
			"Chrono already suspended. ", 
			pChrono));
		return -1;
	}
}

/*
Resume a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int ResumeChrono(CHRONO* pChrono)
{
	if (pChrono->Suspended)
	{
		pChrono->Suspended = FALSE;
		if (clock_gettime(CLOCK_MONOTONIC_RAW, &pChrono->Start) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIME(("ResumeChrono error (%s) : %s"
				"(pChrono=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pChrono));
			return EXIT_FAILURE;
		}
	}
	else
	{
		PRINT_DEBUG_ERROR_OSTIME(("ResumeChrono error (%s) : %s"
			"(pChrono=%#x)\n", 
			strtime_m(), 
			"Chrono not suspended. ", 
			pChrono));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Get the time elapsed of a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.
double* pDuration : (INOUT) Duration (in s).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int GetTimeElapsedChrono(CHRONO* pChrono, double* pDuration)
{
	struct timespec temp;

	if (pChrono->Suspended)
	{
		*pDuration = pChrono->Duration.tv_sec+(double)pChrono->Duration.tv_nsec/1000000000.0;
	}
	else
	{
		if (clock_gettime(CLOCK_MONOTONIC_RAW, &temp) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIME(("GetTimeElapsedChrono error (%s) : %s"
				"(pChrono=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pChrono));
			return EXIT_FAILURE;
		}

		//*pDuration = pChrono->Duration+temp-pChrono->Start

		temp.tv_sec += pChrono->Duration.tv_sec; // Seconds.
		temp.tv_nsec += pChrono->Duration.tv_nsec; // Additional nanoseconds.
		// tv_nsec must stay in [0,1000000000].
		temp.tv_sec += temp.tv_nsec/1000000000;
		temp.tv_nsec = temp.tv_nsec%1000000000;

		temp.tv_sec -= pChrono->Start.tv_sec;
		temp.tv_nsec -= pChrono->Start.tv_nsec;
		// tv_nsec must stay in [0,1000000000].
		if (temp.tv_nsec < 0)
		{
			temp.tv_sec += -1+temp.tv_nsec/1000000000;
			temp.tv_nsec = 1000000000+temp.tv_nsec%1000000000;
		}

		*pDuration = temp.tv_sec+(double)temp.tv_nsec/1000000000.0;
	}

	return EXIT_SUCCESS;
}

/*
Get the time elapsed of a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : Duration (in s).
*/
inline double GetTimeElapsedChronoQuick(CHRONO* pChrono)
{
	struct timespec temp;

	if (pChrono->Suspended)
	{
		return pChrono->Duration.tv_sec+(double)pChrono->Duration.tv_nsec/1000000000.0;
	}
	else
	{
		clock_gettime(CLOCK_MONOTONIC_RAW, &temp);

		//*pDuration = pChrono->Duration+temp-pChrono->Start

		temp.tv_sec += pChrono->Duration.tv_sec; // Seconds.
		temp.tv_nsec += pChrono->Duration.tv_nsec; // Additional nanoseconds.
		// tv_nsec must stay in [0,1000000000].
		temp.tv_sec += temp.tv_nsec/1000000000;
		temp.tv_nsec = temp.tv_nsec%1000000000;

		temp.tv_sec -= pChrono->Start.tv_sec;
		temp.tv_nsec -= pChrono->Start.tv_nsec;
		// tv_nsec must stay in [0,1000000000].
		if (temp.tv_nsec < 0)
		{
			temp.tv_sec += -1+temp.tv_nsec/1000000000;
			temp.tv_nsec = 1000000000+temp.tv_nsec%1000000000;
		}

		return temp.tv_sec+(double)temp.tv_nsec/1000000000.0;
	}
}

/*
Stop a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.
double* pDuration : (INOUT) Duration (in s).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int StopChrono(CHRONO* pChrono, double* pDuration)
{
	struct timespec temp;

	if (pChrono->Suspended)
	{
		*pDuration = pChrono->Duration.tv_sec+(double)pChrono->Duration.tv_nsec/1000000000.0;
	}
	else
	{
		if (clock_gettime(CLOCK_MONOTONIC_RAW, &temp) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_OSTIME(("StopChrono error (%s) : %s"
				"(pChrono=%#x)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				pChrono));
			return EXIT_FAILURE;
		}

		//pChrono->Duration += pChrono->Finish-pChrono->Start

		pChrono->Finish = temp;

		temp.tv_sec += pChrono->Duration.tv_sec; // Seconds.
		temp.tv_nsec += pChrono->Duration.tv_nsec; // Additional nanoseconds.
		// tv_nsec must stay in [0,1000000000].
		temp.tv_sec += temp.tv_nsec/1000000000;
		temp.tv_nsec = temp.tv_nsec%1000000000;

		temp.tv_sec -= pChrono->Start.tv_sec;
		temp.tv_nsec -= pChrono->Start.tv_nsec;
		// tv_nsec must stay in [0,1000000000].
		if (temp.tv_nsec < 0)
		{
			temp.tv_sec += -1+temp.tv_nsec/1000000000;
			temp.tv_nsec = 1000000000+temp.tv_nsec%1000000000;
		}

		pChrono->Duration = temp;

		*pDuration = pChrono->Duration.tv_sec+(double)pChrono->Duration.tv_nsec/1000000000.0;
	}

	return EXIT_SUCCESS;
}

/*
Stop a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : Duration (in s).
*/
inline double StopChronoQuick(CHRONO* pChrono)
{
	struct timespec temp;

	if (pChrono->Suspended)
	{
		return pChrono->Duration.tv_sec+(double)pChrono->Duration.tv_nsec/1000000000.0;
	}
	else
	{
		clock_gettime(CLOCK_MONOTONIC_RAW, &temp);

		//pChrono->Duration += pChrono->Finish-pChrono->Start

		pChrono->Finish = temp;

		temp.tv_sec += pChrono->Duration.tv_sec; // Seconds.
		temp.tv_nsec += pChrono->Duration.tv_nsec; // Additional nanoseconds.
		// tv_nsec must stay in [0,1000000000].
		temp.tv_sec += temp.tv_nsec/1000000000;
		temp.tv_nsec = temp.tv_nsec%1000000000;

		temp.tv_sec -= pChrono->Start.tv_sec;
		temp.tv_nsec -= pChrono->Start.tv_nsec;
		// tv_nsec must stay in [0,1000000000].
		if (temp.tv_nsec < 0)
		{
			temp.tv_sec += -1+temp.tv_nsec/1000000000;
			temp.tv_nsec = 1000000000+temp.tv_nsec%1000000000;
		}

		pChrono->Duration = temp;

		return pChrono->Duration.tv_sec+(double)pChrono->Duration.tv_nsec/1000000000.0;
	}
}
#endif // _WIN32
#else
#ifdef ENABLE_GETTIMEOFDAY_WIN32
/*
Start a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int StartChrono(CHRONO* pChrono)
{
	gettimeofday(&pChrono->Start, NULL);
	pChrono->Finish.tv_sec = 0;
	pChrono->Finish.tv_usec = 0;
	pChrono->Duration.tv_sec = 0;
	pChrono->Duration.tv_usec = 0;
	pChrono->Suspended = FALSE;

	return EXIT_SUCCESS;
}

/*
Suspend a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.
double* pDuration : (INOUT) Duration (in s).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SuspendChrono(CHRONO* pChrono, double* pDuration)
{
	struct timeval temp;

	if (!pChrono->Suspended)
	{
		pChrono->Suspended = TRUE;
		gettimeofday(&pChrono->Finish, NULL);

		//pChrono->Duration += pChrono->Finish-pChrono->Start

		temp = pChrono->Finish;

		temp.tv_sec += pChrono->Duration.tv_sec; // Seconds.
		temp.tv_usec += pChrono->Duration.tv_usec; // Additional microseconds.
		// tv_usec must stay in [0,1000000].
		temp.tv_sec += temp.tv_usec/1000000;
		temp.tv_usec = temp.tv_usec%1000000;

		temp.tv_sec -= pChrono->Start.tv_sec;
		temp.tv_usec -= pChrono->Start.tv_usec;
		// tv_usec must stay in [0,1000000].
		if (temp.tv_usec < 0)
		{
			temp.tv_sec += -1+temp.tv_usec/1000000;
			temp.tv_usec = 1000000+temp.tv_usec%1000000;
		}

		pChrono->Duration = temp;

		*pDuration = pChrono->Duration.tv_sec+(double)pChrono->Duration.tv_usec/1000000.0;
	}
	else
	{
		PRINT_DEBUG_ERROR_OSTIME(("SuspendChrono error (%s) : %s"
			"(pChrono=%#x)\n", 
			strtime_m(), 
			"Chrono already suspended. ", 
			pChrono));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Suspend a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : Duration (in s).
*/
inline double SuspendChronoQuick(CHRONO* pChrono)
{
	struct timeval temp;

	if (!pChrono->Suspended)
	{
		pChrono->Suspended = TRUE;
		gettimeofday(&pChrono->Finish, NULL);

		//pChrono->Duration += pChrono->Finish-pChrono->Start

		temp = pChrono->Finish;

		temp.tv_sec += pChrono->Duration.tv_sec; // Seconds.
		temp.tv_usec += pChrono->Duration.tv_usec; // Additional microseconds.
		// tv_usec must stay in [0,1000000].
		temp.tv_sec += temp.tv_usec/1000000;
		temp.tv_usec = temp.tv_usec%1000000;

		temp.tv_sec -= pChrono->Start.tv_sec;
		temp.tv_usec -= pChrono->Start.tv_usec;
		// tv_usec must stay in [0,1000000].
		if (temp.tv_usec < 0)
		{
			temp.tv_sec += -1+temp.tv_usec/1000000;
			temp.tv_usec = 1000000+temp.tv_usec%1000000;
		}

		pChrono->Duration = temp;

		return pChrono->Duration.tv_sec+(double)pChrono->Duration.tv_usec/1000000.0;
	}
	else
	{
		PRINT_DEBUG_ERROR_OSTIME(("SuspendChronoQuick error (%s) : %s"
			"(pChrono=%#x)\n", 
			strtime_m(), 
			"Chrono already suspended. ", 
			pChrono));
		return -1;
	}
}

/*
Resume a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int ResumeChrono(CHRONO* pChrono)
{
	if (pChrono->Suspended)
	{
		pChrono->Suspended = FALSE;
		gettimeofday(&pChrono->Start, NULL);
	}
	else
	{
		PRINT_DEBUG_ERROR_OSTIME(("ResumeChrono error (%s) : %s"
			"(pChrono=%#x)\n", 
			strtime_m(), 
			"Chrono not suspended. ", 
			pChrono));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Get the time elapsed of a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.
double* pDuration : (INOUT) Duration (in s).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int GetTimeElapsedChrono(CHRONO* pChrono, double* pDuration)
{
	struct timeval temp;

	if (pChrono->Suspended)
	{
		*pDuration = pChrono->Duration.tv_sec+(double)pChrono->Duration.tv_usec/1000000.0;
	}
	else
	{
		gettimeofday(&temp, NULL);

		//*pDuration = pChrono->Duration+temp-pChrono->Start

		temp.tv_sec += pChrono->Duration.tv_sec; // Seconds.
		temp.tv_usec += pChrono->Duration.tv_usec; // Additional microseconds.
		// tv_usec must stay in [0,1000000].
		temp.tv_sec += temp.tv_usec/1000000;
		temp.tv_usec = temp.tv_usec%1000000;

		temp.tv_sec -= pChrono->Start.tv_sec;
		temp.tv_usec -= pChrono->Start.tv_usec;
		// tv_usec must stay in [0,1000000].
		if (temp.tv_usec < 0)
		{
			temp.tv_sec += -1+temp.tv_usec/1000000;
			temp.tv_usec = 1000000+temp.tv_usec%1000000;
		}

		*pDuration = temp.tv_sec+(double)temp.tv_usec/1000000.0;
	}

	return EXIT_SUCCESS;
}

/*
Get the time elapsed of a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : Duration (in s).
*/
inline double GetTimeElapsedChronoQuick(CHRONO* pChrono)
{
	struct timeval temp;

	if (pChrono->Suspended)
	{
		return pChrono->Duration.tv_sec+(double)pChrono->Duration.tv_usec/1000000.0;
	}
	else
	{
		gettimeofday(&temp, NULL);

		//*pDuration = pChrono->Duration+temp-pChrono->Start

		temp.tv_sec += pChrono->Duration.tv_sec; // Seconds.
		temp.tv_usec += pChrono->Duration.tv_usec; // Additional microseconds.
		// tv_usec must stay in [0,1000000].
		temp.tv_sec += temp.tv_usec/1000000;
		temp.tv_usec = temp.tv_usec%1000000;

		temp.tv_sec -= pChrono->Start.tv_sec;
		temp.tv_usec -= pChrono->Start.tv_usec;
		// tv_usec must stay in [0,1000000].
		if (temp.tv_usec < 0)
		{
			temp.tv_sec += -1+temp.tv_usec/1000000;
			temp.tv_usec = 1000000+temp.tv_usec%1000000;
		}

		return temp.tv_sec+(double)temp.tv_usec/1000000.0;
	}
}

/*
Stop a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.
double* pDuration : (INOUT) Duration (in s).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int StopChrono(CHRONO* pChrono, double* pDuration)
{
	struct timeval temp;

	if (pChrono->Suspended)
	{
		*pDuration = pChrono->Duration.tv_sec+(double)pChrono->Duration.tv_usec/1000000.0;
	}
	else
	{
		gettimeofday(&pChrono->Finish, NULL);

		//pChrono->Duration += pChrono->Finish-pChrono->Start

		temp = pChrono->Finish;

		temp.tv_sec += pChrono->Duration.tv_sec; // Seconds.
		temp.tv_usec += pChrono->Duration.tv_usec; // Additional microseconds.
		// tv_usec must stay in [0,1000000].
		temp.tv_sec += temp.tv_usec/1000000;
		temp.tv_usec = temp.tv_usec%1000000;

		temp.tv_sec -= pChrono->Start.tv_sec;
		temp.tv_usec -= pChrono->Start.tv_usec;
		// tv_usec must stay in [0,1000000].
		if (temp.tv_usec < 0)
		{
			temp.tv_sec += -1+temp.tv_usec/1000000;
			temp.tv_usec = 1000000+temp.tv_usec%1000000;
		}

		pChrono->Duration = temp;

		*pDuration = pChrono->Duration.tv_sec+(double)pChrono->Duration.tv_usec/1000000.0;
	}

	return EXIT_SUCCESS;
}

/*
Stop a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : Duration (in s).
*/
inline double StopChronoQuick(CHRONO* pChrono)
{
	struct timeval temp;

	if (pChrono->Suspended)
	{
		return pChrono->Duration.tv_sec+(double)pChrono->Duration.tv_usec/1000000.0;
	}
	else
	{
		gettimeofday(&pChrono->Finish, NULL);

		//pChrono->Duration += pChrono->Finish-pChrono->Start

		temp = pChrono->Finish;

		temp.tv_sec += pChrono->Duration.tv_sec; // Seconds.
		temp.tv_usec += pChrono->Duration.tv_usec; // Additional microseconds.
		// tv_usec must stay in [0,1000000].
		temp.tv_sec += temp.tv_usec/1000000;
		temp.tv_usec = temp.tv_usec%1000000;

		temp.tv_sec -= pChrono->Start.tv_sec;
		temp.tv_usec -= pChrono->Start.tv_usec;
		// tv_usec must stay in [0,1000000].
		if (temp.tv_usec < 0)
		{
			temp.tv_sec += -1+temp.tv_usec/1000000;
			temp.tv_usec = 1000000+temp.tv_usec%1000000;
		}

		pChrono->Duration = temp;

		return pChrono->Duration.tv_sec+(double)pChrono->Duration.tv_usec/1000000.0;
	}
}
#else
/*
Start a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int StartChrono(CHRONO* pChrono)
{
	pChrono->Start = clock();
	pChrono->Finish = 0;
	pChrono->Duration = 0;
	pChrono->Suspended = FALSE;

	if (pChrono->Start == (clock_t)-1)
	{
		PRINT_DEBUG_ERROR_OSTIME(("StartChrono error (%s) : %s\n", 
			strtime_m(), 
			"Clock error. "));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Suspend a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.
double* pDuration : (INOUT) Duration (in s).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SuspendChrono(CHRONO* pChrono, double* pDuration)
{
	if (!pChrono->Suspended)
	{
		pChrono->Suspended = TRUE;
		pChrono->Finish = clock();

		if (pChrono->Finish == (clock_t)-1)
		{
			PRINT_DEBUG_ERROR_OSTIME(("SuspendChrono error (%s) : %s"
				"(pChrono=%#x)\n", 
				strtime_m(), 
				"Clock error. ", 
				pChrono));
			return EXIT_FAILURE;
		}

		// We need to consider that clock() can wrap around, so first cast in clock_t 
		// the difference. Note there will still be an error of modulo 2^(8*sizeof(clock_t)). 
		// Then we cast in double to be able to divide correctly and get a time in s.
		pChrono->Duration += (double)((clock_t)(pChrono->Finish-pChrono->Start))/(double)CLOCKS_PER_SEC;

		*pDuration = pChrono->Duration;
	}
	else
	{
		PRINT_DEBUG_ERROR_OSTIME(("SuspendChrono error (%s) : %s"
			"(pChrono=%#x)\n", 
			strtime_m(), 
			"Chrono already suspended. ", 
			pChrono));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Suspend a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : Duration (in s).
*/
inline double SuspendChronoQuick(CHRONO* pChrono)
{
	if (!pChrono->Suspended)
	{
		pChrono->Suspended = TRUE;
		pChrono->Finish = clock();

		// We need to consider that clock() can wrap around, so first cast in clock_t 
		// the difference. Note there will still be an error of modulo 2^(8*sizeof(clock_t)). 
		// Then we cast in double to be able to divide correctly and get a time in s.
		pChrono->Duration += (double)((clock_t)(pChrono->Finish-pChrono->Start))/(double)CLOCKS_PER_SEC;

		return pChrono->Duration;
	}
	else
	{
		PRINT_DEBUG_ERROR_OSTIME(("SuspendChronoQuick error (%s) : %s"
			"(pChrono=%#x)\n", 
			strtime_m(), 
			"Chrono already suspended. ", 
			pChrono));
		return -1;
	}
}

/*
Resume a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int ResumeChrono(CHRONO* pChrono)
{
	if (pChrono->Suspended)
	{
		pChrono->Suspended = FALSE;
		pChrono->Start = clock();

		if (pChrono->Start == (clock_t)-1)
		{
			PRINT_DEBUG_ERROR_OSTIME(("ResumeChrono error (%s) : %s"
				"(pChrono=%#x)\n", 
				strtime_m(), 
				"Clock error. ", 
				pChrono));
			return EXIT_FAILURE;
		}
	}
	else
	{
		PRINT_DEBUG_ERROR_OSTIME(("ResumeChrono error (%s) : %s"
			"(pChrono=%#x)\n", 
			strtime_m(), 
			"Chrono not suspended. ", 
			pChrono));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Get the time elapsed of a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.
double* pDuration : (INOUT) Duration (in s).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int GetTimeElapsedChrono(CHRONO* pChrono, double* pDuration)
{
	clock_t temp;

	if (pChrono->Suspended)
	{
		*pDuration = pChrono->Duration;
	}
	else
	{
		temp = clock();

		if (temp == (clock_t)-1)
		{
			PRINT_DEBUG_ERROR_OSTIME(("GetTimeElapsedChrono error (%s) : %s"
				"(pChrono=%#x)\n", 
				strtime_m(), 
				"Clock error. ", 
				pChrono));
			return EXIT_FAILURE;
		}

		// We need to consider that clock() can wrap around, so first cast in clock_t 
		// the difference. Note there will still be an error of modulo 2^(8*sizeof(clock_t)). 
		// Then we cast in double to be able to divide correctly and get a time in s.
		*pDuration = pChrono->Duration+(double)((clock_t)(temp-pChrono->Start))/(double)CLOCKS_PER_SEC;
	}

	return EXIT_SUCCESS;
}

/*
Get the time elapsed of a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : Duration (in s).
*/
inline double GetTimeElapsedChronoQuick(CHRONO* pChrono)
{
	clock_t temp;

	if (pChrono->Suspended)
	{
		return pChrono->Duration;
	}
	else
	{
		temp = clock();

		// We need to consider that clock() can wrap around, so first cast in clock_t 
		// the difference. Note there will still be an error of modulo 2^(8*sizeof(clock_t)). 
		// Then we cast in double to be able to divide correctly and get a time in s.
		return pChrono->Duration+(double)((clock_t)(temp-pChrono->Start))/(double)CLOCKS_PER_SEC;
	}
}

/*
Stop a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.
double* pDuration : (INOUT) Duration (in s).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int StopChrono(CHRONO* pChrono, double* pDuration)
{
	if (pChrono->Suspended)
	{
		*pDuration = pChrono->Duration;
	}
	else
	{
		pChrono->Finish = clock();

		if (pChrono->Finish == (clock_t)-1)
		{
			PRINT_DEBUG_ERROR_OSTIME(("StopChrono error (%s) : %s"
				"(pChrono=%#x)\n", 
				strtime_m(), 
				"Clock error. ", 
				pChrono));
			return EXIT_FAILURE;
		}

		// We need to consider that clock() can wrap around, so first cast in clock_t 
		// the difference. Note there will still be an error of modulo 2^(8*sizeof(clock_t)). 
		// Then we cast in double to be able to divide correctly and get a time in s.
		pChrono->Duration += (double)((clock_t)(pChrono->Finish-pChrono->Start))/(double)CLOCKS_PER_SEC;

		*pDuration = pChrono->Duration;
	}

	return EXIT_SUCCESS;
}

/*
Stop a chronometer.

CHRONO* pChrono : (INOUT) Valid pointer to the structure.

Return : Duration (in s).
*/
inline double StopChronoQuick(CHRONO* pChrono)
{
	if (pChrono->Suspended)
	{
		return pChrono->Duration;
	}
	else
	{
		pChrono->Finish = clock();

		// We need to consider that clock() can wrap around, so first cast in clock_t 
		// the difference. Note there will still be an error of modulo 2^(8*sizeof(clock_t)). 
		// Then we cast in double to be able to divide correctly and get a time in s.
		pChrono->Duration += (double)((clock_t)(pChrono->Finish-pChrono->Start))/(double)CLOCKS_PER_SEC;

		return pChrono->Duration;
	}
}
#endif // ENABLE_GETTIMEOFDAY_WIN32
#endif // USE_OLD_CHRONO

#endif // OSTIME_H
