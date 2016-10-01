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

#ifndef _WFA_SOCK_H
#define _WFA_SOCK_H

#define MAX_UDP_LEN        16384 
#define MAX_RCV_BUF_LEN  32*1024
#define MAX_ETH_PAYLOAD_LEN   1450
#define MAX_LEGACY_PAYLOAD_LEN    1000

#define MAX_SOCKOPT_RECVBUF_LEN  128*1024
#define MAX_SOCKOPT_SNDBUF_LEN  128*1024

#define SOCK_TYPE_UDP   0
#define SOCK_TYPE_TCP   1

struct sockfds
{
	int *agtfd;      /* dut agent main socket fd */
	int *cafd;       /* sock fd to control agent */
	int *tgfd;       /* traffic agent fd         */
	int *wmmfds;     /* wmm stream ids           */
	int *psfd;       /* wmm-ps socket id         */
};

extern int wfaCreateTCPServSockImpl(char *serverIpAddr, unsigned short port);
extern int wfaCreateTCPServSock(unsigned short sport);
extern int wfaCreateUDPSock(char *sipaddr, unsigned short sport);
extern int wfaCreateSock(int sockType, char *ipaddr, unsigned short port);

extern int wfaAcceptTCPConn(int servSock);

extern int wfaConnectToPeer(int sockType, int mysock, char *daddr, int dport);
extern int wfaConnectUDPPeer(int sock, char *dipaddr, int dport);

extern void wfaSetSockFiDesc(fd_set *sockset, int *, struct sockfds *);

extern int wfaCtrlSend(SOCKET sock, unsigned char *buf, int bufLen);
extern int wfaCtrlRecv(int sock, unsigned char *buf, int bufLen);
extern int wfaTrafficSendTo(int sock, char *buf, int bufLen, struct sockaddr *to);
extern int wfaTrafficRecv(int sock, char *buf, struct sockaddr *from, int bufLen);

extern int wfaSetSockMcastRecvOpt(int, char*);
extern int wfaSetSockMcastSendOpt(int);
extern int wfaSetProcPriority(int);

int wfaCreateTCPCliSock();
int wfaConnectTCPPeer(int mysock, char *daddr, int dport);

#endif /* _WFA_SOCK_H */
