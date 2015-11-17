/***************************************************************************************************************:')

OSTime.c

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

#include "OSTime.h"

char strftime_m_tmpbuf[32]; // Used to store the string returned by strtime_m().
char strftime_fns_tmpbuf[32]; // Used to store the string returned by strtime_fns().

/*
Return a string like ctime() but in this format :

Mon Aug 27 19:28:04 2007\0

(without the "\n" of ctime()).
Should not be used in concurrent threads as the string value returned might 
be changed by another thread.

Return : This string.
*/
char* strtime_m(void)
{
	time_t t;
	struct tm *timeptr = NULL;

	time(&t);
	timeptr = localtime(&t);

	if (timeptr == NULL)
	{
		return NULL;
	}

	// Use strftime to build a customized time string. 
	if (strftime(
		strftime_m_tmpbuf,
		32,
		"%a %b %d %H:%M:%S %Y",
		timeptr
		) <= 0)
	{
		return NULL;
	}

	return strftime_m_tmpbuf;
}

/*
Return a string like ctime() but in this format :

Mon_Aug_27_19h28min04sec_2007\0

(without the "\n", ":", " " of ctime() in order to be safely used in file names).
Should not be used in concurrent threads as the string value returned might 
be changed by another thread.

Return : This string.
*/
char* strtime_fns(void)	
{
	time_t t;
	struct tm *timeptr = NULL;

	time(&t);
	timeptr = localtime(&t);

	if (timeptr == NULL)
	{
		return NULL;
	}

	// Use strftime to build a customized time string. 
	if (strftime(
		strftime_fns_tmpbuf,
		32,
		"%a_%b_%d_%Hh%Mmin%Ssec_%Y",
		timeptr
		) <= 0)
	{
		return NULL;
	}

	return strftime_fns_tmpbuf;
}
