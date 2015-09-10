/***************************************************************************************************************:')

OSCore.h

Include the main Linux and Windows headers. Some Windows types ported to Linux.
Debug and error handling functions, macros and strings.

Fabrice Le Bars

Created : 2007

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

#ifndef OSCORE_H
#define OSCORE_H

/*

Predefined macros depending on the compiler : 

_ Visual Studio : _MSC_VER

_ Borland C++ Builder : __BORLANDC__	

_ GCC : __GNUC__

Predifined macros depending on the target operating system : 

_ 32 bit Windows : _WIN32 & !_WIN64

_ 32 and 64 bit Windows : _WIN32

_ 64 bit Windows : _WIN32 & _WIN64

_ 32 bit Linux : __linux__ & !_LP64

_ 32 and 64 bit Linux : __linux__

_ 64 bit Linux : __linux__ & _LP64

*/

#ifdef _WIN32
// Set the default target Windows version to Windows XP, unless we use Borland C++ Builder 5
// for which it is set to Windows 2000.
#	ifdef __BORLANDC__
#		ifndef _WIN32_WINNT
#			define _WIN32_WINNT 0x0500
#		endif // _WIN32_WINNT
#	else
#		ifndef _WIN32_WINNT
#			define _WIN32_WINNT 0x0501
#		endif // _WIN32_WINNT
#	endif // __BORLANDC__
#endif // _WIN32

#ifdef _MSC_VER
// Disable some Visual Studio warnings.
#	ifndef CRT_SECURE_NO_DEPRECATE
#		define CRT_SECURE_NO_DEPRECATE
#	endif // CRT_SECURE_NO_DEPRECATE
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS
#	endif // _CRT_SECURE_NO_WARNINGS
//#	ifndef _CRT_NONSTDC_NO_WARNINGS
//#		define _CRT_NONSTDC_NO_WARNINGS
//#	endif // _CRT_NONSTDC_NO_WARNINGS
// To avoid compiler warnings about functions compiled as native code (if CLR).
#	if defined(__cplusplus) && defined(_M_CEE)
#		pragma warning(disable : 4793)
#	endif // defined(__cplusplus) && defined(_M_CEE)
// Enable additional features in math.h.
#	ifndef _USE_MATH_DEFINES
#		define _USE_MATH_DEFINES
#	endif // _USE_MATH_DEFINES
#	if defined(_DEBUG) && !defined(DISABLE_ADDITIONAL_DEBUG_FEATURES)
// Enable additional debug features for malloc functions.
#		ifndef _CRTDBG_MAP_ALLOC
#			define _CRTDBG_MAP_ALLOC
#		endif // _CRTDBG_MAP_ALLOC
#		include <stdlib.h>
#		include <crtdbg.h>
#	endif // defined(_DEBUG) && !defined(DISABLE_ADDITIONAL_DEBUG_FEATURES)
#endif // _MSC_VER

#ifdef __GNUC__
// Enable GNU extensions.
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif // _GNU_SOURCE
// Reentrant versions of several functions get declared in libc.
#	ifndef _REENTRANT
#		define _REENTRANT
#	endif // _REENTRANT
// Thread-safe versions of several functions get declared in libc.
#	ifndef _THREAD_SAFE
#		define _THREAD_SAFE
#	endif // _THREAD_SAFE
#endif // __GNUC__

// C Standard Library headers.
#include <assert.h> // Diagnostics.
#include <ctype.h> // Character Class Tests.
#include <errno.h> // Error Codes Reported by (Some) Library Functions.
#include <float.h> // Implementation-defined Floating-Point Limits.
#include <limits.h> // Implementation-defined Limits.
#include <locale.h> // Locale-specific Information.
#include <math.h> // Mathematical Functions.
#include <setjmp.h> // Non-local Jumps.
#include <signal.h> // Signals.
#include <stdarg.h> // Variable Argument Lists.
#include <stddef.h> // Definitions of General Use.
#include <stdio.h> // Input and Output.
#include <stdlib.h> // Utility functions.
#include <string.h> // String functions.
#include <time.h> // Time and Date functions.

#ifdef __GNUC__
// C99 headers. Some headers are not supported by all the compilers or depends 
// on its options.
//#include <complex.h> // Complex arithmetic.
//#include <fenv.h> // IEEE-style floating-point arithmetic.
//#include <inttypes.h> // Integer types.
//#include <iso646.h> // ISO 646 variant character sets.
//#include <stdbool.h> // Boolean type name and constants.
#include <stdint.h> // Integer types with size constraints.
//#include <tgmath.h> // Type-generic math functions.
//#include <wchar.h> // Wide streams and several kinds of strings.
//#include <wctype.h> // Wide characters.
#endif // __GNUC__

#include <fcntl.h>

#ifdef _WIN32
// Prevent Winsock.h from being included by the Windows.h header.
// This must be done if we plan to include Winsock2.h in other files.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>
#else 
#include <unistd.h>
#include <sys/time.h>
#endif // _WIN32

#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern 
#endif // __cplusplus
#endif // EXTERN_C

#ifndef __cplusplus
#ifdef _MSC_VER
#define inline __inline
#endif // _MSC_VER
#ifdef __BORLANDC__
#define inline __inline
#endif // __BORLANDC__
#ifdef __GNUC__
// extern __inline__ in ws2tcpip.h for GNU?
#define inline static __inline__
#endif // __GNUC__
#endif // __cplusplus

#ifndef _WIN32
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif // max
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif // min
typedef char				CHAR;
typedef unsigned char		UCHAR;
typedef short				SHORT;
typedef unsigned short		USHORT;
typedef int					INT;
typedef unsigned int		UINT;
typedef long				LONG;
typedef unsigned long		ULONG;
typedef float				FLOAT;
#endif // _WIN32
typedef double				DOUBLE;

#ifdef __GNUC__
#undef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) (void)(P)
#endif // __GNUC__

#ifdef __BORLANDC__
#undef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) 
#endif // __BORLANDC__

#ifndef _WIN32
#ifndef ZeroMemory
#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))
#endif // ZeroMemory
typedef void* HANDLE;
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)-1)
#endif // INVALID_HANDLE_VALUE
typedef int                 BOOL;
#ifndef FALSE
#define FALSE               0
#endif // FALSE
#ifndef TRUE
#define TRUE                1
#endif // TRUE
typedef unsigned int		u_int;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef BYTE                BOOLEAN;           
#endif // _WIN32

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32; // Might be unsigned long...

// Conflict with OpenCV...
#ifdef ENABLE_INT64_TYPEDEF
// Note that the long long type is not accepted by Borland C++ Builder 5.
#ifdef __GNUC__
typedef int64_t int64;
typedef uint64_t uint64;
#else
typedef __int64 int64;
typedef unsigned __int64 uint64;
#endif // __GNUC__
#endif // ENABLE_INT64_TYPEDEF

union uShort
{
	uint16 v;  
	uint8 c[2];
};
typedef union uShort uShort;

union uLong
{
	long v;  
	uint8 c[4];
};
typedef union uLong uLong;

union uFloat
{
	float v;  
	uint8 c[4];
};
typedef union uFloat uFloat;

#ifndef _WIN32
typedef union _LARGE_INTEGER {
	struct {
		unsigned long LowPart;
		long HighPart;
	};
	struct {
		unsigned long LowPart;
		long HighPart;
	} u;
	long long QuadPart;
} LARGE_INTEGER;

typedef union _ULARGE_INTEGER {
	struct {
		unsigned long LowPart;
		unsigned long HighPart;
	};
	struct {
		unsigned long LowPart;
		unsigned long HighPart;
	} u;
	unsigned long long QuadPart;
} ULARGE_INTEGER;
#endif // _WIN32

/*
Structure corresponding to a color in a RGB format (red, green and blue
levels).
*/
struct RGBCOLOR
{
	UCHAR r; // Red value.
	UCHAR g; // Green value.
	UCHAR b; // Blue value.
};
typedef struct RGBCOLOR RGBCOLOR;

// If you use Visual Studio in debug configuration, call INIT_DEBUG at the
// beginning of the program to enable memory leaks detection.
// If you do not use Visual Studio, INIT_DEBUG will be ignored.
#if defined(_MSC_VER) && defined(_DEBUG) && !defined(DISABLE_ADDITIONAL_DEBUG_FEATURES)
#	ifndef INIT_DEBUG
#		define INIT_DEBUG _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#	endif // INIT_DEBUG
#else
#	ifndef INIT_DEBUG
#		define INIT_DEBUG
#	endif // INIT_DEBUG
#endif // defined(_MSC_VER) && defined(_DEBUG) && !defined(DISABLE_ADDITIONAL_DEBUG_FEATURES)

/*
enum EXIT_CODE
{

}
*/

// Return values of functions.
// The return value for a Linux program should be between 0 and 127.
// For Windows, at least STILL_ACTIVE (259) should not be used.
#define EXIT_TIMEOUT 2
#define EXIT_INVALID_PARAMETER 3
#define EXIT_INVALID_DATA 4
#define EXIT_NAME_TOO_LONG 5
#define EXIT_OUT_OF_MEMORY 6
#define EXIT_OBJECT_NONSIGNALED 7
#define EXIT_KILLED_THREAD 8
#define EXIT_CANCELED_THREAD 9
#define EXIT_IO_PENDING 10
#define EXIT_KILLED_PROCESS 11
#define EXIT_CHANGED 12
#define EXIT_NOT_CHANGED 13
#define EXIT_FOUND 14
#define EXIT_NOT_FOUND 15
#define EXIT_NOT_IMPLEMENTED 16

// Strings corresponding to the previous return values.
EXTERN_C const char* szOSUtilsErrMsgs[];

/*
Format a message corresponding to the last error in a system call (thread-
safe).

char* buf : (INOUT) Valid pointer to a buffer that will receive the message.
int buflen : (IN) Size of the buffer in bytes.

Return : buf.
*/
EXTERN_C char* FormatLastErrorMsg(char* buf, int buflen);

#define LAST_ERROR_MSG_SIZE 256

// Specific macro used with PRINT_DEBUG_MESSAGE, PRINT_DEBUG_WARNING 
// or PRINT_DEBUG_ERROR to return the message corresponding to the last error 
// in a system call in the current thread.
#define GetLastErrorMsg() FormatLastErrorMsg(szLastErrMsg, LAST_ERROR_MSG_SIZE)

// For Windows, debug is activated if _DEBUG is defined. 
// In this case, default is to only activate errors for OSUtils.
#if defined(_DEBUG) && !defined(_DEBUG_MESSAGES) && !defined(_DEBUG_WARNINGS) && !defined(_DEBUG_ERRORS)
#define _DEBUG_ERRORS
#endif // defined(_DEBUG) && !defined(_DEBUG_MESSAGES) && !defined(_DEBUG_WARNINGS) && !defined(_DEBUG_ERRORS)

// If _DEBUG_MESSAGES is defined, messages, warnings and errors are activated.
// If _DEBUG_WARNINGS is defined, only warnings and errors are activated.
// If _DEBUG_ERRORS is defined, only errors are activated.
#ifdef _DEBUG_MESSAGES
#	define _DEBUG_WARNINGS
#	define _DEBUG_ERRORS
#else
#	ifdef _DEBUG_WARNINGS
#		define _DEBUG_ERRORS
#	endif // _DEBUG_WARNINGS
#endif // _DEBUG_MESSAGES

#if defined(_DEBUG_MESSAGES) || defined(_DEBUG_WARNINGS) || defined(_DEBUG_ERRORS)

// Default is to print on stdout/err
#if !defined(_DEBUG_DISPLAY) && !defined(_DEBUG_FILE)
#define _DEBUG_DISPLAY
#endif // !defined(_DEBUG_DISPLAY) && !defined(_DEBUG_FILE)

#endif // defined(_DEBUG_MESSAGES) || defined(_DEBUG_WARNINGS) || defined(_DEBUG_ERRORS) 

#ifdef _DEBUG_DISPLAY
#	define PRINT_DEBUG_MESSAGE(params) {char szLastErrMsg[LAST_ERROR_MSG_SIZE];szLastErrMsg[LAST_ERROR_MSG_SIZE-1]=0;fprintf_stdout params;}
#	define PRINT_DEBUG_WARNING(params) {char szLastErrMsg[LAST_ERROR_MSG_SIZE];szLastErrMsg[LAST_ERROR_MSG_SIZE-1]=0;fprintf_stdout params;}
#	define PRINT_DEBUG_ERROR(params) {char szLastErrMsg[LAST_ERROR_MSG_SIZE];szLastErrMsg[LAST_ERROR_MSG_SIZE-1]=0;fprintf_stderr params;}
#else
#	ifdef _DEBUG_FILE
#		define PRINT_DEBUG_MESSAGE(params) {char szLastErrMsg[LAST_ERROR_MSG_SIZE];szLastErrMsg[LAST_ERROR_MSG_SIZE-1]=0;fprintf_messages_file params;}
#		define PRINT_DEBUG_WARNING(params) {char szLastErrMsg[LAST_ERROR_MSG_SIZE];szLastErrMsg[LAST_ERROR_MSG_SIZE-1]=0;fprintf_warnings_file params;}
#		define PRINT_DEBUG_ERROR(params) {char szLastErrMsg[LAST_ERROR_MSG_SIZE];szLastErrMsg[LAST_ERROR_MSG_SIZE-1]=0;fprintf_errors_file params;}
#	else
#		define PRINT_DEBUG_MESSAGE(params)
#		define PRINT_DEBUG_WARNING(params)
#		define PRINT_DEBUG_ERROR(params)
#	endif // _DEBUG_FILE
#endif // _DEBUG_DISPLAY

#ifdef _DEBUG_DISPLAY
EXTERN_C void fprintf_stdout(const char * _Format, ...);
EXTERN_C void fprintf_stderr(const char * _Format, ...);
#endif // _DEBUG_DISPLAY

#ifdef _DEBUG_FILE
#ifdef USE_DIFFERENT_FILES_FOR_ERRORS_WARNINGS_AND_MESSAGES
#define MESSAGES_FILENAME "messages.txt"
#define WARNINGS_FILENAME "warnings.txt"
#define ERRORS_FILENAME "errors.txt"
#else
#define MESSAGES_FILENAME "log.txt"
#define WARNINGS_FILENAME "log.txt"
#define ERRORS_FILENAME "log.txt"
#endif // USE_DIFFERENT_FILES_FOR_ERRORS_WARNINGS_AND_MESSAGES

EXTERN_C void fprintf_messages_file(const char * _Format, ...);
EXTERN_C void fprintf_warnings_file(const char * _Format, ...);
EXTERN_C void fprintf_errors_file(const char * _Format, ...);
#endif // _DEBUG_FILE

/*
Debug macros specific to OSUtils.
*/
#ifdef _DEBUG_MESSAGES
#	define _DEBUG_MESSAGES_OSUTILS
#endif // _DEBUG_MESSAGES

#ifdef _DEBUG_WARNINGS
#	define _DEBUG_WARNINGS_OSUTILS
#endif // _DEBUG_WARNINGS

#ifdef _DEBUG_ERRORS
#	define _DEBUG_ERRORS_OSUTILS
#endif // _DEBUG_ERRORS

#ifdef _DEBUG_MESSAGES_OSUTILS
#	define PRINT_DEBUG_MESSAGE_OSUTILS(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSUTILS(params)
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define PRINT_DEBUG_WARNING_OSUTILS(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSUTILS(params)
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define PRINT_DEBUG_ERROR_OSUTILS(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSUTILS(params)
#endif // _DEBUG_ERRORS_OSUTILS

/*
Debug macros specific to OSCore.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSCORE
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSCORE
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSCORE
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSCORE
#	define PRINT_DEBUG_MESSAGE_OSCORE(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSCORE(params)
#endif // _DEBUG_MESSAGES_OSCORE

#ifdef _DEBUG_WARNINGS_OSCORE
#	define PRINT_DEBUG_WARNING_OSCORE(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSCORE(params)
#endif // _DEBUG_WARNINGS_OSCORE

#ifdef _DEBUG_ERRORS_OSCORE
#	define PRINT_DEBUG_ERROR_OSCORE(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSCORE(params)
#endif // _DEBUG_ERRORS_OSCORE

#endif // OSCORE_H
