/****************************************************************************
*
* Copyright (c) 2014 Wi-Fi Alliance
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
* SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
* RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
* NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
* USE OR PERFORMANCE OF THIS SOFTWARE.
*
*****************************************************************************/

/**
 * @file wfa_sock.c
 * @brief File containing socket routines on windows
*/

#include "wfa_main.h"

#define MAXPENDING 2    /* Maximum outstanding connection requests */

/** 
 * Initially create a TCP socket
 * @param port TCP socket port to listen.
 * @return socket id.
*/
int wfaCreateTCPServSock(unsigned short port)
{
	int sock;                        /* socket to create */
	struct sockaddr_in servAddr; /* Local address */
	const int on = 1;
	WSADATA wsaData;
	BOOL bOpt = TRUE;
	int wsaret=WSAStartup(0x101,&wsaData);

	if(wsaret!=0)
	{
		DPRINT_ERR(WFA_ERR, "createTCPServSock socket() failed");
		return WFA_FAILURE;
	}
	/* Create socket for incoming connections */
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		DPRINT_ERR(WFA_ERR, "createTCPServSock socket() failed");
		return WFA_FAILURE;
	}
	setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,(char*)&bOpt,sizeof(BOOL));
	/* Construct local address structure */
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;        /* Internet address family */
	servAddr.sin_port = htons(port);              /* Local port */
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));
	/* Bind to the local address */
	if (bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
	{
		DPRINT_ERR(WFA_ERR, "bind() failed");
		return WFA_FAILURE;
	}

	/* Mark the socket so it will listen for incoming connections */
	if (listen(sock, MAXPENDING) < 0)
	{
		DPRINT_ERR(WFA_ERR, "listen() failed");
		return WFA_FAILURE;
	}
	return sock;
}

/** 
 * Create a UDP socket
 * @param ipaddr TCP Local ip address for test traffic.
 * @param port UDP port to receive and send packet.
 * @return socket id.
*/
int wfaCreateUDPSock(char *ipaddr, unsigned short port)
{
	int udpsock;                 /* socket to create */
	struct sockaddr_in servAddr; /* Local address */

	WSADATA wsadata;
	int wsaret=WSAStartup(MAKEWORD(2,2),&wsadata);

	if(wsaret!=0)
	{
		int errsv = WSAGetLastError();
		DPRINT_ERR(WFA_ERR, "createUDPSock socket() falled with error %d",errsv);
		return WFA_FAILURE;
	}
	if((udpsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		int errsv = WSAGetLastError();
		DPRINT_ERR(WFA_ERR, "createUDPSock socket() failed with error %d for port %d",errsv,port);
		return WFA_FAILURE;
	}
	servAddr.sin_family      = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port        = htons(port);

	bind(udpsock, (struct sockaddr *) &servAddr, sizeof(servAddr)); 

	return udpsock;
}

/** 
 * Set the socket options for sending multicast traffic
 * @param sockfd The send socket id.
 * @return A status of the set operation.
*/
int wfaSetSockMcastSendOpt(int sockfd)
{
	unsigned char ttlval = 1;

	return setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL,(const char *) &ttlval, sizeof(ttlval));
}

/** 
 * Set the socket options for receiving multicast traffic
 * @param sockfd The receive socket id.
 * @param mcastgroup Multicast group IP address.
 * @return A status of the set operation.
*/
int wfaSetSockMcastRecvOpt(int sockfd, char *mcastgroup)
{
	struct ip_mreq mcreq;
	int so;

	mcreq.imr_multiaddr.s_addr = inet_addr(mcastgroup);
	mcreq.imr_interface.s_addr = htonl(INADDR_ANY);
	so = setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(const char *)&mcreq, sizeof(mcreq));

	return so;
}

/** 
 * Called to set up a virtual connection to the receiver
 * @param mysock Local socket number.
 * @param daddr Destination IP address.
 * @param dport Destination port.
 * @return Local socket number.
*/
int wfaConnectUDPPeer(int mysock, char *daddr, int dport)
{
	struct sockaddr_in peerAddr;

	memset(&peerAddr, 0, sizeof(peerAddr));
	peerAddr.sin_family = AF_INET;
	peerAddr.sin_port   = htons(dport);    

	connect(mysock, (struct sockaddr *)&peerAddr, sizeof(peerAddr));
	return mysock;
}

/** 
 * Handle and accept any incoming socket connection request
 * @param serSock The socket id to listen.
 * @return The connected socket id.
*/
int wfaAcceptTCPConn(int servSock)
{
	int clntSock;                /* Socket descriptor for client */
	struct sockaddr_in clntAddr; /* Client address */
	unsigned int clntLen;        /* Length of client address data structure */

	/* Set the size of the in-out parameter */
	clntLen = sizeof(clntAddr);

	/* Wait for a client to connect */
	if ((clntSock = accept(servSock, (struct sockaddr *) &clntAddr, 
		(int *)&clntLen)) < 0)
	{
		DPRINT_ERR(WFA_ERR, "accept() failed");
		exit(1);
	}

	/* clntSock is connected to a client! */
	return clntSock;
}

/** 
 * Set active socket ids for subsequent use by Select(), which will monitor whether there is any event for these ids
 * @param fdset The set of descriptors for subsequent use by select().
 * @param maxfdn1 The number of descriptors + 1.
 * @param fds Socket decriptors for main agent, control agent, traffic generator (wpa2, wmm, and wmm-ps).
*/
void wfaSetSockFiDesc(fd_set *fdset, int *maxfdn1, struct sockfds *fds) 
{
	FD_ZERO(fdset);
	if(fdset != NULL)
		FD_SET(*fds->agtfd, fdset);

	/* if the control agent socket fd valid */
	if(*fds->cafd >0)
	{
		FD_SET(*fds->cafd, fdset);
		*maxfdn1 = max(*maxfdn1-1, *fds->cafd) + 1;
	}


#ifdef WFA_WMM_PS
	/* if the power save socket port valid */
	if(*fds->psfd > 0)
	{
		FD_SET(*fds->psfd, fdset);
		*maxfdn1 = max(*maxfdn1-1, *fds->psfd) + 1;
	}
#endif

	return;
} 

/** 
 * Send control message/response through control link
 * @param sock The socket over which to send.
 * @param buf The data buffer to receive packets.
 * @param bufLen The length of data buffer.
 * @return The number of bytes sent.
*/
int wfaCtrlSend(SOCKET sock, unsigned char *buf, int bufLen)
{
	int bytesSent = 0;

	if(bufLen == 0)
	{
		DPRINT_WARNING(WFA_WNG, "Buffer Len is 0\n");
		return bufLen;
	}

	bytesSent = send(sock,( const char *)buf, bufLen, 0); 

	if(bytesSent == SOCKET_ERROR)
	{
		DPRINT_WARNING(WFA_WNG, "Error sending tcp packet %d\n", WSAGetLastError());
	}

	return bytesSent;
}

/** 
 * Receive control message/response through control link
 * @param sock The socket over which to receive.
 * @param buf The data buffer to receive packets.
 * @param bufLen The length of data buffer.
 * @return The number of bytes received.
*/
int wfaCtrlRecv(int sock, unsigned char *buf, int bufLen)
{
	int bytesRecvd = 0;
	if (buf == NULL || bufLen <= 0)
	{
		DPRINT_ERR(WFA_ERR, "Error on wfaCtrlRecv pass-in param errCd=%d\n", WSAGetLastError());
		return 0;
	}
	bytesRecvd = recv(sock,(char *) buf, bufLen, 0);

	return bytesRecvd; 
}

/** 
 * Send Traffic through through traffic interface
 * @param sock The socket over which to send packets.
 * @param buf The data buffer.
 * @param bufLen The length of data buffer.
 * @param to The address of the destination.
 * @return The number of bytes sent.
*/
int wfaTrafficSendTo(int sock, char *buf, int bufLen, struct sockaddr *to) 
{
	int bytesSent;

	bytesSent = sendto(sock, buf, bufLen, 0, to, sizeof(struct sockaddr));

	return bytesSent;
}

/** 
 * Receive traffic through through traffic interface
 * @param sock The socket over which packets are received.
 * @param buf The data buffer to hold the received packet.
 * @param from The address of sender of the data.
 * @return The number of bytes received.
*/
int wfaTrafficRecv(int sock, char *buf, struct sockaddr *from)
{
	int bytesRecvd =0;
	int addrLen=sizeof(struct sockaddr);


	int NonBlock=0; /* 0 is blocking */
	if (ioctlsocket(sock, FIONBIO, (u_long *)&NonBlock) == SOCKET_ERROR)
	{
		return SOCKET_ERROR;
	}

	/* check the buffer to avoid an uninitialized pointer - bugz 159 */
	if(buf == NULL)
	{
		DPRINT_ERR(WFA_ERR, "Uninitialized buffer\n");
		return WFA_ERROR;
	}
	bytesRecvd = recv(sock, buf, MAX_RCV_BUF_LEN, 0); 

	return bytesRecvd;
}

