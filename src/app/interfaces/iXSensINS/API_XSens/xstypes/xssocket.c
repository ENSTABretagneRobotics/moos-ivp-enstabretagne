#ifdef _WIN32
#include <winsock2.h>
#include <winerror.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#define SOCKET int32_t
#define SOCKET_ERROR -1
#define INVALID_SOCKET ((int32_t)-1)

/* capture the fact that windows has a specific closesocket function */
static int closesocket(SOCKET s)
{
	close(s);
	return 0;
}
#endif

#include <errno.h>

// MSG_NOSIGNAL is linux stuff
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#include "xssocket.h"

/* the socket data */
struct XsSocketPrivate
{
	SOCKET m_sd;

	enum NetworkLayerProtocol m_ipVersion;
	enum IpProtocol m_ipProtocol;

	struct sockaddr_storage m_remoteAddr;
	socklen_t m_remoteAddrLen;

	XsResultValue m_lastResult;
	int m_lastSystemError;

	XsDataFlags m_flags;

#ifdef _WIN32
	WSADATA m_sockData;
#endif
};

/* Return non-zero if the native socket s is usable */
static int socketIsUsable(SOCKET s)
{
	return (s != INVALID_SOCKET && s > 0);
}


/*!
 * \brief Get the IP address of \a remote.
 * \param[in] remote the socket address info of the remote peer
 * \param[in,out] address XsString to return the host IP address in.
 */
static void getRemoteHostAddress(const struct sockaddr_storage *remote, XsString *address)
{
	void *src;
	socklen_t length;

	switch (remote->ss_family)
	{
		case AF_INET:
		default:
			src = &(((struct sockaddr_in*)remote)->sin_addr);
			length = INET_ADDRSTRLEN;
			break;
		case AF_INET6:
			src = &(((struct sockaddr_in6*)remote)->sin6_addr);
			length = INET6_ADDRSTRLEN;
			break;
	}
	XsString_resize(address, length);
	if (inet_ntop(remote->ss_family, src, address->m_data, length) == NULL)
		XsString_erase(address, 0, address->m_size);
}

/* Update the last result of thisp to retval, if thisp is non-null

	Returns retval for easy use in return statements.
*/
static XsResultValue setLastResult(XsSocket* thisp, XsResultValue retval, int systemError)
{
	if (thisp && thisp->d)
	{
		thisp->d->m_lastResult = retval;
		thisp->d->m_lastSystemError = systemError;
	}
	return retval;
}

/* Translate a platform error into an XsResultValue

	If thisp is non-null, the socket's last result will be updated.
*/
static XsResultValue translateAndReturnSocketError(XsSocket* thisp, int functionResult)
{
	XsResultValue result;
	int err;
	if (functionResult >= 0)
		return setLastResult(thisp, XRV_OK, 0);
#ifdef _WIN32
	err = WSAGetLastError();
	switch (err)
	{
	case WSA_INVALID_HANDLE:
	case WSA_INVALID_PARAMETER:
	case WSAEINVAL:
		result  = XRV_INVALIDPARAM;
		break;
	case WSA_NOT_ENOUGH_MEMORY:
		result = XRV_OUTOFMEMORY;
		break;
	case WSA_OPERATION_ABORTED:
		result = XRV_ABORTED;
		break;
	case WSA_IO_INCOMPLETE:
	case WSA_IO_PENDING:
	case WSAEINTR:
	case WSAEBADF:
	case WSAEACCES:
	case WSAEFAULT:
	case WSAEMFILE:
	case WSAEWOULDBLOCK:
	case WSAEINPROGRESS:
	case WSAEALREADY:
	case WSAENOTSOCK:
	case WSAEDESTADDRREQ:
	case WSAEMSGSIZE:
	case WSAEPROTOTYPE:
	case WSAENOPROTOOPT:
	case WSAEPROTONOSUPPORT:
	case WSAESOCKTNOSUPPORT:
	case WSAEOPNOTSUPP:
	case WSAEPFNOSUPPORT:
	case WSAEAFNOSUPPORT:
	case WSAEADDRINUSE:
	case WSAEADDRNOTAVAIL:
	case WSAENETDOWN:
	case WSAENETUNREACH:
	case WSAENETRESET:
	case WSAECONNABORTED:
	case WSAECONNRESET:
	case WSAENOBUFS:
	case WSAEISCONN:
	case WSAENOTCONN:
	case WSAESHUTDOWN:
	case WSAETOOMANYREFS:
	case WSAETIMEDOUT:
	case WSAECONNREFUSED:
	case WSAELOOP:
	case WSAENAMETOOLONG:
	case WSAEHOSTDOWN:
	case WSAEHOSTUNREACH:
	case WSAENOTEMPTY:
	case WSAEPROCLIM:
	case WSAEUSERS:
	case WSAEDQUOT:
	case WSAESTALE:
	case WSAEREMOTE:
	case WSASYSNOTREADY:
	case WSAVERNOTSUPPORTED:
	case WSANOTINITIALISED:
	case WSAEDISCON:
	case WSAENOMORE:
	case WSAECANCELLED:
	case WSAEINVALIDPROCTABLE:
	case WSAEINVALIDPROVIDER:
	case WSAEPROVIDERFAILEDINIT:
	case WSASYSCALLFAILURE:
	case WSASERVICE_NOT_FOUND:
	case WSATYPE_NOT_FOUND:
	case WSA_E_NO_MORE:
	case WSA_E_CANCELLED:
	case WSAEREFUSED:
	case WSAHOST_NOT_FOUND:
	case WSATRY_AGAIN:
	case WSANO_RECOVERY:
	case WSANO_DATA:
	default:
		result = XRV_OTHER;
		break;
	}
#else
	err = errno;
	switch (err)
	{
	case EROFS:
		result = XRV_READONLY;
		break;
	case EACCES:
		result = XRV_INPUTCANNOTBEOPENED;
		break;
	case EADDRINUSE:
		result = XRV_ALREADYOPEN;
		break;
	case EBADF:
	case EINVAL:
	case ENOTDIR:
	case EFAULT:
		result = XRV_INVALIDPARAM;
		break;
	case ENAMETOOLONG:
		result = XRV_DATAOVERFLOW;
		break;
	case ENOTSOCK:
		result = XRV_UNSUPPORTED;
		break;
	case EADDRNOTAVAIL:
	case ENOENT:
		result = XRV_NOTFOUND;
		break;
	case ELOOP:
	case ENOMEM:
		result = XRV_OUTOFMEMORY;
		break;
	case ETIME:
	case ETIMEDOUT:
		result = XRV_TIMEOUTNODATA;
		break;
	default:
		result = XRV_OTHER;
		break;
	}
#endif
	return setLastResult(thisp, result, err);
}

static void translateSocketError(XsSocket* thisp, int functionResult)
{
	(void)translateAndReturnSocketError(thisp, functionResult);
}

/* Initialize the socket

	This function performs some basic initialization on the socket
*/
static void XsSocket_initialize(XsSocket* thisp, XsDataFlags flags)
{
	thisp->d = (struct XsSocketPrivate*)malloc(sizeof(struct XsSocketPrivate));
	memset(thisp->d, 0, sizeof(*thisp->d));
	thisp->d->m_sd = INVALID_SOCKET;
	thisp->d->m_flags = flags;
#ifdef _WIN32
	(void)WSAStartup(MAKEWORD(2,0), &thisp->d->m_sockData);
#endif
}

/*! \brief Create socket \a thisp for IP version \a ip with protocol \a protocol

	\param[in,out] thisp the socket to initialize
	\param[in] ip the IP version
	\param[in] protocol the protocol. Currently supported are TCP and UDP
	\relates XsSocket
*/
void XsSocket_create(XsSocket* thisp, enum NetworkLayerProtocol ip, enum IpProtocol protocol)
{
	XsSocket_initialize(thisp, XSDF_Managed);

	thisp->d->m_sd = socket(
			(ip == NLP_IPV4) ? PF_INET : PF_INET6,
			(protocol == IP_UDP) ? SOCK_DGRAM : SOCK_STREAM,
			(protocol == IP_UDP) ? IPPROTO_UDP : IPPROTO_TCP);

	thisp->d->m_ipVersion = ip;
	thisp->d->m_ipProtocol = protocol;
}

/* Create a socket from a native socket

	Usually we expect that theirinfo and infolen are filled in. If theirInfo is NULL though,
	we will fetch the information from the socket. Doing so by default would add a possible
	extra point of failure.
*/
void XsSocket_createFromNativeSocket(XsSocket* thisp, SOCKET nativeSocket, struct sockaddr *theirInfo, socklen_t infolen, XsDataFlags flags)
{
	XsSocket_initialize(thisp, flags);
	thisp->d->m_sd = nativeSocket;

	if (theirInfo)
	{
		memcpy(&thisp->d->m_remoteAddr, theirInfo, infolen);
		thisp->d->m_remoteAddrLen = infolen;
	}
	else
	{
		int rv;
		thisp->d->m_remoteAddrLen = sizeof(thisp->d->m_remoteAddr);
		rv = getpeername(thisp->d->m_sd, (struct sockaddr*)&thisp->d->m_remoteAddr, &thisp->d->m_remoteAddrLen);
		translateSocketError(thisp, rv);
	}

	switch (thisp->d->m_remoteAddr.ss_family)
	{
	case PF_INET6:
		thisp->d->m_ipVersion = NLP_IPV6;
		break;
	case PF_INET:
		thisp->d->m_ipVersion = NLP_IPV4;
		break;
	default:
		// IRDA and the likes. do we need it?
		break;
	}
}

/*!
 * \brief Create a socket from a native file descriptor.
 * \param[in,out] thisp the socket to initialize
 * \param[in] sockfd the file descriptor of the underlying socket
 * \param[in] flags flags to inicate if the underlying socket should be managed by this object
 * \relates XsSocket
 *
 * If the socket should be closed when this XsSocket is destroyed then the flags
 * should be set to XSDF_Managed.
 */
void XsSocket_createFromFileDescriptor(XsSocket* thisp, int sockfd, XsDataFlags flags)
{
	XsSocket_createFromNativeSocket(thisp, sockfd, NULL, 0, flags);
}

/*! \brief Return the native file descriptor
 *
 * The native socket descriptor returned from this function should only be
 * passed to third party libraries. However, it is possible to select, read and
 * write on it, if you know what you're doing. Do remember that this object
 * still manages the lifetime of the file descriptor.
 */
XSOCKET XsSocket_nativeDescriptor(XsSocket const* thisp)
{
	return thisp->d->m_sd;
}

/* Close a socket

	Theoretically, the socket should be reusable after this function is called.
*/
static XsResultValue XsSocket_close(XsSocket* thisp)
{
	if (thisp->d->m_sd == INVALID_SOCKET)
		return XRV_OK;

	if (closesocket(thisp->d->m_sd) == SOCKET_ERROR)
		return XRV_ERROR;

	thisp->d->m_sd = INVALID_SOCKET;
	return XRV_OK;
}

/*! \brief Destroy the given socket

	After calling this function, the XsSocket will no longer be usable for socket communications.

	It is safe to call this function twice on the same XsSocket.

	If the socket was created from a file descriptor using
	XsSocket_createFromFileDescriptor and had the XSDF_Managed flag set then
	the underlying socket will be closed.

	\param[in] thisp the socket to destroy
	\relates XsSocket
*/
void XsSocket_destroy(XsSocket* thisp)
{
	if (thisp->d)
	{
		if ((thisp->d->m_flags & XSDF_Managed) != 0)
		{
			(void)XsSocket_close(thisp);
		}
		free(thisp->d);
		thisp->d = NULL;
#ifdef _WIN32
		// decrease ref counter and possibly clean up socket dll
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ms741549%28v=vs.85%29.aspx
		(void)WSACleanup();
#endif
	}
}

/*! \brief Wait for read or write for mstimeout milliseconds on socket thisp
  \param[in] thisp socket to select on
  \param[in] mstimeout timeout in milliseconds
  \param[in,out] canRead if not null will be set to non-zero if the socket has data to read
  \param[in,out] canWrite if not null will be set to non-zero if the socket can be written to
  \returns -1 on error, 0 when no data is available, a positive number otherwise
  \relates XsSocket

  Use canRead and canWrite to determine whether you can read from or write to the socket.
  They will be set to non-zero if you can write, zero if you can't.
  If only one of canRead or canWrite points to non-null, a positive non-zero return value
  already indicates the filled in value is set to non-zero.
*/
int XsSocket_select(XsSocket* thisp, int mstimeout, int *canRead, int *canWrite)
{
	fd_set readfd;
	fd_set writefd;
	fd_set errorfd;
	struct timeval timeout;
	int rv;
	FD_ZERO(&readfd);
	FD_ZERO(&writefd);
	FD_ZERO(&errorfd);
	FD_SET(thisp->d->m_sd, &readfd);
	FD_SET(thisp->d->m_sd, &writefd);
	FD_SET(thisp->d->m_sd, &errorfd);

	if (canRead)
		*canRead = 0;
	if (canWrite)
		*canWrite = 0;

	timeout.tv_sec = mstimeout/1000;
	timeout.tv_usec = (mstimeout%1000) * 1000;

	rv = select(FD_SETSIZE, (canRead ? &readfd : NULL),
				(canWrite ? &writefd : NULL),
				&errorfd, &timeout);

	switch (rv)
	{
	case -1:
		translateSocketError(thisp, rv);
		break;
	case 0:
		(void)setLastResult(thisp, XRV_TIMEOUT, 0);
		break;
	default:
		if (FD_ISSET(thisp->d->m_sd, &errorfd))
		{
			(void)setLastResult(thisp, XRV_ERROR, 0);
			rv = -1;
			break;
		}

		if (canRead)
			*canRead = FD_ISSET(thisp->d->m_sd, &readfd);
		if (canWrite)
			*canWrite = FD_ISSET(thisp->d->m_sd, &writefd);
		break;
	}
	return rv;
}

/*! \brief Read \a size data into \a dest

	\param[in,out] thisp The socket to read data from
	\param[in,out] dest a pointer to a data buffer
	\param[in] size the size of the buffer \a dest points to
	\param[in] timeout the amount of time in milliseconds to wait for data

	\returns the size of the data read from the socket, -1 on error.
	\relates XsSocket
*/
int XsSocket_read(XsSocket* thisp, void* dest, XsSize size, int timeout)
{
	return XsSocket_readFrom(thisp, dest, (int)size, NULL, NULL, timeout);
}

#ifndef PEEKBUFSIZE
#define PEEKBUFSIZE 2048
#endif
/* peek at the size of the incoming data */
static int peekPendingDataSize(XsSocket* thisp)
{
	char buf[PEEKBUFSIZE];
	int rv;

	rv = recvfrom(thisp->d->m_sd, buf, PEEKBUFSIZE, MSG_PEEK, NULL, NULL);
	if (rv < 0)
		translateSocketError(thisp, rv);
	return rv;
}

/*! \brief Read \a size data into \a dest

	\param[in,out] thisp The socket to read data from
	\param[in,out] dest a pointer to a data buffer, may be NULL, in which case only the size of the pending data is reported.
	\param[in] size the size of the buffer \a dest points to
	\param[in,out] hostname a pointer to a string that contains the hostname of the sender after returning from this function. May be NULL.
	\param[in,out] port a pointer that will contain the port of the sender after returning from this function. May be NULL.
	\param[in] timeout the amount of time in milliseconds to wait for data

	\returns the size of the data read from the socket, -1 on error.
	\relates XsSocket
*/
int XsSocket_readFrom(XsSocket* thisp, void* dest, XsSize size, XsString* hostname, uint16_t* port, int timeout)
{
	int rv;
	int canRead;
	struct sockaddr_storage sender;
	socklen_t l = sizeof(sender);

	rv = XsSocket_select(thisp, timeout, &canRead, NULL);
	if (rv <= 0)
		return rv;

	if (!dest)
		return peekPendingDataSize(thisp);

	rv = recvfrom(thisp->d->m_sd, dest, (int)size, 0, (struct sockaddr *)&sender, &l);

	if (hostname)
	{
		getRemoteHostAddress(&sender, hostname);
	}

	if (port)
		*port = ntohs(((struct sockaddr_in*)&sender)->sin_port);

	translateSocketError(thisp, rv);
	return rv;
}

/*! \brief Write data to the socket

	\param[in,out] thisp the socket to write data to
	\param[in] data a pointer to the data to write
	\param[in] size the size of the data to write

	\returns the number of bytes written, -1 on error
	\relates XsSocket
*/
int XsSocket_write(XsSocket* thisp, const void* data, XsSize size)
{
	int canWrite;
	int rv = XsSocket_select(thisp, 0, NULL, &canWrite);
	if (rv <= 0)
		return rv;

	return send(thisp->d->m_sd, data, (int)size, MSG_NOSIGNAL);
}

/* Return non-zero if the hostname is actually an IPv4 address */
int isIPv4Address(const XsString* hostname)
{
	char *c;
	int expectNum = 1;
	int expectDot = 0;
	int numbersFound = 0;

	if (!hostname || !hostname->m_data)
		return 0;

	for (c = hostname->m_data; *c != '\0'; c++)
	{
		if (*c == '.')
		{
			if (!expectDot)
				return 0;
			expectNum = 1;
			numbersFound = 0;
			expectDot = 0;
		}
		else if (isdigit(*c))
		{
			if (!expectNum)
				return 0;
			expectDot = 1;
			++numbersFound;
			if (numbersFound == 3)
				expectNum = 0;
		}
		else
		{
			return 0;
		}
	}
	return 1;
}

/* Prefix the hostname with ::ffff: if we're on ipv6 and hostname looks like a ipv4 address */
void XsSocket_fixupHostname(XsSocket* thisp, XsString* hostname)
{
	if (!hostname || !hostname->m_data)
		return;

	if (thisp->d->m_ipVersion == NLP_IPV4)
		return;

	if (isIPv4Address(hostname))
	{
		const char prefix[] = "::ffff:";
		XsArray_insert(hostname, 0, strlen(prefix), prefix);
	}
}

typedef int (*lookupTestFunction)(XsSocket* thisp, SOCKET currentSocket, struct addrinfo* info);

/* Do a lookup of the given hostname and port

	This is an internal function that centralizes the lookup code.

	thisp  - the socket to use information from
	hostname - the hostname to look up
	port - the port to check the connection on
	hints_flags - the flags we should put into the lookup hints, e.g. AI_PASSIVE
	tester - a function that performs some sanity checks on the passed info
	info - a pointer to a sockaddr buffer. Should be at least as big as sockaddr, preferably be sockaddr_storage
	addrlen - the size of info. This function will change addrlen based on the actual size of info if necessary

	return OK or NOTFOUND
*/
static XsResultValue XsSocket_internalLookup(XsSocket* thisp, const XsString* hostname, uint16_t port,
						int hints_flags, lookupTestFunction tester,
						struct sockaddr* info, socklen_t* addrlen)
{
	struct addrinfo *lookupInfo, *p;
	SOCKET s;
	char gaport[7];
	struct addrinfo hints;
	int ret;
	memset(&hints, 0, sizeof(hints));

	switch (thisp->d->m_ipVersion)
	{
	case NLP_IPV6: hints.ai_family = AF_INET6;  break;
	case NLP_IPV4: hints.ai_family = AF_INET;   break;
	case NLP_IPVX: hints.ai_family = AF_UNSPEC; break;
	}
	hints.ai_socktype = (thisp->d->m_ipProtocol == IP_UDP) ? SOCK_DGRAM : SOCK_STREAM;
	hints.ai_flags = hints_flags;

	sprintf(gaport, "%u", port);
	if (hostname)
	{
		XsString host;

		XsString_construct(&host);
		XsArray_copy(&host, hostname);

		XsSocket_fixupHostname(thisp, &host);

		ret = getaddrinfo(host.m_data, gaport, &hints, &lookupInfo);

		XsString_destruct(&host);
	}
	else
	{
		ret = getaddrinfo(NULL, gaport, &hints, &lookupInfo);
	}

	if (ret) {
		switch (ret)
		{
		case EAI_BADFLAGS:
			return setLastResult(thisp, XRV_INVALIDPARAM, -1);
		case EAI_AGAIN:
			return translateAndReturnSocketError(thisp, EAGAIN);
		case EAI_FAIL:
			return setLastResult(thisp, XRV_ERROR, -1);
		case EAI_MEMORY:
			return setLastResult(thisp, XRV_INSUFFICIENTSPACE, -1);
		//case EAI_NODATA:
		//	return setLastResult(thisp, XRV_TIMEOUTNODATA, -1);
		case EAI_NONAME:
			return setLastResult(thisp, XRV_INSUFFICIENTDATA, -1);
		//case EAI_ADDRFAMILY:
		case EAI_SERVICE:
		case EAI_FAMILY:
		case EAI_SOCKTYPE:
			return setLastResult(thisp, XRV_UNSUPPORTED, -1);
		default:
			return translateAndReturnSocketError(thisp, ret);
		}
	}

	for (p = lookupInfo; p != NULL; p = p->ai_next)
	{
		s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (!socketIsUsable(s))
			continue;

		ret = tester(thisp, s, p);
		if (ret == 0)
		{
			(void)closesocket(s);
			if (info && addrlen)
			{
				if ((socklen_t)p->ai_addrlen < *addrlen)
					*addrlen = (socklen_t)p->ai_addrlen;
				memcpy(info, p->ai_addr, *addrlen);
			}
			break;
		}
		(void)closesocket(s);
	}
	freeaddrinfo(lookupInfo);
	return setLastResult(thisp, (ret == 0) ? XRV_OK : XRV_NOTFOUND, ret);
}

/* try and see if we can connect on currentSocket to the remote host */
static int defaultLookupTest(XsSocket* thisp, SOCKET currentSocket, struct addrinfo* info)
{
	(void)thisp;
	return connect(currentSocket, info->ai_addr, (int)info->ai_addrlen);
}

/*! \brief Perform a lookup */
static XsResultValue XsSocket_lookup(XsSocket* thisp, const XsString* hostname, uint16_t port, struct sockaddr* info, socklen_t* addrlen)
{
	return XsSocket_internalLookup(thisp, hostname, port, 0, defaultLookupTest, info, addrlen);
}

/*! \brief Write data to the socket to the host indicated by \a hostname, \a port

	\param[in,out] thisp the socket to write data to
	\param[in] data the data to write
	\param[in] size the size of the data
	\param[in] hostname the name of the host to send data to
	\param[in] port the port to send data to

	\returns the number of bytes written, -1 on error.
	\relates XsSocket
*/
int XsSocket_writeTo(XsSocket* thisp, const void* data, XsSize size, const XsString* hostname, uint16_t port)
{
	struct sockaddr_storage storage;
	struct sockaddr* addr = NULL;
	socklen_t addrlen = 0;
	int sent;
	int canWrite;
	int rv;

	rv = XsSocket_select(thisp, 0, NULL, &canWrite);
	if (rv <= 0)
		return rv;

	if (thisp->d->m_ipProtocol == IP_UDP)
	{
		addr = (struct sockaddr*)&storage;
		addrlen = sizeof(storage);

		if (XsSocket_lookup(thisp, hostname, port, addr, &addrlen) != XRV_OK)
			return -1;
	}

	sent = sendto(thisp->d->m_sd, data, (int)size, MSG_NOSIGNAL, addr, addrlen);
	translateSocketError(thisp, sent);
	return sent;
}

/*! \brief Flush the incoming data

  Flush all data from the incoming buffer.

  \param[in,out] thisp the socket to flush

  \relates XsSocket
*/
void XsSocket_flush(XsSocket* thisp)
{
	char buf[512];

	while (XsSocket_read(thisp, buf, sizeof(buf), 0) > 0)
	{
		/* nop */
	}
}

/*! \brief Accept an incoming connection

  This function requires that XsSocket_listen() has already been called.

  The returned pointer should be freed using XsSocket_freeAcceptedSocket().

  \param[in] thisp the socket to accept a connection on
  \param[in] mstimeout the timeout in milliseconds

  \returns a pointer to a newly created socket for the new connection. NULL on error.
  \relates XsSocket
  */
XsSocket* XsSocket_accept(XsSocket* thisp, int mstimeout)
{
	XsSocket *ns;
	struct sockaddr_storage theirInfo;
	socklen_t infoLength = sizeof(struct sockaddr_storage);
	SOCKET sd;

	if (mstimeout >= 0)
	{
		int read;
		int rv = XsSocket_select(thisp, mstimeout, &read, NULL);
		if (rv == 0)
		{
			(void)setLastResult(thisp, XRV_TIMEOUTNODATA, 0);
			return NULL;
		}
		else if (rv == -1)
		{
			translateSocketError(thisp, rv);
			return NULL;
		}
	}

	sd = accept(thisp->d->m_sd, (struct sockaddr *)&theirInfo, &infoLength);
	if (!socketIsUsable(sd))
		return NULL;

	ns = XsSocket_allocate();
	XsSocket_createFromNativeSocket(ns, sd, (struct sockaddr*)&theirInfo, infoLength, XSDF_Managed);
	return ns;
}

/*! \brief Dynamically allocate an XsSocket

	After calling this function, it is still required to call
	XsSocket_create or the internal function XsSocket_createFromNativeSocket

	Use XsSocket_free to safely free the returned XsSocket.

	\returns a pointer to a newly allocated XsSocket, or NULL on error
*/
XsSocket* XsSocket_allocate()
{
	return (XsSocket*)malloc(sizeof(XsSocket));
}

/*! \brief Free a socket returned from XsSocket_accept() or XsSocket_allocate()

	Use this function only on sockets returned from XsSocket_accept().

	This function calls XsSocket_destroy() before freeing the memory.

	\param[in] thisp the socket to free
	\see XsSocket_destroy
	\relates XsSocket
*/
void XsSocket_free(XsSocket* thisp)
{
	XsSocket_destroy(thisp);
	free(thisp);
}

/*! \brief Changes the value of a socket option
 *	\param[in] thisp the socket to change the option for
 *	\param[in] option the socket option to change
 *	\param[in] valuePtr poins to the value the option must be set to
 *	\param[in] valueSize the size of the value \a valuePtr points to
 *	\return an XsResultValue indicating the result of the operation, possibly pointing towards a cause
 *	\relates XsSocket
 */
XsResultValue XsSocket_setSocketOption(XsSocket *thisp, enum XsSocketOption option, void* valuePtr, int valueSize)
{
	int res;
	int nativeOption;
#ifdef _WIN32
	const char *valPtr = (const char*)valuePtr;
#else
	const void *valPtr = valuePtr;
#endif

	switch(option)
	{
		case XSO_ReuseAddress:
			nativeOption = SO_REUSEADDR;
			break;

		default:
			return XRV_OK;
	}

	res = setsockopt(thisp->d->m_sd, SOL_SOCKET, nativeOption, valPtr, valueSize);
	return translateAndReturnSocketError(thisp, res);
}


/* test if we can bind to info

	keeps the bind alive after leaving the function
*/
static int binder(XsSocket* thisp, SOCKET currentSocket, struct addrinfo* info)
{
	int res;
	int yesval = 1;
#ifdef _WIN32
	const char *yes = (const char*)&yesval;
#else
	const void *yes = &yesval;
#endif

	(void)currentSocket;

	res = setsockopt(thisp->d->m_sd, SOL_SOCKET, SO_REUSEADDR, yes, sizeof(yesval));
	if (res)
		return res;

	return bind(thisp->d->m_sd, info->ai_addr, (int)info->ai_addrlen);
}

/*! \brief Bind to the \a hostname and \a port combination

	\param[in] thisp the socket to bind on
	\param[in] hostname the hostname to bind to, may be NULL, in which we will bind to any address
	\param[in] port the port to bind to. Must be a value above 1023, unless we have elevated privileges

	\return an XsResultValue indicating the result of the operation, possibly pointing towards a cause
	\relates XsSocket
*/
XsResultValue XsSocket_bind(XsSocket* thisp, const XsString* hostname, uint16_t port)
{
	int rv;
	struct sockaddr_storage s;
	socklen_t addrlen;

	if (hostname)
		return XsSocket_internalLookup(thisp, hostname, port, AI_PASSIVE, binder, NULL, NULL);

	memset(&s, 0, sizeof(s));

	if (thisp->d->m_ipVersion == NLP_IPV4)
	{
		struct sockaddr_in *sin = (struct sockaddr_in*)&s;
		sin->sin_family = AF_INET;
		sin->sin_port = htons(port);
		sin->sin_addr.s_addr = INADDR_ANY;
		addrlen = sizeof(*sin);
	}
	else
	{
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)&s;
		sin6->sin6_family = AF_INET6;
		sin6->sin6_port = htons(port);
		sin6->sin6_addr = in6addr_any;
		addrlen = sizeof(*sin6);
	}

	rv = bind(thisp->d->m_sd, (struct sockaddr*)&s, addrlen);
	return translateAndReturnSocketError(thisp, rv);
}

/*! \brief Start listening for incoming connections on this socket

	\param[in,out] thisp the socket to start listening on
	\param[in] maxPending the maximum amount of pending connections

	\return an XsResultValue indicating the result of the operation
	\relates XsSocket
*/
XsResultValue XsSocket_listen(XsSocket* thisp, int maxPending)
{
	int r = listen(thisp->d->m_sd, maxPending);
	return translateAndReturnSocketError(thisp, r);
}

/* Connect to info

	Keep the connection alive
*/
static int connector(XsSocket* thisp, SOCKET currentSocket, struct addrinfo* info)
{
	int ret;
	(void)currentSocket;

	ret = connect(thisp->d->m_sd, info->ai_addr, (int)info->ai_addrlen);

	if (ret == 0)
		thisp->d->m_ipVersion = (info->ai_family == AF_INET ? NLP_IPV4 : NLP_IPV6);
	else
		translateSocketError(thisp, ret);
	return ret;
}

/*! \brief Connect to \a hostame on \a port

	\param[in,out] thisp the socket to connect to
	\param[in] host the hostname to connect to
	\param[in] port the port to connect to

	\returns an XsResultValue indicating the result of the operation
	\relates XsSocket
*/
XsResultValue XsSocket_connect(XsSocket* thisp, const XsString* host, uint16_t port)
{
	return XsSocket_internalLookup(thisp, host, port, 0, connector, NULL, NULL);
}

/*! \brief Return non-zero if the socket is usable for reading and writing data

	\param[in] thisp the socket to check

	\returns non-zero if the socket is usable for reading and writing data, zero otherwise
	\relates XsSocket
*/
int XsSocket_isUsable(const XsSocket* thisp)
{
	return socketIsUsable(thisp->d->m_sd);
}

/*!
 * \brief Get the numeric IP address of remote host of this socket.
 * \param[in] thisp the socket to get the remote address for
 * \param[in,out] address XsString to return the remote host IP address in
 */
void XsSocket_getRemoteAddress(const XsSocket* thisp, XsString *address)
{
	getRemoteHostAddress(&thisp->d->m_remoteAddr, address);
}
