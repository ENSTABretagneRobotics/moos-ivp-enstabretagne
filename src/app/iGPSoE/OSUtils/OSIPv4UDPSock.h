/***************************************************************************************************************:')

OSIPv4UDPSock.h

IPv4 UDP sockets handling.
You must call InitNet() before using any network function.

Fabrice Le Bars

Created : 2011-08-18

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

#ifndef OSIPV4UDPSOCK_H
#define OSIPV4UDPSOCK_H

#include "OSNet.h"

/*
Debug macros specific to OSIPv4UDPSock.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSIPV4UDPSOCK
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSIPV4UDPSOCK
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSIPV4UDPSOCK
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSIPV4UDPSOCK
#	define PRINT_DEBUG_MESSAGE_OSIPV4UDPSOCK(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSIPV4UDPSOCK(params)
#endif // _DEBUG_MESSAGES_OSIPV4UDPSOCK

#ifdef _DEBUG_WARNINGS_OSIPV4UDPSOCK
#	define PRINT_DEBUG_WARNING_OSIPV4UDPSOCK(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSIPV4UDPSOCK(params)
#endif // _DEBUG_WARNINGS_OSIPV4UDPSOCK

#ifdef _DEBUG_ERRORS_OSIPV4UDPSOCK
#	define PRINT_DEBUG_ERROR_OSIPV4UDPSOCK(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSIPV4UDPSOCK(params)
#endif // _DEBUG_ERRORS_OSIPV4UDPSOCK

/*
Create an IPv4 UDP socket.
Used at the beginning to create an IPv4 UDP server or client application.
InitNet() must be called before using any network function.

SOCKET* pSocket : (INOUT) Valid pointer that will receive the socket created.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int CreateSocketIPv4UDP(SOCKET* pSocket)
{
	int iResult = INVALID_SOCKET;

	*pSocket = socket(AF_INET, // Address family : AF_INET for IPv4, AF_INET6 for IPv6.
		SOCK_DGRAM, // Socket type : SOCK_STREAM for TCP, SOCK_DGRAM for UDP.
		IPPROTO_UDP); // Protocol to be used : IPPROTO_TCP for TCP, IPPROTO_UDP for UDP.

	if (*pSocket == INVALID_SOCKET)
	{
		PRINT_DEBUG_ERROR_OSSIPV4UDPSOCK(("CreateSocketIPv4UDP error (%s) : %s\n", 
			strtime_m(), WSAGetLastErrorMsg()));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Destroy an IPv4 UDP socket.
InitNet() must be called before using any network function.

SOCKET* pSocket : (INOUT) Valid pointer to the socket.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int DestroySocketIPv4UDP(SOCKET* pSocket)
{
	if (closesocket(*pSocket) != EXIT_SUCCESS)
	{
		PRINT_DEBUG_ERROR_OSSIPV4UDPSOCK(("DestroySocketIPv4UDP error (%s) : %s"
			"(pSocket=%#x)\n", 
			strtime_m(), 
			WSAGetLastErrorMsg(), 
			pSocket));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Disable sends or receives on an IPv4 UDP socket.

SOCKET* pSocket : (INOUT) Valid pointer to the socket.
int how : (IN) SD_RECEIVE if it stops receiving, SD_SEND if it stops sending, SD_BOTH for both.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int ShutdownIPv4UDP(SOCKET* pSocket, int how)
{
	int iResult = shutdown(*pSocket, how);
	if (iResult == SOCKET_ERROR)
	{
		PRINT_DEBUG_ERROR_OSIPV4UDPSOCK(("ShutdownIPv4UDP error (%s) : %s"
			"(pSocket=%#x, how=%d)\n", 
			strtime_m(), 
			WSAGetLastErrorMsg(), 
			pSocket, how));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}





inline int EnableBroadcastIPv4UDP(SOCKET sock, BOOL bEnableBroadcast)
{
	// setsockopt with SO_BROADCAST enabled. 
}

inline int BroadcastOnAllInterfacesIPv4UDP(SOCKET sock, char* sendbuf, UINT sendbuflen, char* port)
{
	INADDR_BROADCAST 
	sendto 

}

inline int BroadcastIPv4UDP(SOCKET sock, char* sendbuf, UINT sendbuflen, char* subnetbcastaddr, unsigned char port)
{
	 sockaddr_in RecvAddr;

 RecvAddr.sin_family = AF_INET; // IPv4
      RecvAddr.sin_port = htons(port);
      RecvAddr.sin_addr.s_addr = inet_addr(subnetbcastaddr);


	subnet broadcast address, e.g. 192.168.1.255

	sendto(sock, sendbuf,(int)sendbuflen, 0, (struct sockaddr*)&RecvAddr, sizeof(RecvAddr));

}


inline int SendToIPv4UDP(SOCKET* pSocket, char* address, char* port)
{
	int iResult = INVALID_SOCKET;

	*pSocket = socket(AF_INET, // Address family : AF_INET for IPv4, AF_INET6 for IPv6.
		SOCK_DGRAM, // Socket type : SOCK_STREAM for TCP sockets, SOCK_DGRAM for IPv4UDP sockets.
		IPPROTO_IPv4UDP); // Protocol to be used : IPPROTO_IPv4UDP for IPv4UDP sockets, IPPROTO_TCP for TCP sockets.

	if (*pSocket == INVALID_SOCKET)
	{
		PRINT_DEBUG_ERROR_OSSIPv4UDPSOCK(("SendToIPv4UDP error (%s) : %s"
			"(pSocket=%#x, address=%s, port=%s)\n", 
			strtime_m(), 
			WSAGetLastErrorMsg(), 
			pSocket, address, port));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


#endif // OSIPV4UDPSOCK_H
