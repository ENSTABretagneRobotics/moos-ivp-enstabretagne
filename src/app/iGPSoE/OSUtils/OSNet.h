/***************************************************************************************************************:')

OSNet.h

Network includes, typedefs, defines and initialization.

Fabrice Le Bars

Created : 2007

Version status : Not finished

***************************************************************************************************************:)*/

// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef OSNET_H
#define OSNET_H

#include "OSTime.h"

/*
Debug macros specific to OSNet.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSNET
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSNET
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSNET
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSNET
#	define PRINT_DEBUG_MESSAGE_OSNET(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSNET(params)
#endif // _DEBUG_MESSAGES_OSNET

#ifdef _DEBUG_WARNINGS_OSNET
#	define PRINT_DEBUG_WARNING_OSNET(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSNET(params)
#endif // _DEBUG_WARNINGS_OSNET

#ifdef _DEBUG_ERRORS_OSNET
#	define PRINT_DEBUG_ERROR_OSNET(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSNET(params)
#endif // _DEBUG_ERRORS_OSNET

#ifdef _WIN32
// The maximum number of sockets that an application can actually use is independent of
// the number of sockets supported by a particular implementation. The maximum number of
// sockets that a Windows Sockets application can use is determined at compile time by the
// manifest constant FD_SETSIZE. This value is used in constructing the FD_SET structures
// used in select. The default value in Winsock2.h is 64.
//#define FD_SETSIZE 64
#else
//#define FD_SETSIZE 64
#endif // _WIN32

#ifdef _WIN32
#ifdef __BORLANDC__
// Disable some Borland C++ Builder warnings that happen in ws2tcpip.h.
#pragma warn -8004
#endif // __BORLANDC__
#include <winsock2.h>
#if (_WIN32_WINNT <= 0x0500)
#include <ws2tcpip.h>
#include <Wspiapi.h>
#else
#include <ws2tcpip.h>
#endif // (_WIN32_WINNT <= 0x0500)
//#include <iphlpapi.h>
#ifdef __BORLANDC__
// Restore the Borland C++ Builder warnings previously disabled for ws2tcpip.h.
#pragma warn .8004
#endif // __BORLANDC__
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif // _WIN32

#ifdef _WIN32
/*
Format a message corresponding to the last error in a system call related
to network with Winsock (thread-safe).

char* buf : (INOUT) Valid pointer to a buffer that will receive the message.
int buflen : (IN) Size of the buffer in bytes.

Return : buf.
*/
inline char* WSAFormatLastErrorMsg(char* buf, int buflen)
{
	memset(buf, 0, (size_t)buflen);
	if (!FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_MAX_WIDTH_MASK, // Remove the line break at the end.
		// Note that there is still a dot and a space at the end.
		NULL,
		WSAGetLastError(),
		0,
		(LPTSTR)buf,
		(DWORD)buflen,
		NULL
		))
	{
#ifdef _MSC_VER
		sprintf_s(buf, (size_t)buflen, "FormatMessage failed. ");
#else
		snprintf(buf, (size_t)buflen, "FormatMessage failed. ");
		buf[buflen-1] = 0;
#endif // _MSC_VER
		return buf;
	}
	return buf;
}
#else
typedef int SOCKET;

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

#define SD_RECEIVE SHUT_RD
#define SD_SEND SHUT_WR
#define SD_BOTH SHUT_RDWR

#define closesocket(sock) close((sock))
#endif // _WIN32

// Specific macro used with PRINT_DEBUG_MESSAGE, PRINT_DEBUG_WARNING
// or PRINT_DEBUG_ERROR to return the message corresponding to the last error
// in a system call related to network (with Winsock for Windows) in the
// current thread.
#ifdef _WIN32
#define WSAGetLastErrorMsg() WSAFormatLastErrorMsg(szLastErrMsg, LAST_ERROR_MSG_SIZE)
#else
#define WSAGetLastErrorMsg GetLastErrorMsg
#endif // _WIN32

/*
Initialize the network. Must be called before the use of any function from OSTCPSock.c/.h
or OSUDPSock.c/.h. Use ReleaseNet() to release the network at the end.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int InitNet(void)
{
#ifdef _WIN32
	WSADATA wsaData;

	// Initiate use of the Winsock DLL by a process.
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	switch (iResult)
	{
	case EXIT_SUCCESS:
		break;
#ifdef _DEBUG_ERRORS_OSNET
	case WSASYSNOTREADY:
		PRINT_DEBUG_ERROR_OSNET(("InitNet error (%s) : %s"
			"\n",
			strtime_m(),
			"The underlying network subsystem is not ready for network communication. "));
		return EXIT_FAILURE;
	case WSAVERNOTSUPPORTED:
		PRINT_DEBUG_ERROR_OSNET(("InitNet error (%s) : %s"
			"\n",
			strtime_m(),
			"The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation. "));
		return EXIT_FAILURE;
	case WSAEINPROGRESS:
		PRINT_DEBUG_ERROR_OSNET(("InitNet error (%s) : %s"
			"\n",
			strtime_m(),
			"A blocking Windows Sockets 1.1 operation is in progress. "));
		return EXIT_FAILURE;
	case WSAEPROCLIM:
		PRINT_DEBUG_ERROR_OSNET(("InitNet error (%s) : %s"
			"\n",
			strtime_m(),
			"A limit on the number of tasks supported by the Windows Sockets implementation has been reached. "));
		return EXIT_FAILURE;
	case WSAEFAULT:
		PRINT_DEBUG_ERROR_OSNET(("InitNet error (%s) : %s"
			"\n",
			strtime_m(),
			"The lpWSAData parameter is not a valid pointer. "));
		return EXIT_FAILURE;
#endif // _DEBUG_ERRORS_OSNET
	default:
		PRINT_DEBUG_ERROR_OSNET(("InitNet error (%s) : %s"
			"\n",
			strtime_m(),
			"WSAStartup failed. "));
		return EXIT_FAILURE;
	}
#endif

	return EXIT_SUCCESS;
}

/*
Release the network (previously initialized by InitNet()).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int ReleaseNet(void)
{
#ifdef _WIN32
	// Terminate use of the Winsock 2 DLL (Ws2_32.dll).
	int iResult = WSACleanup();
	switch (iResult)
	{
	case EXIT_SUCCESS:
		break;
#ifdef _DEBUG_ERRORS_OSNET
	case WSANOTINITIALISED:
		PRINT_DEBUG_ERROR_OSNET(("ReleaseNet error (%s) : %s"
			"\n",
			strtime_m(),
			"A successful WSAStartup call must occur before using this function. "));
		return EXIT_FAILURE;
	case WSAENETDOWN:
		PRINT_DEBUG_ERROR_OSNET(("ReleaseNet error (%s) : %s"
			"\n",
			strtime_m(),
			"The network subsystem has failed. "));
		return EXIT_FAILURE;
	case WSAEINPROGRESS:
		PRINT_DEBUG_ERROR_OSNET(("ReleaseNet error (%s) : %s"
			"\n",
			strtime_m(),
			"A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. "));
		return EXIT_FAILURE;
#endif // _DEBUG_ERRORS_OSNET
	default:
		PRINT_DEBUG_ERROR_OSNET(("ReleaseNet error (%s) : %s"
			"\n",
			strtime_m(),
			"WSACleanup failed. "));
		return EXIT_FAILURE;
	}
#endif

	return EXIT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Portable functions that can be directly used.
//////////////////////////////////////////////////////////////////////////////////////////

// Note that struct sockaddr variables should be defined as struct sockaddr_storage (to be protocol independant) and then
// cast to struct sockaddr when called in functions that need this type (especially when it is a pointer).

// If a send or receive operation times out on a socket, the socket state is indeterminate, and should not be used.

// If the pNodeName parameter points to a computer name, all permanent addresses for the computer that can be used as a source address are returned.
// If the pNodeName parameter points to a string equal to "localhost", all loopback addresses on the local computer are returned.
// If the pNodeName parameter contains an empty string, all registered addresses on the local computer are returned.
// ppResult is a linked list.
// Do not set errno, use gai_strerror() for Linux (thread-safe?) or WSAGetLastErrorMsg() for Windows.
//int getaddrinfo(char* pNodeName, char* pServiceName, struct addrinfo* pHints, struct addrinfo** ppResult);

// Free the linked-list returned by getaddrinfo()
//void freeaddrinfo(struct addrinfo* pAi);

// Convert a socket address to a corresponding host/address and service/port.
// Use NI_MAXHOST and NI_MAXSERV for the size of host and serv.
// flags can be a combination of NI_NAMEREQD (a host name that cannot be resolved by DNS results in an error),
// NI_NOFQDN (return only the hostname part of the fully qualified domain name for local hosts)
// NI_NUMERICHOST (returns the numeric form of the host name instead of its name)
// NI_NUMERICSERV (returns the port number of the service instead of its name)
// NI_DGRAM (indicates that the service is an UDP service rather than TCP)
// Do not set errno, use gai_strerror() for Linux (thread-safe?) or WSAGetLastErrorMsg() for Windows.
//int getnameinfo(const struct sockaddr* sa, socklen_t salen, char* host, size_t hostlen, char* serv, size_t servlen, int flags);

// Translate the error codes of getaddrinfo() and getnameinfo() to a string for Linux.
//const char* gai_strerror(int errcode);

/*
Create a socket for a specific protocol (TCP/IPv4, UDP/IPv4, TCP/IPv6 or UDP/IPv6 are the
most common).
Used at the beginning to create a server or client application.
Often use the addrinfo results of a call to getaddrinfo().
InitNet() must be called before using any network function.

int af : (IN) Address family : AF_INET for IPv4, AF_INET6 for IPv6. Often set to the ai_family
field of an addrinfo element of the linked list returned by getaddrinfo().
int type : (IN) Socket type : SOCK_STREAM for TCP, SOCK_DGRAM for UDP. Often set to the ai_socktype
field of an addrinfo element of the linked list returned by getaddrinfo().
int protocol : (IN) Protocol to be used : IPPROTO_TCP for TCP, IPPROTO_UDP for UDP. Often set to the ai_protocol
field of an addrinfo element of the linked list returned by getaddrinfo().

Return : The socket created or INVALID_SOCKET if there is an error.
*/
//SOCKET socket(int af, int type, int protocol);

/*
Close an existing socket.
InitNet() must be called before using any network function.

SOCKET s : (IN) Socket to close.

Return : EXIT_SUCCESS or SOCKET_ERROR if there is an error.
*/
//int closesocket(SOCKET s);

/*
Associate a local address and port with a server socket.
InitNet() must be called before using any network function.

SOCKET s : (IN) Unbound server socket.
struct sockaddr* name : (IN) Address family, host address and port to assign to the socket as a sockaddr structure.
Can be obtained from getaddrinfo() (ai_addr field of the resulting addrinfo structures) or built using its
fields sin_family, sin_addr.s_addr (using the conversion function inet_addr()) and sin_port (using the
conversion function htons()). There are several special addresses : INADDR_LOOPBACK ("127.0.0.1") always
refers to the local host via the loopback device, INADDR_ANY ("0.0.0.0") means any address for binding.
When INADDR_ANY is specified, the socket will be bound to all local interfaces.
int namelen : (IN) Length of the value in the name parameter in bytes.

Return : EXIT_SUCCESS or SOCKET_ERROR if there is an error.
*/
//int bind(SOCKET s, struct sockaddr* name, int namelen);

/*
Place a socket in a state in which it is listening for an incoming connection. The socket is put into
passive mode where incoming connection requests are acknowledged and queued pending acceptance by the
process. If a connection request arrives and the queue is full, the client will receive an error.
Should be used only with TCP sockets.
InitNet() must be called before using any network function.

SOCKET s : (IN) Bound and unconnected server socket.
int backlog : (IN) Maximum length of the queue of pending connections. Set to 1 if there
should be only 1 simultaneous client or to SOMAXCONN for a default maximum reasonable value.

Return : EXIT_SUCCESS or SOCKET_ERROR if there is an error.
*/
//int listen(SOCKET s, int backlog);

/*
Determine the status of the sockets in the different fd_set structures, waiting if necessary until
at least one socket meets the specified criteria depending on timeout. Upon return, the fd_set structures
are updated to reflect the subset of the sockets that meet the specified condition.
Macros to manipulate and check fd_set contents :

FD_ZERO(*set)
Initialize the set to the null set.

FD_SET(s, *set)
Add socket s to set.

FD_ISSET(s, *set)
Nonzero if s is a member of the set. Otherwise, 0.

FD_CLR(s, *set)
Remove the socket s from set.

Structure to define the timeout :

struct timeval {
long tv_sec; // Seconds.
long tv_usec; // Microseconds.
};

InitNet() must be called before using any network function.

int nfds : (IN) Highest-numbered file descriptor in any of the three sets, plus 1.
fd_set* pReadfds : (INOUT) Valid pointer to a set of sockets to be checked for readability
(data is available i.e. a recv(), recvfrom() is guaranteed not to block, a connect request has been received
by a listening socket such that an accept() is guaranteed to complete without blocking, or a request to
close the socket has been received). NULL to ignore.
fd_set* pWritefds : (INOUT) Valid pointer to a set of sockets to be checked for writability (data can be
sent i.e. a send(), sendto() is guaranteed to succeed). NULL to ignore.
fd_set* pExceptfds : (INOUT) Valid pointer to a set of sockets to be checked for errors/exceptions (do not use).
NULL to ignore.
struct timeval* pTimeout : (INOUT) Valid pointer to a timeval structure specifying the maximum time to wait,
NULL for infinite. Return immediately if the structure is initialized to {0,0}. The structure may be modified
after the call.

Return : The total number of sockets contained in the updated fd_set structures, 0 if the time limit expired
or SOCKET_ERROR if there is an error.
*/
//int select(int nfds, fd_set* pReadfds, fd_set* pWritefds, fd_set* pExceptfds, struct timeval* pTimeout);

/*
Extract the first connection request on the queue of pending connections for a listening socket.
Can block the caller until a connection is present.
Should be used only with TCP sockets.
InitNet() must be called before using any network function.

SOCKET s : (IN) Bound and listening server socket.
struct sockaddr* addr : (INOUT) Address family, host address and port of the accepted client socket as a
sockaddr structure (fields sin_family, sin_addr.s_addr (using the conversion function inet_addr()) and
sin_port (using the conversion function htons()). NULL to ignore.
int* addrlen : (INOUT) Valid pointer to the length of the addr parameter in bytes.
On return it will contain the actual length in bytes of the address returned. NULL to ignore.

Return : The client socket or INVALID_SOCKET if there is an error.
*/
//SOCKET accept(SOCKET s, struct sockaddr* addr, int* addrlen);

/*
Establish a connection to a server.
Usually used with TCP sockets.
InitNet() must be called before using any network function.

SOCKET s : (IN) Unconnected client socket.
struct sockaddr* name : (IN) Address family, host address and port of the server socket as a sockaddr
structure (fields sin_family, sin_addr.s_addr (using the conversion function inet_addr()) and sin_port
(using the conversion function htons()).
int namelen : (IN) Length of the value in the name parameter in bytes.

Return : EXIT_SUCCESS or SOCKET_ERROR if there is an error.
*/
//int connect(SOCKET s, struct sockaddr* name, int namelen);

//int send(SOCKET s);

//int recv(SOCKET s);

/*
Usually used with UDP sockets.
InitNet() must be called before using any network function.

struct sockaddr* to : (IN) Address family, host address and port of the target socket as a sockaddr
structure (fields sin_family, sin_addr.s_addr (using the conversion function inet_addr()) and sin_port
(using the conversion function htons()).
int tolen : (IN) Length of the value in the to parameter in bytes.

*/
//int sendto(SOCKET s, char* buf, int len, int flags, struct sockaddr* to, int tolen);

/*
Usually used with UDP sockets.
InitNet() must be called before using any network function.

struct sockaddr* from : (INOUT) Address family, host address and port of the source socket as a
sockaddr structure (fields sin_family, sin_addr.s_addr (using the conversion function inet_addr()) and
sin_port (using the conversion function htons()). NULL to ignore.
int fromlen : (IN) INOUT) Valid pointer to the length of the from parameter in bytes.
On return it will contain the actual length in bytes of the address returned. NULL to ignore.

*/
//int recvfrom(SOCKET s, char* buf, int len, int flags, struct sockaddr* from, int fromlen);

//int shutdown(SOCKET s);

#define DEFAULT_SOCK_TIMEOUT 10000

/*
Set timeout options for a given socket.

SOCKET sock : (IN) Socket.
int timeout : (IN) Timeout in ms for send and recv (0 to disable timeouts).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int setsockettimeouts(SOCKET sock, int timeout)
{
#ifdef _WIN32
	int iOptVal = 0;
#else
	struct timeval tv;
#endif // _WIN32
	int iOptLen = 0;

#ifdef _WIN32
	iOptVal = timeout; // In ms.
	iOptLen = sizeof(int);
	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&iOptVal, iOptLen) == SOCKET_ERROR)
	{
		printf("setsockopt() failed.\n");
		return EXIT_FAILURE;
	}
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&iOptVal, iOptLen) == SOCKET_ERROR)
	{
		printf("setsockopt() failed.\n");
		return EXIT_FAILURE;
	}
#else
	tv.tv_sec = (long)(timeout/1000);
	tv.tv_usec = (long)((timeout%1000)*1000);
	iOptLen = sizeof(struct timeval);
	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, iOptLen) == SOCKET_ERROR)
	{
		printf("setsockopt() failed.\n");
		return EXIT_FAILURE;
	}
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, iOptLen) == SOCKET_ERROR)
	{
		printf("setsockopt() failed.\n");
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Connect to an IPv4 UDP server.

SOCKET* pSock : (INOUT) Valid pointer to a socket that will be used to communicate with the server.
char* address : (IN) IPv4 address of the server.
char* port : (IN) TCP port of the server.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int initudpcli(SOCKET* pSock, char* address, char* port)
{
	struct sockaddr_in sa;

#ifdef _WIN32
	WSADATA wsaData;
#endif // _WIN32

#ifdef _WIN32
	// Initiate use of the Winsock DLL by a process.
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != EXIT_SUCCESS)
	{
		printf("WSAStartup() failed.\n");
		return EXIT_FAILURE;
	}
#endif // _WIN32

	// Create a UDP IPv4 socket.
	*pSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (*pSock == INVALID_SOCKET)
	{
		printf("socket() failed.\n");
#ifdef _WIN32
		WSACleanup();
#endif // _WIN32
		return EXIT_FAILURE;
	}

	// Configure timeouts for send and recv.
	if (setsockettimeouts(*pSock, DEFAULT_SOCK_TIMEOUT) != EXIT_SUCCESS)
	{
		printf("setsockettimeouts() failed.\n");
		closesocket(*pSock);
#ifdef _WIN32
		WSACleanup();
#endif // _WIN32
		return EXIT_FAILURE;
	}
/*
	memset(&sa, 0, sizeof(sa));

	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the client.
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(address);
	sa.sin_port = htons((unsigned short)atoi(port));

	// Associate the client to the desired address and port.
	if (bind(*pSock, (struct sockaddr*)&sa, sizeof(sa)) != EXIT_SUCCESS)
	{
		printf("bind() failed.\n");
		closesocket(*pSock);
#ifdef _WIN32
		WSACleanup();
#endif // _WIN32
		return EXIT_FAILURE;
	}
*/
	memset(&sa, 0, sizeof(sa));

	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(address);
	sa.sin_port = htons((unsigned short)atoi(port));

	// Connect to server.
	if (connect(*pSock, (struct sockaddr*)&sa, sizeof(sa)) != EXIT_SUCCESS)
	{
		printf("connect() failed.\n");
		closesocket(*pSock);
#ifdef _WIN32
		WSACleanup();
#endif // _WIN32
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Connect to an IPv4 TCP server.

SOCKET* pSock : (INOUT) Valid pointer to a socket that will be used to communicate with the server.
char* address : (IN) IPv4 address of the server.
char* port : (IN) TCP port of the server.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int inittcpcli(SOCKET* pSock, char* address, char* port)
{
	struct sockaddr_in sa;

#ifdef _WIN32
	WSADATA wsaData;
#endif // _WIN32

#ifdef _WIN32
	// Initiate use of the Winsock DLL by a process.
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != EXIT_SUCCESS)
	{
		printf("WSAStartup() failed.\n");
		return EXIT_FAILURE;
	}
#endif // _WIN32

	// Create a TCP IPv4 socket.
	*pSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*pSock == INVALID_SOCKET)
	{
		printf("socket() failed.\n");
#ifdef _WIN32
		WSACleanup();
#endif // _WIN32
		return EXIT_FAILURE;
	}

	// Configure timeouts for send and recv.
	if (setsockettimeouts(*pSock, DEFAULT_SOCK_TIMEOUT) != EXIT_SUCCESS)
	{
		printf("setsockettimeouts() failed.\n");
		closesocket(*pSock);
#ifdef _WIN32
		WSACleanup();
#endif // _WIN32
		return EXIT_FAILURE;
	}

	memset(&sa, 0, sizeof(sa));

	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(address);
	sa.sin_port = htons((unsigned short)atoi(port));

	// Connect to server.
	if (connect(*pSock, (struct sockaddr*)&sa, sizeof(sa)) != EXIT_SUCCESS)
	{
		printf("connect() failed Here.\n");
		closesocket(*pSock);
#ifdef _WIN32
		WSACleanup();
#endif // _WIN32
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int sendtoall(SOCKET sock, char* sendbuf, int sendbuflen, struct sockaddr* sa, int salen)
{
	int BytesSent = 0;
	int Bytes = 0;

	while (BytesSent < sendbuflen)
	{
		Bytes = sendto(sock, sendbuf + BytesSent, sendbuflen - BytesSent, 0, sa, salen);
		if (Bytes >= 0)
		{
			if (Bytes == 0)
			{
				printf("sendto() timed out.\n");
				//printf("Total bytes sent : %u\n", BytesSent);
				return EXIT_TIMEOUT;
			}
			else
			{
				//printf("Bytes sent : %d\n", Bytes);
			}
		}
		else
		{
			printf("sendto() failed.\n");
			//printf("Total bytes sent : %u\n", BytesSent);
			return EXIT_FAILURE;
		}

		BytesSent += Bytes;
	}

	return EXIT_SUCCESS;
}

inline int recvfromall(SOCKET sock, char* recvbuf, int recvbuflen, struct sockaddr* sa, int* pSalen)
{
	int BytesReceived = 0;
	int Bytes = 0;

	while (BytesReceived < recvbuflen)
	{
		Bytes = recvfrom(sock, recvbuf + BytesReceived, recvbuflen - BytesReceived, 0, sa, (socklen_t*)pSalen);
		if (Bytes >= 0)
		{
			if (Bytes == 0)
			{
				printf("recvfrom() timed out.\n");
				//printf("Total bytes received : %u\n", BytesReceived);
				return EXIT_TIMEOUT;
			}
			else
			{
				//printf("Bytes received : %d\n", Bytes);
			}
		}
		else
		{
			printf("recvfrom() failed.\n");
			//printf("Total bytes received : %u\n", BytesReceived);
			return EXIT_FAILURE;
		}

		BytesReceived += Bytes;
	}

	return EXIT_SUCCESS;
}

/*
Send data to a given socket. Retry automatically if all the bytes were not sent.
Fail when a timeout occurs if it is enabled on the socket.

SOCKET sock : (IN) Socket.
char* sendbuf : (IN) Data to send.
int sendbuflen : (IN) Number of bytes to send.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int sendall(SOCKET sock, char* sendbuf, int sendbuflen)
{
	int BytesSent = 0;
	int Bytes = 0;

	while (BytesSent < sendbuflen)
	{
		Bytes = send(sock, sendbuf + BytesSent, sendbuflen - BytesSent, 0);
		if (Bytes >= 0)
		{
			if (Bytes == 0)
			{
				printf("send() timed out.\n");
				//printf("Total bytes sent : %u\n", BytesSent);
				return EXIT_TIMEOUT;
			}
			else
			{
				//printf("Bytes sent : %d\n", Bytes);
			}
		}
		else
		{
			printf("send() failed.\n");
			//printf("Total bytes sent : %u\n", BytesSent);
			return EXIT_FAILURE;
		}

		BytesSent += Bytes;
	}

	return EXIT_SUCCESS;
}

/*
Receive data at a given socket. Retry automatically if all the bytes were not received.
Fail when a timeout occurs if it is enabled on the socket.

SOCKET sock : (IN) Socket.
char* recvbuf : (INOUT) Buffer which will contain the data received.
int recvbuflen : (IN) Number of bytes to receive.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int recvall(SOCKET sock, char* recvbuf, int recvbuflen)
{
	int BytesReceived = 0;
	int Bytes = 0;

	while (BytesReceived < recvbuflen)
	{
		Bytes = recv(sock, recvbuf + BytesReceived, recvbuflen - BytesReceived, 0);
		if (Bytes >= 0)
		{
			if (Bytes == 0)
			{
				printf("recv() timed out.\n");
				//printf("Total bytes received : %u\n", BytesReceived);
				return EXIT_TIMEOUT;
			}
			else
			{
				//printf("Bytes received : %d\n", Bytes);
			}
		}
		else
		{
			printf("recv() failed.\n");
			//printf("Total bytes received : %u\n", BytesReceived);
			return EXIT_FAILURE;
		}

		BytesReceived += Bytes;
	}

	return EXIT_SUCCESS;
}

inline int recvlatest(SOCKET sock, char* recvbuf, int recvbuflen)
{
	int BytesReceived = 0;
	int Bytes = 0;
	char* savebuf = NULL;

	savebuf = (char*)calloc(recvbuflen, sizeof(char));

	if (savebuf == NULL)
	{
		printf("calloc() failed.\n");
		return EXIT_OUT_OF_MEMORY;
	}

	Bytes = recv(sock, recvbuf, recvbuflen, 0);
	if (Bytes >= 0)
	{
		if (Bytes == 0)
		{
			printf("recv() timed out.\n");
			//printf("Total bytes received : %u\n", BytesReceived);
			free(savebuf);
			return EXIT_TIMEOUT;
		}
		else
		{
			//printf("Bytes received : %d\n", Bytes);
		}
	}
	else
	{
		printf("recv() failed.\n");
		//printf("Total bytes received : %u\n", BytesReceived);
		free(savebuf);
		return EXIT_FAILURE;
	}

	BytesReceived += Bytes;

	while (Bytes == recvbuflen)
	{
		memcpy(savebuf, recvbuf, Bytes);
		Bytes = recv(sock, recvbuf, recvbuflen, 0);
		if (Bytes >= 0)
		{
			if (Bytes == 0)
			{
				printf("recv() timed out.\n");
				//printf("Total bytes received : %u\n", BytesReceived);
				free(savebuf);
				return EXIT_TIMEOUT;
			}
			else
			{
				//printf("Bytes received : %d\n", Bytes);
			}
		}
		else
		{
			printf("recv() failed.\n");
			//printf("Total bytes received : %u\n", BytesReceived);
			free(savebuf);
			return EXIT_FAILURE;
		}

		BytesReceived += Bytes;
	}

	if (BytesReceived < recvbuflen)
	{
		int iResult = recvall(sock, recvbuf+BytesReceived, recvbuflen-BytesReceived);
		if (iResult != EXIT_SUCCESS)
		{
			free(savebuf);
			return iResult;
		}
	}
	else
	{
		memmove(recvbuf+recvbuflen-Bytes, recvbuf, Bytes);
		memcpy(recvbuf, savebuf+Bytes, recvbuflen-Bytes);
	}

	free(savebuf);

	return EXIT_SUCCESS;
}

/*
Receive data at a given socket until a specific end character is received.
If this character is found (and the maximum number of bytes to receive has not
been reached), it is not necessarily the last received byte in the buffer
(other bytes might have been received after).
Fail when a timeout occurs if it is enabled on the socket.

SOCKET sock : (IN) Socket.
char* recvbuf : (INOUT) Buffer which will contain the data received.
char endchar : (IN) End character to wait for.
int maxrecvbuflen : (IN) Maximum number of bytes to receive.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int recvatleastuntil(SOCKET sock, char* recvbuf, char endchar, int maxrecvbuflen)
{
	int BytesReceived = 0;
	int Bytes = 0;
	int bStop = 0;

	for (;;)
	{
		if (BytesReceived >= maxrecvbuflen)
		{
			printf("recvbuf full.\n");
			//printf("Total bytes received : %u\n", BytesReceived);
			return EXIT_FAILURE;
		}

		Bytes = recv(sock, recvbuf + BytesReceived, maxrecvbuflen - BytesReceived, 0);
		if (Bytes >= 0)
		{
			if (Bytes == 0)
			{
				printf("recv() timed out.\n");
				//printf("Total bytes received : %u\n", BytesReceived);
				return EXIT_TIMEOUT;
			}
			else
			{
				int i = 0;

				//printf("Bytes received : %d\n", Bytes);

				// Look for endchar in the bytes just received.
				for (i = BytesReceived; i < BytesReceived+Bytes; i++)
				{
					if (recvbuf[i] == endchar)
					{
						bStop = 1;
						break;
					}
				}

				if (bStop)
				{
					break;
				}
			}
		}
		else
		{
			printf("recv() failed.\n");
			//printf("Total bytes received : %u\n", BytesReceived);
			return EXIT_FAILURE;
		}

		BytesReceived += Bytes;
	}

	return EXIT_SUCCESS;
}

/*
Receive data at a given socket until a specific end character is received.
If this character is found (and the maximum number of bytes to receive has not
been reached), it is the last received byte in the buffer.
This function might take more network load than recvatleastuntil() but guarantees
that no bytes are received after the end character (the bytes that might be between
the end character and the maximum number of bytes to receive are left unchanged).
Fail when a timeout occurs if it is enabled on the socket.

SOCKET sock : (IN) Socket.
char* recvbuf : (INOUT) Buffer which will contain the data received.
char endchar : (IN) End character to wait for.
int maxrecvbuflen : (IN) Maximum number of bytes to receive.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int recvuntil(SOCKET sock, char* recvbuf, char endchar, int maxrecvbuflen)
{
	int BytesReceived = 0;
	int Bytes = 0;

	// Receive byte per byte.
	while ((BytesReceived <= 0)||(recvbuf[BytesReceived-1] != endchar))
	{
		if (BytesReceived >= maxrecvbuflen)
		{
			printf("recvbuf full.\n");
			//printf("Total bytes received : %u\n", BytesReceived);
			return EXIT_FAILURE;
		}

		// Receive 1 byte.
		Bytes = recv(sock, recvbuf + BytesReceived, 1, 0);
		if (Bytes >= 0)
		{
			if (Bytes == 0)
			{
				printf("recv() timed out.\n");
				//printf("Total bytes received : %u\n", BytesReceived);
				return EXIT_TIMEOUT;
			}
			else
			{
				//printf("Bytes received : %d\n", Bytes);
			}
		}
		else
		{
			printf("recv() failed.\n");
			//printf("Total bytes received : %u\n", BytesReceived);
			return EXIT_FAILURE;
		}

		BytesReceived += Bytes;
	}

	return EXIT_SUCCESS;
}

/*
Disconnect from an IPv4 UDP server.

SOCKET sock : (IN) Socket used to communicate with the server.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int releaseudpcli(SOCKET sock)
{
	// Shutdown the connection.
	if (shutdown(sock, SD_BOTH) != EXIT_SUCCESS)
	{
		printf("shutdown() failed.\n");
		//return EXIT_FAILURE;
	}

	// Destroy the socket created by socket().
	if (closesocket(sock) != EXIT_SUCCESS)
	{
		printf("closesocket() failed.\n");
		return EXIT_FAILURE;
	}

#ifdef _WIN32
	// Terminate use of the Winsock 2 DLL (Ws2_32.dll).
	if (WSACleanup() != EXIT_SUCCESS)
	{
		printf("WSAStartup() failed.\n");
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Disconnect from an IPv4 TCP server.

SOCKET sock : (IN) Socket used to communicate with the server.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int releasetcpcli(SOCKET sock)
{
	// Shutdown the connection.
	if (shutdown(sock, SD_BOTH) != EXIT_SUCCESS)
	{
		printf("shutdown() failed.\n");
		//return EXIT_FAILURE;
	}

	// Destroy the socket created by socket().
	if (closesocket(sock) != EXIT_SUCCESS)
	{
		printf("closesocket() failed.\n");
		return EXIT_FAILURE;
	}

#ifdef _WIN32
	// Terminate use of the Winsock 2 DLL (Ws2_32.dll).
	if (WSACleanup() != EXIT_SUCCESS)
	{
		printf("WSAStartup() failed.\n");
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Create an IPv4 UDP server.

SOCKET* pSock : (INOUT) Valid pointer to a server socket.
char* address : (IN) IPv4 address of the server. Set to "0.0.0.0" to use all local
network interfaces.
char* port : (IN) UDP port of the server.
int timeout : (IN) Timeout in ms for send and recv (0 to disable timeouts).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int initudpsrv(SOCKET* pSock, char* address, char* port, int timeout)
{
	struct sockaddr_in sa;

#ifdef _WIN32
	WSADATA wsaData;
#endif // _WIN32

#ifdef _WIN32
	// Initiate use of the Winsock DLL by a process.
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != EXIT_SUCCESS)
	{
		printf("WSAStartup() failed.\n");
		return EXIT_FAILURE;
	}
#endif // _WIN32

	// Create a UDP IPv4 socket.
	*pSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (*pSock == INVALID_SOCKET)
	{
		printf("socket() failed.\n");
#ifdef _WIN32
		WSACleanup();
#endif // _WIN32
		return EXIT_FAILURE;
	}

	// Configure timeouts for send and recv.
	if (setsockettimeouts(*pSock, timeout) != EXIT_SUCCESS)
	{
		printf("setsockettimeouts() failed.\n");
		closesocket(*pSock);
#ifdef _WIN32
		WSACleanup();
#endif // _WIN32
		return EXIT_FAILURE;
	}

	memset(&sa, 0, sizeof(sa));

	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server.
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(address);
	sa.sin_port = htons((unsigned short)atoi(port));

	// Associate the server to the desired address and port.
	if (bind(*pSock, (struct sockaddr*)&sa, sizeof(sa)) != EXIT_SUCCESS)
	{
		printf("bind() failed.\n");
		closesocket(*pSock);
#ifdef _WIN32
		WSACleanup();
#endif // _WIN32
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Create an IPv4 TCP server.

SOCKET* pSock : (INOUT) Valid pointer to a server socket.
char* address : (IN) IPv4 address of the server. Set to "0.0.0.0" to use all local
network interfaces.
char* port : (IN) TCP port of the server.
int maxnbcli : (IN) Maximum number of simultaneous client connections. Set to 1 if there
should be only 1 simultaneous client or to SOMAXCONN for a default maximum reasonable value.
int timeout : (IN) Timeout in ms for send and recv (0 to disable timeouts).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int inittcpsrv(SOCKET* pSock, char* address, char* port, int maxnbcli, int timeout)
{
	struct sockaddr_in sa;

#ifdef _WIN32
	WSADATA wsaData;
#endif // _WIN32

#ifdef _WIN32
	// Initiate use of the Winsock DLL by a process.
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != EXIT_SUCCESS)
	{
		printf("WSAStartup() failed.\n");
		return EXIT_FAILURE;
	}
#endif // _WIN32

	// Create a TCP IPv4 socket.
	*pSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*pSock == INVALID_SOCKET)
	{
		printf("socket() failed.\n");
#ifdef _WIN32
		WSACleanup();
#endif // _WIN32
		return EXIT_FAILURE;
	}

	// Configure timeouts for send and recv.
	if (setsockettimeouts(*pSock, timeout) != EXIT_SUCCESS)
	{
		printf("setsockettimeouts() failed.\n");
		closesocket(*pSock);
#ifdef _WIN32
		WSACleanup();
#endif // _WIN32
		return EXIT_FAILURE;
	}

	memset(&sa, 0, sizeof(sa));

	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server.
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(address);
	sa.sin_port = htons((unsigned short)atoi(port));

	// Associate the server to the desired address and port.
	if (bind(*pSock, (struct sockaddr*)&sa, sizeof(sa)) != EXIT_SUCCESS)
	{
		printf("bind() failed.\n");
		closesocket(*pSock);
#ifdef _WIN32
		WSACleanup();
#endif // _WIN32
		return EXIT_FAILURE;
	}

	// Set the number of simultaneous client connections that can be accepted by the server.
	if (listen(*pSock, maxnbcli) != EXIT_SUCCESS)
	{
		printf("listen() failed.\n");
		closesocket(*pSock);
#ifdef _WIN32
		WSACleanup();
#endif // _WIN32
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

#ifdef _MSC_VER
// Disable some Visual Studio warnings.
#pragma warning(disable : 4127)
#endif // _MSC_VER
/*
Wait for a client for an IPv4 UDP server.

SOCKET socksrv : (IN) Server socket.
SOCKET* pSockCli : (INOUT) Valid pointer that will receive the client socket.
int timeout : (IN) Max time to wait in ms (0 to disable timeout).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int waitforcliforudpsrv(SOCKET socksrv, SOCKET* pSockCli, int timeout)
{
	fd_set sock_set;
	int iResult = SOCKET_ERROR;
	struct timeval tv;
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(struct sockaddr_storage);
	char hostname[NI_MAXHOST];
	char service[NI_MAXSERV];
	char buf[1];

	tv.tv_sec = (long)(timeout/1000);
	tv.tv_usec = (long)((timeout%1000)*1000);

	*pSockCli = INVALID_SOCKET;

	// Initialize a fd_set and add the server socket to it.
	FD_ZERO(&sock_set);
	FD_SET(socksrv, &sock_set);

	// Wait for the readability of the socket in the fd_set, with a timeout.
	if (timeout != 0)
	{
		iResult = select((int)socksrv+1, &sock_set, NULL, NULL, &tv);
	}
	else
	{
		iResult = select((int)socksrv+1, &sock_set, NULL, NULL, NULL);
	}

	// Remove the server socket from the set.
	// No need to use FD_ISSET() here, as we only have one socket the return value of select() is
	// sufficient to know what happened.
	FD_CLR(socksrv, &sock_set);

	if (iResult == SOCKET_ERROR)
	{
		printf("select() failed.\n");
		return EXIT_FAILURE;
	}

	if (iResult == 0)
	{
		printf("select() timed out.\n");
		return EXIT_TIMEOUT;
	}
/*
	recvfrom(socksrv, buf, 0, 0, (struct sockaddr*)&addr, &addrlen);

	// Connect to client.
	if (connect(socksrv, (struct sockaddr*)&addr, addrlen) != EXIT_SUCCESS)
	{
		printf("connect() failed.\n");
		return EXIT_FAILURE;
	}

	// Display hostname and service.
	if (getnameinfo((struct sockaddr*)&addr, addrlen, hostname, NI_MAXHOST, service,
		NI_MAXSERV, NI_NUMERICHOST|NI_NUMERICSERV) != EXIT_SUCCESS)
	{
		printf("getnameinfo() failed.\n");
		return EXIT_FAILURE;
	}

	printf("Accepted connection from host %s and port %s.\n", hostname, service);
*/
	printf("Accepted connection.\n");

	*pSockCli = socksrv;

	return EXIT_SUCCESS;
}

/*
Wait for a client for an IPv4 TCP server.

SOCKET socksrv : (IN) Server socket.
SOCKET* pSockCli : (INOUT) Valid pointer that will receive the client socket.
int timeout : (IN) Max time to wait in ms (0 to disable timeout).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int waitforclifortcpsrv(SOCKET socksrv, SOCKET* pSockCli, int timeout)
{
	fd_set sock_set;
	int iResult = SOCKET_ERROR;
	struct timeval tv;
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(struct sockaddr_storage);
	char hostname[NI_MAXHOST];
	char service[NI_MAXSERV];

	tv.tv_sec = (long)(timeout/1000);
	tv.tv_usec = (long)((timeout%1000)*1000);

	*pSockCli = INVALID_SOCKET;

	// Initialize a fd_set and add the server socket to it.
	FD_ZERO(&sock_set);
	FD_SET(socksrv, &sock_set);

	// Wait for the readability of the socket in the fd_set, with a timeout.
	if (timeout != 0)
	{
		iResult = select((int)socksrv+1, &sock_set, NULL, NULL, &tv);
	}
	else
	{
		iResult = select((int)socksrv+1, &sock_set, NULL, NULL, NULL);
	}

	// Remove the server socket from the set.
	// No need to use FD_ISSET() here, as we only have one socket the return value of select() is
	// sufficient to know what happened.
	FD_CLR(socksrv, &sock_set);

	if (iResult == SOCKET_ERROR)
	{
		printf("select() failed.\n");
		return EXIT_FAILURE;
	}

	if (iResult == 0)
	{
		printf("select() timed out.\n");
		return EXIT_TIMEOUT;
	}

	*pSockCli = accept(socksrv, (struct sockaddr*)&addr, &addrlen);
	if (*pSockCli == INVALID_SOCKET)
	{
		printf("accept() failed.\n");
		return EXIT_FAILURE;
	}

	// Display hostname and service.
	if (getnameinfo((struct sockaddr*)&addr, addrlen, hostname, NI_MAXHOST, service,
		NI_MAXSERV, NI_NUMERICHOST|NI_NUMERICSERV) != EXIT_SUCCESS)
	{
		printf("getnameinfo() failed.\n");
		return EXIT_FAILURE;
	}

	printf("Accepted connection from host %s and port %s.\n", hostname, service);

	return EXIT_SUCCESS;
}
#ifdef _MSC_VER
// Restore the Visual Studio warnings previously disabled.
#pragma warning(default : 4127)
#endif // _MSC_VER

/*
Disconnect a client from an IPv4 UDP server.

SOCKET sock : (IN) Client socket to disconnect from the server.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int disconnectclifromudpsrv(SOCKET sock)
{
	// Shutdown the connection.
	if (shutdown(sock, SD_BOTH) != EXIT_SUCCESS)
	{
		printf("shutdown() failed.\n");
		//return EXIT_FAILURE;
	}

	// Destroy the socket created by socket().
	if (closesocket(sock) != EXIT_SUCCESS)
	{
		printf("closesocket() failed.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Disconnect a client from an IPv4 TCP server.

SOCKET sock : (IN) Client socket to disconnect from the server.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int disconnectclifromtcpsrv(SOCKET sock)
{
	// Shutdown the connection.
	if (shutdown(sock, SD_BOTH) != EXIT_SUCCESS)
	{
		printf("shutdown() failed.\n");
		//return EXIT_FAILURE;
	}

	// Destroy the socket created by socket().
	if (closesocket(sock) != EXIT_SUCCESS)
	{
		printf("closesocket() failed.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Stop an IPv4 UDP server.

SOCKET sock : (IN) Server socket.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int releaseudpsrv(SOCKET sock)
{
	// Shutdown the connection.
	if (shutdown(sock, SD_BOTH) != EXIT_SUCCESS)
	{
		printf("shutdown() failed.\n");
		//return EXIT_FAILURE;
	}

	// Destroy the socket created by socket().
	if (closesocket(sock) != EXIT_SUCCESS)
	{
		printf("closesocket() failed.\n");
		return EXIT_FAILURE;
	}

#ifdef _WIN32
	// Terminate use of the Winsock 2 DLL (Ws2_32.dll).
	if (WSACleanup() != EXIT_SUCCESS)
	{
		printf("WSAStartup() failed.\n");
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Stop an IPv4 TCP server.

SOCKET sock : (IN) Server socket.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int releasetcpsrv(SOCKET sock)
{
	// Shutdown the connection.
	if (shutdown(sock, SD_BOTH) != EXIT_SUCCESS)
	{
		printf("shutdown() failed.\n");
		//return EXIT_FAILURE;
	}

	// Destroy the socket created by socket().
	if (closesocket(sock) != EXIT_SUCCESS)
	{
		printf("closesocket() failed.\n");
		return EXIT_FAILURE;
	}

#ifdef _WIN32
	// Terminate use of the Winsock 2 DLL (Ws2_32.dll).
	if (WSACleanup() != EXIT_SUCCESS)
	{
		printf("WSAStartup() failed.\n");
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

#ifdef _MSC_VER
// Disable some Visual Studio warnings.
#pragma warning(disable : 4127)
#pragma warning(disable : 4702)
#endif // _MSC_VER
inline int LaunchUDPSrv(char* port, int (*handlecli)(SOCKET, void*), void* pParam)
{
	int iResult = EXIT_FAILURE;
	SOCKET socksrv = INVALID_SOCKET;
	SOCKET sockcli = INVALID_SOCKET;

	if (initudpsrv(&socksrv, "0.0.0.0", port, DEFAULT_SOCK_TIMEOUT) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	for (;;)
	{
		iResult = waitforcliforudpsrv(socksrv, &sockcli, DEFAULT_SOCK_TIMEOUT);
		switch (iResult)
		{
		case EXIT_SUCCESS:
			if (handlecli(sockcli, pParam) != EXIT_SUCCESS)
			{
				printf("Error while communicating with the client.\n");
			}
			if (disconnectclifromudpsrv(sockcli) != EXIT_SUCCESS)
			{
				releaseudpsrv(socksrv);
				return EXIT_FAILURE;
			}
			break;
		case EXIT_TIMEOUT:
			break;
		default:
			releaseudpsrv(socksrv);
			return EXIT_FAILURE;
		}
	}

	if (releaseudpsrv(socksrv) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int LaunchSingleCliTCPSrv(char* port, int (*handlecli)(SOCKET, void*), void* pParam)
{
	int iResult = EXIT_FAILURE;
	SOCKET socksrv = INVALID_SOCKET;
	SOCKET sockcli = INVALID_SOCKET;

	if (inittcpsrv(&socksrv, "0.0.0.0", port, 1, DEFAULT_SOCK_TIMEOUT) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	for (;;)
	{
		iResult = waitforclifortcpsrv(socksrv, &sockcli, DEFAULT_SOCK_TIMEOUT);
		switch (iResult)
		{
		case EXIT_SUCCESS:
			if (handlecli(sockcli, pParam) != EXIT_SUCCESS)
			{
				printf("Error while communicating with the client.\n");
			}
			if (disconnectclifromtcpsrv(sockcli) != EXIT_SUCCESS)
			{
				releasetcpsrv(socksrv);
				return EXIT_FAILURE;
			}
			break;
		case EXIT_TIMEOUT:
			break;
		default:
			releasetcpsrv(socksrv);
			return EXIT_FAILURE;
		}
	}

	if (releasetcpsrv(socksrv) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
#ifdef _MSC_VER
// Restore the Visual Studio warnings previously disabled.
#pragma warning(default : 4127)
#pragma warning(default : 4702)
#endif // _MSC_VER

#endif // OSNET_H
