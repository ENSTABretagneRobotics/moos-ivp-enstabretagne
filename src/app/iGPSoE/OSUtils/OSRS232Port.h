/***************************************************************************************************************:')

OSRS232Port.h

RS232 port handling (open, close, read, write,...).

Fabrice Le Bars

Created : 2009-03-28

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

#ifndef OSRS232PORT_H
#define OSRS232PORT_H

#include "OSTime.h"

/*
Debug macros specific to OSRS232Port.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSRS232PORT
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSRS232PORT
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSRS232PORT
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSRS232PORT
#	define PRINT_DEBUG_MESSAGE_OSRS232PORT(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSRS232PORT(params)
#endif // _DEBUG_MESSAGES_OSRS232PORT

#ifdef _DEBUG_WARNINGS_OSRS232PORT
#	define PRINT_DEBUG_WARNING_OSRS232PORT(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSRS232PORT(params)
#endif // _DEBUG_WARNINGS_OSRS232PORT

#ifdef _DEBUG_ERRORS_OSRS232PORT
#	define PRINT_DEBUG_ERROR_OSRS232PORT(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSRS232PORT(params)
#endif // _DEBUG_ERRORS_OSRS232PORT

#ifdef _WIN32
#else 
#include <termios.h>
//#include <sys/ioctl.h>
#endif // _WIN32

#ifndef _WIN32
#define NOPARITY            0
#define ODDPARITY           1
#define EVENPARITY          2
#define MARKPARITY          3
#define SPACEPARITY         4

#define ONESTOPBIT          0
#define TWOSTOPBITS         2
#endif // _WIN32

#define MAX_TIMEOUT_RS232PORT 25500

inline UINT _BaudRate2Constant(UINT BaudRate)
{
#ifdef _WIN32
	switch (BaudRate)
	{
	case 110:
		return CBR_110;
	case 300:
		return CBR_300;
	case 600:
		return CBR_600;
	case 1200:
		return CBR_1200;
	case 4800:
		return CBR_4800;
	case 2400:
		return CBR_2400;
	case 9600:
		return CBR_9600;
	case 19200:
		return CBR_19200;
	case 38400:
		return CBR_38400;
	case 57600:
		return CBR_57600;
	case 115200:
		return CBR_115200;
	default:
		return 0;
	}
#else 
	switch (BaudRate)
	{
	case 110:
		return B110;
	case 300:
		return B300;
	case 600:
		return B600;
	case 1200:
		return B1200;
	case 2400:
		return B2400;
	case 4800:
		return B4800;
	case 9600:
		return B9600;
	case 19200:
		return B19200;
	case 38400:
		return B38400;
	case 57600:
		return B57600;
	case 115200:
		return B115200;
	default:
		return 0;
	}
#endif // _WIN32
}

inline UINT _Constant2BaudRate(UINT Constant)
{
#ifdef _WIN32
	switch (Constant)
	{
	case CBR_110:
		return 110;
	case CBR_300:
		return 300;
	case CBR_600:
		return 600;
	case CBR_1200:
		return 1200;
	case CBR_4800:
		return 4800;
	case CBR_2400:
		return 2400;
	case CBR_9600:
		return 9600;
	case CBR_19200:
		return 19200;
	case CBR_38400:
		return 38400;
	case CBR_57600:
		return 57600;
	case CBR_115200:
		return 115200;
	default:
		return 0;
	}
#else 
	switch (Constant)
	{
	case B110:
		return 110;
	case B300:
		return 300;
	case B600:
		return 600;
	case B1200:
		return 1200;
	case B2400:
		return 2400;
	case B4800:
		return 4800;
	case B9600:
		return 9600;
	case B19200:
		return 19200;
	case B38400:
		return 38400;
	case B57600:
		return 57600;
	case B115200:
		return 115200;
	default:
		return 0;
	}
#endif // _WIN32
}

/*
Open a serial port.

HANDLE* phDev : (INOUT) Valid pointer that will receive an identifier of the
serial port opened.
char* szDevice : (IN) Serial port to open.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int OpenRS232Port(HANDLE* phDev, char* szDevice)
{
#ifdef _WIN32
	char szDeviceTemp[256];

	// To be able to use COM10 and greater we need to add "\\.\" (that becomes "\\\\.\\" 
	// in C because the '\' is a special character).
	sprintf(szDeviceTemp, "\\\\.\\%s", szDevice);

	*phDev = CreateFile( 
		szDeviceTemp,
		GENERIC_READ | GENERIC_WRITE,
		0,    // Must be opened with exclusive-access.
		NULL, // No security attributes.
		OPEN_EXISTING, // Must use OPEN_EXISTING.
		0,    // Not overlapped I/O.
		NULL  // hTemplate must be NULL for comm devices.
		);

	if (*phDev == INVALID_HANDLE_VALUE)
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("OpenRS232Port error (%s) : %s"
			"(szDevice=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			szDevice));
		return EXIT_FAILURE;
	}
#else 
	// The O_NOCTTY flag tells UNIX that this program does not want to be the
	// "controlling terminal" for that port. If you don't specify this then any input (such
	// as keyboard abort signals and so forth) will affect your process. Programs like
	// getty(1M/8) use this feature when starting the login process, but normally a
	// user program does not want this behavior.
	// The O_NDELAY flag tells UNIX that this program does not care what state the
	// DCD signal line is in - whether the other end of the port is up and running. If
	// you do not specify this flag, your process will be put to sleep until the DCD
	// signal line is the space voltage.
	int fd = open(szDevice, O_RDWR | O_NOCTTY| O_NDELAY);

	if (fd == -1)
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("OpenRS232Port error (%s) : %s"
			"(szDevice=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			szDevice));
		return EXIT_FAILURE;
	}

	// The read function can be made to return immediately by
	// doing the following :
	// fcntl(fd, F_SETFL, FNDELAY);
	// The FNDELAY option causes the read function to return 0 if no characters are
	// available on the port. To restore normal (blocking) behavior, call fcntl() without
	// the FNDELAY option:
	if (fcntl(fd, F_SETFL, 0) == (-1))
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("OpenRS232Port error (%s) : %s"
			"(szDevice=%s)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			szDevice));
		close(fd);
		return EXIT_FAILURE;
	}

	*phDev = (HANDLE)fd;
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Set the options of a serial port.

HANDLE hDev : (IN) Identifier of the serial port.
UINT BaudRate : (IN) Baud rate at which the device connected to the serial port operates.
BYTE ParityMode : (IN) Parity mode. Should be either NOPARITY, ODDPARITY, EVENPARITY, MARKPARITY or SPACEPARITY.
BOOL bCheckParity : (IN) If TRUE, enable input parity checking.
BYTE nbDataBits : (IN) Number of bits of the data bytes.
BYTE StopBitsMode : (IN) Stop bits mode. Should be either ONESTOPBIT or TWOSTOPBITS.
UINT timeout : (IN) Time to wait to get at least 1 byte in ms (near 1000 ms for example, max is 
MAX_TIMEOUT_RS232PORT).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SetOptionsRS232Port(HANDLE hDev, UINT BaudRate, BYTE ParityMode, BOOL bCheckParity, BYTE nbDataBits, 
							   BYTE StopBitsMode, UINT timeout)
{
#ifdef _WIN32
	DCB dcb;
	COMMTIMEOUTS timeouts;

	memset(&dcb, 0, sizeof(DCB));

	if (!GetCommState(hDev, &dcb))
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("SetOptionsRS232Port error (%s) : %s"
			"(hDev=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		return EXIT_FAILURE;
	}

	memset(&timeouts, 0, sizeof(COMMTIMEOUTS));

	if (!GetCommTimeouts(hDev, &timeouts))
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("SetOptionsRS232Port error (%s) : %s"
			"(hDev=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		return EXIT_FAILURE;
	}

	// Binary mode.
	dcb.fBinary = TRUE;
	// No flow control.
	dcb.fOutxCtsFlow = FALSE;			
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fDsrSensitivity = FALSE;		 
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fAbortOnError = FALSE;

	dcb.BaudRate = _BaudRate2Constant(BaudRate);
	if (dcb.BaudRate == 0)
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("SetOptionsRS232Port error (%s) : %s"
			"(hDev=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			"Invalid BaudRate. ", 
			hDev, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		return EXIT_FAILURE;
	}

	// An even parity bit will be set to "1" if the number of 1's + 1 is even, and an odd 
	// parity bit will be set to "1" if the number of 1's +1 is odd.
	// If the parity bit is present but not used, it may be referred to as mark parity 
	// (when the parity bit is always 1) or space parity (the bit is always 0).
	// None parity means that no parity bit is sent at all. 
	dcb.Parity = ParityMode;
	dcb.fParity = bCheckParity;
	dcb.fErrorChar = FALSE; // Indicates whether bytes received with parity errors are 
	// replaced with the character specified by the ErrorChar member. 
	dcb.ByteSize = nbDataBits;
	dcb.StopBits = StopBitsMode;

	// The SetCommState() function reconfigures the communications resource, but it does not affect
	// the internal output and input buffers of the specified driver. The buffers are not flushed, 
	// and pending read and write operations are not terminated prematurely.
	if (!SetCommState(hDev, &dcb))
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("SetOptionsRS232Port error (%s) : %s"
			"(hDev=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		return EXIT_FAILURE;
	}

	// Special mode for read timeouts.
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutConstant = timeout;
	timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
	//timeouts.WriteTotalTimeoutConstant = 0; // Linux do not seem to have options for write timeouts, so disable for Windows...
	timeouts.WriteTotalTimeoutConstant = timeout;
	timeouts.WriteTotalTimeoutMultiplier = 0;

	if (!SetCommTimeouts(hDev, &timeouts))
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("SetOptionsRS232Port error (%s) : %s"
			"(hDev=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		return EXIT_FAILURE;
	}
#else 
	struct termios options;
	speed_t speed = 0;

	memset(&options, 0, sizeof(options));

	if (tcgetattr((int)hDev, &options) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("SetOptionsRS232Port error (%s) : %s"
			"(hDev=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		return EXIT_FAILURE;
	}

	// The c_cflag member contains two options that should always be enabled,
	// CLOCAL and CREAD. These will ensure that your program does not become
	// the 'owner' of the port subject to sporatic job control and hangup signals, and
	// also that the serial interface driver will read incoming data bytes.
	options.c_cflag |= (CLOCAL | CREAD);

	// Raw input, no echo, no signals.
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	// Raw output.
	options.c_oflag &= ~OPOST;

	// Disable hardware flow control.
	options.c_cflag &= ~CRTSCTS;

	// Disable software flow control.
	options.c_iflag &= ~(IXON | IXOFF | IXANY);

	speed = _BaudRate2Constant(BaudRate);
	if (speed == 0)
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("SetOptionsRS232Port error (%s) : %s"
			"(hDev=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			"Invalid BaudRate. ", 
			hDev, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		return EXIT_FAILURE;
	}

	if (cfsetospeed(&options, speed) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("SetOptionsRS232Port error (%s) : %s"
			"(hDev=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		return EXIT_FAILURE;
	}

	if (cfsetispeed(&options, speed) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("SetOptionsRS232Port error (%s) : %s"
			"(hDev=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		return EXIT_FAILURE;
	}

	// An even parity bit will be set to "1" if the number of 1's + 1 is even, and an odd 
	// parity bit will be set to "1" if the number of 1's +1 is odd.
	// If the parity bit is present but not used, it may be referred to as mark parity 
	// (when the parity bit is always 1) or space parity (the bit is always 0).
	// None parity means that no parity bit is sent at all. 
	switch (ParityMode)
	{
	case NOPARITY :
		options.c_cflag &= ~CMSPAR;
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~PARODD;
		break;
	case MARKPARITY :  
		options.c_cflag |= CMSPAR;
		options.c_cflag &= ~PARENB;
		options.c_cflag |= PARODD;
		break;
	case SPACEPARITY :  
		options.c_cflag |= CMSPAR;
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~PARODD;
		break;
	case ODDPARITY :    
		options.c_cflag &= ~CMSPAR;
		options.c_cflag |= PARENB;
		options.c_cflag |= PARODD;
		break;
	case EVENPARITY :  
		options.c_cflag &= ~CMSPAR;
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		break;
	default :
		PRINT_DEBUG_ERROR_OSRS232PORT(("SetOptionsRS232Port error (%s) : %s"
			"(hDev=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			"Invalid parity mode. ", 
			hDev, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		return EXIT_FAILURE;
	}

	// The most likely scenario where this flag is useful is if the communication channel is 
	// configured for parity and seven bits per character. In this case, the eigth bit on every 
	// received character is a parity bit, not part of the data payload. The user program does 
	// not need to know the value of the parity bit.
	options.c_iflag &= ~ISTRIP;

	options.c_iflag &= ~PARMRK; // Never mark a framing or parity error with prefix bytes.
	options.c_iflag |= IGNPAR; // A character with a framing or parity error will be discarded.
	// This is only valid (at least for parity errors) if parity checking is enabled.

	if (bCheckParity)
	{
		options.c_iflag |= INPCK;
	}
	else
	{
		options.c_iflag &= ~INPCK;
	}

	options.c_cflag &= ~CSIZE; // Erase the previous flag for the number of data bits.

	switch (nbDataBits)
	{
	case 8:
		options.c_cflag |= CS8;
		break;
	case 7:
		options.c_cflag |= CS7;
		break;
	case 6:
		options.c_cflag |= CS6;
		break;
	case 5:
		options.c_cflag |= CS5;
		break;
	default :
		PRINT_DEBUG_ERROR_OSRS232PORT(("SetOptionsRS232Port error (%s) : %s"
			"(hDev=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			"Invalid number of data bits. ", 
			hDev, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		return EXIT_FAILURE;
	}

	switch (StopBitsMode)
	{
	case ONESTOPBIT:
		options.c_cflag &= ~CSTOPB;
		break;
	case TWOSTOPBITS:
		options.c_cflag |= CSTOPB;
		break;
	default :
		PRINT_DEBUG_ERROR_OSRS232PORT(("SetOptionsRS232Port error (%s) : %s"
			"(hDev=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			"Invalid stop bits mode. ", 
			hDev, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		return EXIT_FAILURE;
	}

	if (timeout/100 > 255)
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("SetOptionsRS232Port error (%s) : %s"
			"(hDev=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			"Too high timeout value. ", 
			hDev, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		return EXIT_FAILURE;
	}

	// Timeouts.
	options.c_cc[VMIN] = 0; // Minimum number of characters to read. 
	options.c_cc[VTIME] = timeout/100; // Time to wait for every character read in tenths of seconds.

	if (tcsetattr((int)hDev, TCSADRAIN, &options) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("SetOptionsRS232Port error (%s) : %s"
			"(hDev=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Get the options of a serial port.

HANDLE hDev : (IN) Identifier of the serial port.
UINT* pBaudRate : (INOUT) Valid pointer that will receive the baud rate at
which the serial port operates.
BYTE* pParityMode : (INOUT) Valid pointer that will receive the serial port
parity mode. It will be either NOPARITY, ODDPARITY, EVENPARITY, MARKPARITY or
SPACEPARITY.
BOOL* pbCheckParity : (INOUT) Valid pointer that will receive TRUE if input 
parity checking is enabled, FALSE otherwise.
BYTE* pNbDataBits : (INOUT) Valid pointer that will receive the number of bits
of the data bytes.
BYTE* pStopBitsMode : (INOUT) Valid pointer that will receive the serial port
stop bits mode. It will be either ONESTOPBIT or TWOSTOPBITS.
UINT* pTimeout : (INOUT) Valid pointer that will receive the time to wait for 
every character read/write in ms.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int GetOptionsRS232Port(HANDLE hDev, UINT* pBaudRate, BYTE* pParityMode, BOOL* pbCheckParity, BYTE* pNbDataBits, 
							   BYTE* pStopBitsMode, UINT* pTimeout)
{
#ifdef _WIN32
	DCB dcb;
	COMMTIMEOUTS timeouts;

	memset(&dcb, 0, sizeof(DCB));

	if (!GetCommState(hDev, &dcb))
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("GetOptionsRS232Port error (%s) : %s(hDev=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev));
		return EXIT_FAILURE;
	}

	memset(&timeouts, 0, sizeof(COMMTIMEOUTS));

	if (!GetCommTimeouts(hDev, &timeouts))
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("GetOptionsRS232Port error (%s) : %s(hDev=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev));
		return EXIT_FAILURE;
	}

	*pBaudRate = _Constant2BaudRate(dcb.BaudRate); 
	if (*pBaudRate == 0)
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("GetOptionsRS232Port error (%s) : %s(hDev=%#x)\n", 
			strtime_m(), 
			"Invalid BaudRate. ", 
			hDev));
		return EXIT_FAILURE;
	}

	*pParityMode = dcb.Parity;
	*pbCheckParity = dcb.fParity?TRUE:FALSE;
	*pNbDataBits = dcb.ByteSize;
	*pStopBitsMode = dcb.StopBits;
	*pTimeout = timeouts.ReadTotalTimeoutConstant;
#else 
	struct termios options;

	memset(&options, 0, sizeof(options));

	if (tcgetattr((int)hDev, &options) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("GetOptionsRS232Port error (%s) : %s(hDev=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev));
		return EXIT_FAILURE;
	}

	*pBaudRate = _Constant2BaudRate(cfgetispeed(&options)); 
	if (*pBaudRate == 0)
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("GetOptionsRS232Port error (%s) : %s(hDev=%#x)\n", 
			strtime_m(), 
			"Invalid BaudRate. ", 
			hDev));
		return EXIT_FAILURE;
	}

	switch (options.c_cflag & (CMSPAR | PARENB | PARODD))
	{
	case 0:
		*pParityMode = NOPARITY;
		break;
	case (CMSPAR | PARODD):
		*pParityMode = MARKPARITY;
		break;
	case (CMSPAR):
		*pParityMode = SPACEPARITY;
		break;
	case (PARENB | PARODD):
		*pParityMode = ODDPARITY;
		break;
	case (PARENB):
		*pParityMode = EVENPARITY;
		break;
	default:
		PRINT_DEBUG_ERROR_OSRS232PORT(("GetOptionsRS232Port error (%s) : %s(hDev=%#x)\n", 
			strtime_m(), 
			"Invalid parity mode. ", 
			hDev));
		return EXIT_FAILURE;
	}

	*pbCheckParity = (options.c_iflag & INPCK)?TRUE:FALSE;

	switch (options.c_cflag & CSIZE)
	{
	case CS8:
		*pNbDataBits = 8;
		break;
	case CS7:
		*pNbDataBits = 7;
		break;
	case CS6:
		*pNbDataBits = 6;
		break;
	case CS5:
		*pNbDataBits = 5;
		break;
	default:
		PRINT_DEBUG_ERROR_OSRS232PORT(("GetOptionsRS232Port error (%s) : %s(hDev=%#x)\n", 
			strtime_m(), 
			"Invalid number of data bits. ", 
			hDev));
		return EXIT_FAILURE;
	}

	*pStopBitsMode = (options.c_cflag & CSTOPB)?TWOSTOPBITS:ONESTOPBIT;

	*pTimeout = options.c_cc[VTIME]*100;
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Discard non-transmitted output data and non-read input data on a serial port.

HANDLE hDev : (IN) Identifier of the serial port.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int PurgeRS232Port(HANDLE hDev)
{
#ifdef _WIN32
	if (!PurgeComm(hDev, PURGE_RXCLEAR | PURGE_TXCLEAR))
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("PurgeRS232Port error (%s) : %s(hDev=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev));
		return EXIT_FAILURE;
	}
#else 
	if (tcflush((int)hDev, TCIOFLUSH) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("PurgeRS232Port error (%s) : %s(hDev=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

#ifdef ENABLE_DRAINRS232PORT
/*
Empty the OS internal output buffer and wait until all output written to the serial 
port has been transmitted (synchronous operation subject to flow control). Will not 
return until all pending write operations have been transmitted.

HANDLE hDev : (IN) Identifier of the serial port.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int DrainRS232Port(HANDLE hDev)
{
#ifdef _WIN32
	if (!FlushFileBuffers(hDev))
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("DrainRS232Port error (%s) : %s(hDev=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev));
		return EXIT_FAILURE;
	}
#else 
	if (tcdrain((int)hDev) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("DrainRS232Port error (%s) : %s(hDev=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}
#endif // ENABLE_DRAINRS232PORT

/*
Read data from a serial port. 

HANDLE hDev : (IN) Identifier of the serial port.
uint8* readbuf : (INOUT) Valid pointer that will receive the data read.
UINT readbuflen : (IN) Number of bytes to read.
int* pReadBytes : (INOUT) Valid pointer that will receive the number of bytes read.

Return : EXIT_SUCCESS if some bytes are read, EXIT_TIMEOUT if a timeout occurs or 
EXIT_FAILURE if there is an error.
*/
inline int ReadRS232Port(HANDLE hDev, uint8* readbuf, UINT readbuflen, int* pReadBytes)
{
#ifdef _DEBUG_MESSAGES_OSRS232PORT
	int i = 0;
#endif // _DEBUG_MESSAGES_OSRS232PORT

#ifdef _WIN32
	if (ReadFile(hDev, readbuf, readbuflen, (LPDWORD)pReadBytes, NULL))
	{
#else 
	*pReadBytes = read((int)hDev, readbuf, readbuflen);
	if (*pReadBytes >= 0)
	{
#endif // _WIN32
		if (*pReadBytes == 0)
		{
			PRINT_DEBUG_WARNING_OSRS232PORT(("ReadRS232Port warning (%s) : %s"
				"(hDev=%#x, readbuf=%#x, readbuflen=%u)\n", 
				strtime_m(), 
				szOSUtilsErrMsgs[EXIT_TIMEOUT], 
				hDev, readbuf, readbuflen));
			return EXIT_TIMEOUT;
		}
		else
		{
#ifdef _DEBUG_MESSAGES_OSRS232PORT
			for (i = 0; i < *pReadBytes; i++)
			{
				PRINT_DEBUG_MESSAGE_OSRS232PORT(("%.2x ", (int)readbuf[i]));
			}
			PRINT_DEBUG_MESSAGE_OSRS232PORT(("\n"));
#endif // _DEBUG_MESSAGES_OSRS232PORT
			PRINT_DEBUG_MESSAGE_OSRS232PORT(("Bytes read : %d\n", *pReadBytes));
		}
	}
	else
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("ReadRS232Port error (%s) : %s"
			"(hDev=%#x, readbuf=%#x, readbuflen=%u)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev, readbuf, readbuflen));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Write data to a serial port.

HANDLE hDev : (IN) Identifier of the serial port.
uint8* writebuf : (IN) Valid pointer to the data to write.
UINT writebuflen : (IN) Number of bytes to write.
int* pWrittenBytes : (INOUT) Valid pointer that will receive the number of bytes written.

Return : EXIT_SUCCESS if some bytes are written, EXIT_TIMEOUT if a timeout occurs or 
EXIT_FAILURE if there is an error.
*/
inline int WriteRS232Port(HANDLE hDev, uint8* writebuf, UINT writebuflen, int* pWrittenBytes)
{
#ifdef _DEBUG_MESSAGES_OSRS232PORT
	int i = 0;
#endif // _DEBUG_MESSAGES_OSRS232PORT

#ifdef _WIN32
	if (WriteFile(hDev, writebuf, writebuflen, (LPDWORD)pWrittenBytes, NULL))
	{
#else 
	*pWrittenBytes = write((int)hDev, writebuf, writebuflen);
	if (*pWrittenBytes >= 0)
	{
#endif // _WIN32
		if (*pWrittenBytes == 0)
		{
			PRINT_DEBUG_WARNING_OSRS232PORT(("WriteRS232Port warning (%s) : %s"
				"(hDev=%#x, writebuf=%#x, writebuflen=%u)\n", 
				strtime_m(), 
				szOSUtilsErrMsgs[EXIT_TIMEOUT], 
				hDev, writebuf, writebuflen));
			return EXIT_TIMEOUT;
		}
		else
		{
#ifdef _DEBUG_MESSAGES_OSRS232PORT
			for (i = 0; i < *pWrittenBytes; i++)
			{
				PRINT_DEBUG_MESSAGE_OSRS232PORT(("%.2x ", (int)writebuf[i]));
			}
			PRINT_DEBUG_MESSAGE_OSRS232PORT(("\n"));
#endif // _DEBUG_MESSAGES_OSRS232PORT
			PRINT_DEBUG_MESSAGE_OSRS232PORT(("Bytes written : %d\n", *pWrittenBytes));
		}
	}
	else
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("WriteRS232Port error (%s) : %s"
			"(hDev=%#x, writebuf=%#x, writebuflen=%u)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			hDev, writebuf, writebuflen));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Read data from a serial port. Retry automatically if all the bytes were not read.

HANDLE hDev : (IN) Identifier of the serial port.
uint8* readbuf : (INOUT) Valid pointer that will receive the data read.
UINT readbuflen : (IN) Number of bytes to read.

Return : EXIT_SUCCESS if all the bytes are read, EXIT_TIMEOUT if a timeout occurs or 
EXIT_FAILURE if there is an error.
*/
inline int ReadAllRS232Port(HANDLE hDev, uint8* readbuf, UINT readbuflen)
{
#ifdef _DEBUG_MESSAGES_OSRS232PORT
	UINT i = 0;
#endif // _DEBUG_MESSAGES_OSRS232PORT
	UINT BytesRead = 0;
	int Bytes = 0;

	while (BytesRead < readbuflen)
	{
#ifdef _WIN32
		if (ReadFile(hDev, readbuf + BytesRead, readbuflen - BytesRead, (LPDWORD)&Bytes, NULL))
		{
#else 
		Bytes = read((int)hDev, readbuf + BytesRead, readbuflen - BytesRead);
		if (Bytes >= 0)
		{
#endif // _WIN32
			if (Bytes == 0)
			{
				PRINT_DEBUG_WARNING_OSRS232PORT(("ReadAllRS232Port warning (%s) : %s"
					"(hDev=%#x, readbuf=%#x, readbuflen=%u)\n", 
					strtime_m(), 
					szOSUtilsErrMsgs[EXIT_TIMEOUT], 
					hDev, readbuf, readbuflen));
#ifdef _DEBUG_MESSAGES_OSRS232PORT
				for (i = 0; i < BytesRead; i++)
				{
					PRINT_DEBUG_MESSAGE_OSRS232PORT(("%.2x ", (int)readbuf[i]));
				}
				PRINT_DEBUG_MESSAGE_OSRS232PORT(("\n"));
#endif // _DEBUG_MESSAGES_OSRS232PORT
				PRINT_DEBUG_MESSAGE_OSRS232PORT(("Total bytes read : %u\n", BytesRead));
				return EXIT_TIMEOUT;
			}
			else
			{
				PRINT_DEBUG_MESSAGE_OSRS232PORT(("Bytes read : %d\n", Bytes));
			}
		}
		else
		{
			PRINT_DEBUG_ERROR_OSRS232PORT(("ReadAllRS232Port error (%s) : %s"
				"(hDev=%#x, readbuf=%#x, readbuflen=%u)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				hDev, readbuf, readbuflen));
#ifdef _DEBUG_MESSAGES_OSRS232PORT
			for (i = 0; i < BytesRead; i++)
			{
				PRINT_DEBUG_MESSAGE_OSRS232PORT(("%.2x ", (int)readbuf[i]));
			}
			PRINT_DEBUG_MESSAGE_OSRS232PORT(("\n"));
#endif // _DEBUG_MESSAGES_OSRS232PORT
			PRINT_DEBUG_MESSAGE_OSRS232PORT(("Total bytes read : %u\n", BytesRead));
			return EXIT_FAILURE;
		}

		BytesRead += Bytes;
	}

#ifdef _DEBUG_MESSAGES_OSRS232PORT
	for (i = 0; i < BytesRead; i++)
	{
		PRINT_DEBUG_MESSAGE_OSRS232PORT(("%.2x ", (int)readbuf[i]));
	}
	PRINT_DEBUG_MESSAGE_OSRS232PORT(("\n"));
#endif // _DEBUG_MESSAGES_OSRS232PORT
	PRINT_DEBUG_MESSAGE_OSRS232PORT(("Total bytes read : %u\n", BytesRead));

	return EXIT_SUCCESS;
}

/*
Write data to a serial port. Retry automatically if all the bytes were not written.

HANDLE hDev : (IN) Identifier of the serial port.
uint8* writebuf : (IN) Valid pointer to the data to write.
UINT writebuflen : (IN) Number of bytes to write.

Return : EXIT_SUCCESS if all the bytes are written, EXIT_TIMEOUT if a timeout occurs or 
EXIT_FAILURE if there is an error.
*/
inline int WriteAllRS232Port(HANDLE hDev, uint8* writebuf, UINT writebuflen)
{
#ifdef _DEBUG_MESSAGES_OSRS232PORT
	UINT i = 0;
#endif // _DEBUG_MESSAGES_OSRS232PORT
	UINT BytesWritten = 0;
	int Bytes = 0;

	while (BytesWritten < writebuflen)
	{
#ifdef _WIN32
		if (WriteFile(hDev, writebuf + BytesWritten, writebuflen - BytesWritten, (LPDWORD)&Bytes, NULL))
		{
#else 
		Bytes = write((int)hDev, writebuf + BytesWritten, writebuflen - BytesWritten);
		if (Bytes >= 0)
		{
#endif // _WIN32
			if (Bytes == 0)
			{
				PRINT_DEBUG_WARNING_OSRS232PORT(("WriteAllRS232Port warning (%s) : %s"
					"(hDev=%#x, writebuf=%#x, writebuflen=%u)\n", 
					strtime_m(), 
					szOSUtilsErrMsgs[EXIT_TIMEOUT], 
					hDev, writebuf, writebuflen));
#ifdef _DEBUG_MESSAGES_OSRS232PORT
				for (i = 0; i < BytesWritten; i++)
				{
					PRINT_DEBUG_MESSAGE_OSRS232PORT(("%.2x ", (int)writebuf[i]));
				}
				PRINT_DEBUG_MESSAGE_OSRS232PORT(("\n"));
#endif // _DEBUG_MESSAGES_OSRS232PORT
				PRINT_DEBUG_MESSAGE_OSRS232PORT(("Total bytes written : %u\n", BytesWritten));
				return EXIT_TIMEOUT;
			}
			else
			{
				PRINT_DEBUG_MESSAGE_OSRS232PORT(("Bytes written : %d\n", Bytes));
			}
		}
		else
		{
			PRINT_DEBUG_ERROR_OSRS232PORT(("WriteAllRS232Port error (%s) : %s"
				"(hDev=%#x, writebuf=%#x, writebuflen=%u)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				hDev, writebuf, writebuflen));
#ifdef _DEBUG_MESSAGES_OSRS232PORT
			for (i = 0; i < BytesWritten; i++)
			{
				PRINT_DEBUG_MESSAGE_OSRS232PORT(("%.2x ", (int)writebuf[i]));
			}
			PRINT_DEBUG_MESSAGE_OSRS232PORT(("\n"));
#endif // _DEBUG_MESSAGES_OSRS232PORT
			PRINT_DEBUG_MESSAGE_OSRS232PORT(("Total bytes written : %u\n", BytesWritten));
			return EXIT_FAILURE;
		}

		BytesWritten += Bytes;
	}

#ifdef _DEBUG_MESSAGES_OSRS232PORT
	for (i = 0; i < BytesWritten; i++)
	{
		PRINT_DEBUG_MESSAGE_OSRS232PORT(("%.2x ", (int)writebuf[i]));
	}
	PRINT_DEBUG_MESSAGE_OSRS232PORT(("\n"));
#endif // _DEBUG_MESSAGES_OSRS232PORT
	PRINT_DEBUG_MESSAGE_OSRS232PORT(("Total bytes written : %u\n", BytesWritten));

	return EXIT_SUCCESS;
}

/*
Close a serial port.

HANDLE* phDev : (INOUT) Valid pointer to the identifier of the serial port.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int CloseRS232Port(HANDLE* phDev)
{
#ifdef _WIN32
	if (!CloseHandle(*phDev))
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("CloseRS232Port error (%s) : %s(*phDev=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			*phDev));
		return EXIT_FAILURE;
	}
#else
	if (close((int)*phDev) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSRS232PORT(("CloseRS232Port error (%s) : %s(*phDev=%#x)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			*phDev));
		return EXIT_FAILURE;
	}
#endif // _WIN32
	*phDev  = (HANDLE)(-1);

	return EXIT_SUCCESS;
}

#endif // OSRS232PORT_H
