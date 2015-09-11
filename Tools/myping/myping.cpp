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

/*****************************************************************************
Copyright (c) Microsoft
All rights reserved.
Licensed under the Microsoft Limited Public License (the “License”); you may not
use this file except in compliance with the License.
You may obtain a copy of the License at http://msdn.microsoft.com/en-us/cc300389.

THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS
OF TITLE, FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.

See the Microsoft Limited Public License (Ms-LPL) for specific language governing
permissions and limitations under the License.
*****************************************************************************/

/*
* This sample illustrates how to use raw sockets to send ICMP
* echo requests and receive their response. This sample performs
* both IPv4 and IPv6 ICMP echo requests. When using raw sockets,
* the protocol value supplied to the socket API is used as the
* protocol field (or next header field) of the IP packet. Then
* as a part of the data submitted to sendto, we include both
* the ICMP request and data.

* For IPv4 the IP record route option is supported via the IP_OPTIONS socket option.

* Added wait time paramerter and interval to match with CAPI.
* Changed from blocking to non-blocking sockets.
*/

#ifdef _IA64_
#pragma warning (disable: 4267)
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#include "resolve.h"
#include "iphdr.h"

#define TG_WMM_AC_BE          1      /* Best Effort */
#define TG_WMM_AC_BK          2      /* Background  */
#define TG_WMM_AC_VI          3      /* Video       */
#define TG_WMM_AC_VO          4      /* Voice       */
#define TG_WMM_AC_UAPSD       5      /* UAPSD        */

/* wmm defs */
#define TOS_VO7     0xE0         // 111 0  0000 (7)  AC_VO tos/dscp values
#define TOS_VO      0xC0         // 110 0  0000 (6)  AC_VO tos/dscp values

#define TOS_VI      0xA0         // 101 0  0000 (5)  AC_VI
#define TOS_VI4     0x80         // 100 0  0000 (4)  AC_VI

#define TOS_BE      0x00         // 000 0  0000 (0)  AC_BE
#define TOS_EE      0x60         // 011 0  0000 (3)  AC_BE

#define TOS_BK      0x20         // 001 0  0000 (1)  AC_BK
#define TOS_LE      0x40         // 010 0  0000 (2)  AC_BK

#define DEFAULT_DATA_SIZE      32       // default data size
#define DEFAULT_RECV_TIMEOUT   6000     // six second
#define DEFAULT_TTL            128
#define MAX_RECV_BUF_LEN       0xFFFF   // Max incoming packet size.

#define DEFAULT_INTERVAL       1000		// Default Inteval between each packet, 1sec.
#define DEFAULT_MAX_COUNT       4		// Default number of Ping packets.
#define DEFAULT_WAIT_TIME       500  // Default Wait Time for each ping packet 500msec.

int   gAddressFamily=AF_UNSPEC,         // Address family to use
    gProtocol=IPPROTO_ICMP,           // Protocol value
    gTtl=DEFAULT_TTL,					// Default TTL value
    gInteval=DEFAULT_INTERVAL,		// Default Interval value
    gMaxCount=DEFAULT_MAX_COUNT,		// Default Number of pings
    gWaitTime=DEFAULT_WAIT_TIME;		// Default Wait Time for reply pings

int   gDataSize=DEFAULT_DATA_SIZE;      // Amount of data to send
BOOL  bRecordRoute=FALSE;               // Use IPv4 record route?
char *gDestination=NULL,                // Destination
    recvbuf[MAX_RECV_BUF_LEN];        // For received packets
int   recvbuflen = MAX_RECV_BUF_LEN;    // Length of received packets.
int DEFAULT_SEND_COUNT = 4,sendi=0,recd=0;
int tosprio = 0;
int vend;

FILE *resultFile = NULL;

int strncasecmp(const char *s1, const char *s2, size_t n);
int strcasecmp(const char *s1, const char *s2);

//
// Function: usage
//
// Description:
//    Print usage information.
//
void usage(char *progname)
{
    printf("usage: %s [options] <host> \n", progname);
    printf("        host        Remote machine to ping\n");
    printf("        options: \n");
    printf("            -a 4|6       Address family (default: AF_UNSPEC)\n");
    printf("            -i ttl       Time to live (default: 128) \n");
    printf("            -l bytes     Amount of data to send (default: 32) \n");
    printf("            -r           Record route (IPv4 only)\n");
    printf("            -f           File name for output\n");
    printf("            -p           QoS packet priority\n");
    printf("            -d           wifi device provider\n");

    return;
}

// 
// Function: InitIcmpHeader
//
// Description:
//    Helper function to fill in various stuff in our ICMP request.
//
void InitIcmpHeader(char *buf, int datasize)
{
    ICMP_HDR   *icmp_hdr=NULL;
    char       *datapart=NULL;

    icmp_hdr = (ICMP_HDR *)buf;
    icmp_hdr->icmp_type     = ICMPV4_ECHO_REQUEST_TYPE;        // request an ICMP echo
    icmp_hdr->icmp_code     = ICMPV4_ECHO_REQUEST_CODE;
    icmp_hdr->icmp_id       = (USHORT)GetCurrentProcessId();
    icmp_hdr->icmp_checksum = 0;
    icmp_hdr->icmp_sequence = 0;

    datapart = buf + sizeof(ICMP_HDR);
    //
    // Place some data in the buffer.
    //
    memset(datapart, 'E', datasize);
}

// For decoding the reply

int CheckIcmpReplyHeader(char *buf, int bytes)
{
    int     hdrlen=0;

    IPV4_OPTION_HDR *v4opt=NULL;
    IPV4_HDR        *v4hdr=NULL;
    ICMP_HDR * icmphdr;

    UNREFERENCED_PARAMETER(bytes);

    printf("\nInside ICMP header decode");
    if (gAddressFamily == AF_INET )
    {
        v4hdr = (IPV4_HDR *)buf;
        hdrlen = (v4hdr->ip_verlen & 0x0F) * 4;

        icmphdr =(ICMP_HDR *) ( buf + hdrlen );
        printf("\nReply type - calculation");				
        printf("\nReply type - %d",icmphdr->icmp_type);

        if( icmphdr->icmp_type == ICMPV4_ECHO_REPLY_TYPE)
        {
            printf("\nReply type - Echo reply");				
            return 0;
        }
        if( icmphdr->icmp_type == 3)
        {
            printf("\n Reply type - Host not reachable");				
            return 3;
        }

        printf("\n Reply type - Unknown");
        return icmphdr->icmp_type;
    }

    return -1;
}

//
// Function: InitIcmp6Header
//
// Description:
//    Initialize the ICMP6 header as well as the echo request header.
//
int InitIcmp6Header(char *buf, int datasize)
{
    ICMPV6_HDR          *icmp6_hdr=NULL;
    ICMPV6_ECHO_REQUEST *icmp6_req=NULL;
    char                *datapart=NULL;

    // Initialize the ICMP6 headerf ields
    icmp6_hdr = (ICMPV6_HDR *)buf;
    icmp6_hdr->icmp6_type     = ICMPV6_ECHO_REQUEST_TYPE;
    icmp6_hdr->icmp6_code     = ICMPV6_ECHO_REQUEST_CODE;
    icmp6_hdr->icmp6_checksum = 0;

    // Initialize the echo request fields
    icmp6_req = (ICMPV6_ECHO_REQUEST *)(buf + sizeof(ICMPV6_HDR));
    icmp6_req->icmp6_echo_id       = (USHORT)GetCurrentProcessId();
    icmp6_req->icmp6_echo_sequence = 0;

    datapart = (char *)buf + sizeof(ICMPV6_HDR) + sizeof(ICMPV6_ECHO_REQUEST);

    memset(datapart, '#', datasize);

    return (sizeof(ICMPV6_HDR) + sizeof(ICMPV6_ECHO_REQUEST));
}

// 
// Function: checksum
//
// Description:
//    This function calculates the 16-bit one's complement sum
//    of the supplied buffer (ICMP) header.
//
USHORT checksum(USHORT *buffer, int size) 
{
    unsigned long cksum=0;

    while (size > 1) 
    {
        cksum += *buffer++;
        size -= sizeof(USHORT);
    }
    if (size) 
    {
        cksum += *(UCHAR*)buffer;
    }
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >>16);
    return (USHORT)(~cksum);
}

//
// Function: ValidateArgs
//
// Description:
//    Parse the command line arguments.
//
BOOL ValidateArgs(int argc, char **argv)
{
    int                i;
    BOOL               isValid = FALSE;
    char *priostr, *dname;

    for(i=1; i < argc ;i++)
    {
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            switch (tolower(argv[i][1]))
            {
            case 'a':        // address family
                if (i+1 >= argc)
                {
                    usage(argv[0]);
                    goto CLEANUP;
                }
                if (argv[i+1][0] == '4')
                    gAddressFamily = AF_INET;
                else if (argv[i+1][0] == '6')
                    gAddressFamily = AF_INET6;
                else
                {
                    usage(argv[0]);
                    goto CLEANUP;
                }

                i++;
                break;

            case 'i':        // Set inteval value
                printf("Inteval i\n");
                if (i+1 >= argc)
                {
                    usage(argv[0]);
                    goto CLEANUP;
                }

                gInteval = (int)((atoi(argv[++i])) * 0.2);
                printf("Inteval %d\n",gInteval);

                gWaitTime = (int)((atoi(argv[i])) * 0.8);
                printf("Wait time %d\n",gWaitTime);
                break;

            case 'l':        // buffer size tos end
                if (i+1 >= argc)
                {
                    usage(argv[0]);
                    goto CLEANUP;
                }

                gDataSize = atoi(argv[++i]);
                break;

            case 'n':        //number of peckets
                if (i+1 >= argc)
                {
                    usage(argv[0]);
                    goto CLEANUP;
                }

                gMaxCount = atoi(argv[++i]);
                break;

            case 'w':        //wait time
                if (i+1 >= argc)
                {
                    usage(argv[0]);
                    goto CLEANUP;
                }

                gWaitTime = atoi(argv[++i]);
                break;

            case 'r':        // record route option
                bRecordRoute = TRUE;
                break;

            case 'f':        // write into File
                if (i+1 >= argc)
                {
                    usage(argv[0]);
                    goto CLEANUP;
                }
                resultFile = fopen(argv[++i],"w");
                break;

            case 'p':       // traffic priority
                if(i+1 >= argc)
                {
                    usage(argv[0]);
                    goto CLEANUP;
                }
                priostr = argv[++i];

                if(atoi(priostr) > 5)
                    tosprio = TG_WMM_AC_BE;
                else	
                    tosprio = atoi(priostr);
                break;

            case 'd':
                if(i+1 >= argc)
                {
                    usage(argv[0]);
                    goto CLEANUP;
                }
                dname = argv[++i];

                vend = atoi(dname);
                break;

            default:
                usage(argv[0]);
                goto CLEANUP;
            }
        }
        else
        {
            gDestination = argv[i];
        }
    }

    isValid = TRUE;

CLEANUP:    
    return isValid;
}

//
// Function: SetIcmpSequence
//
// Description:
//    This routine sets the sequence number of the ICMP request packet.
//
void SetIcmpSequence(char *buf)
{
    ULONG    sequence=0;

    sequence = GetTickCount();
    if (gAddressFamily == AF_INET)
    {
        ICMP_HDR    *icmpv4=NULL;

        icmpv4 = (ICMP_HDR *)buf;

        icmpv4->icmp_sequence = (USHORT)sequence;
    }
    else if (gAddressFamily == AF_INET6)
    {
        ICMPV6_HDR          *icmpv6=NULL;
        ICMPV6_ECHO_REQUEST *req6=NULL;

        icmpv6 = (ICMPV6_HDR *)buf;
        req6   = (ICMPV6_ECHO_REQUEST *)(buf + sizeof(ICMPV6_HDR));

        req6->icmp6_echo_sequence = (USHORT)sequence;
    }
}

//
// Function: ComputeIcmp6PseudoHeaderChecksum
//
// Description:
//    This routine computes the ICMP6 checksum which includes the pseudo
//    header of the IPv6 header (see RFC2460 and RFC2463). The one difficulty
//    here is we have to know the source and destination IPv6 addresses which
//    will be contained in the IPv6 header in order to compute the checksum.
//    To do this we call the SIO_ROUTING_INTERFACE_QUERY ioctl to find which
//    local interface for the outgoing packet.
//
USHORT ComputeIcmp6PseudoHeaderChecksum(SOCKET s, char *icmppacket, int icmplen, struct addrinfo *dest)
{
    SOCKADDR_STORAGE localif;
    DWORD            bytes;
    char             tmp[MAX_RECV_BUF_LEN] = {'\0'},
        *ptr=NULL,
        proto=0;
    int              rc, total, length, i;

    // Find out which local interface for the destination
    rc = WSAIoctl(
        s,
        SIO_ROUTING_INTERFACE_QUERY,
        dest->ai_addr,
        (DWORD) dest->ai_addrlen,
        (SOCKADDR *) &localif,
        (DWORD) sizeof(localif),
        &bytes,
        NULL,
        NULL
        );
    if (rc == SOCKET_ERROR)
    {
        fprintf(stderr, "WSAIoctl failed: %d\n", WSAGetLastError());
        return 0xFFFF;
    }

    // We use a temporary buffer to calculate the pseudo header. 
    ptr = tmp;
    total = 0;

    // Copy source address
    memcpy(ptr, &((SOCKADDR_IN6 *)&localif)->sin6_addr, sizeof(struct in6_addr));
    ptr   += sizeof(struct in6_addr);
    total += sizeof(struct in6_addr);    

    // Copy destination address
    memcpy(ptr, &((SOCKADDR_IN6 *)dest->ai_addr)->sin6_addr, sizeof(struct in6_addr));
    ptr   += sizeof(struct in6_addr);
    total += sizeof(struct in6_addr);

    // Copy ICMP packet length
    length = htonl(icmplen);

    memcpy(ptr, &length, sizeof(length));
    ptr   += sizeof(length);
    total += sizeof(length);   

    // Zero the 3 bytes
    memset(ptr, 0, 3);
    ptr   += 3;
    total += 3;

    // Copy next hop header
    proto = IPPROTO_ICMP6;

    memcpy(ptr, &proto, sizeof(proto));
    ptr   += sizeof(proto);
    total += sizeof(proto);

    // Copy the ICMP header and payload
    memcpy(ptr, icmppacket, icmplen);
    ptr   += icmplen;
    total += icmplen;

    for(i=0; i < icmplen%2 ;i++)
    {
        *ptr = 0;
        ptr++;
        total++;
    }

    return checksum((USHORT *)tmp, total);
}

//
// Function: ComputeIcmpChecksum
//
// Description:
//    This routine computes the checksum for the ICMP request. For IPv4 its
//    easy, just compute the checksum for the ICMP packet and data. For IPv6,
//    its more complicated. The pseudo checksum has to be computed for IPv6
//    which includes the ICMP6 packet and data plus portions of the IPv6
//    header which is difficult since we aren't building our own IPv6
//    header.
//
void ComputeIcmpChecksum(SOCKET s, char *buf, int packetlen, struct addrinfo *dest)
{
    if (gAddressFamily == AF_INET)
    {
        ICMP_HDR    *icmpv4=NULL;

        icmpv4 = (ICMP_HDR *)buf;
        icmpv4->icmp_checksum = 0;
        icmpv4->icmp_checksum = checksum((USHORT *)buf, packetlen);
    }
    else if (gAddressFamily == AF_INET6)
    {
        ICMPV6_HDR  *icmpv6=NULL;

        icmpv6 = (ICMPV6_HDR *)buf;
        icmpv6->icmp6_checksum = 0;
        icmpv6->icmp6_checksum = ComputeIcmp6PseudoHeaderChecksum(
            s,
            buf,
            packetlen,
            dest
            );
    }
}

//
// Function: PostRecvfrom
//
// Description:
//    This routine posts an overlapped WSARecvFrom on the raw socket.
//
int PostRecvfrom(SOCKET s, char *buf, int buflen, SOCKADDR *from, int *fromlen, WSAOVERLAPPED *ol)
{
    WSABUF  wbuf;
    DWORD   flags,
        bytes;
    int     rc;

    wbuf.buf = buf;
    wbuf.len = buflen;

    flags = 0;

    rc = WSARecvFrom(
        s,
        &wbuf,
        1,
        &bytes,
        &flags,
        from,
        fromlen,
        ol,
        NULL
        );
    if (rc == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            fprintf(stderr, "WSARecvFrom failed: %d\n", WSAGetLastError());
            return SOCKET_ERROR;
        }
    }
    return NO_ERROR;
}

//
// Function: PrintPayload
// 
// Description:
//    This routine is for IPv4 only. It determines if there are any IP options
//    present (by seeing if the IP header length is greater than 20 bytes) and
//    if so it prints the IP record route options.
//
void PrintPayload(char *buf, int bytes)
{
    int hdrlen=0, routes=0, i;

    UNREFERENCED_PARAMETER(bytes);

    if (gAddressFamily == AF_INET)
    {
        SOCKADDR_IN      hop;
        IPV4_OPTION_HDR *v4opt=NULL;
        IPV4_HDR        *v4hdr=NULL;

        hop.sin_family = (USHORT)gAddressFamily;
        hop.sin_port   = 0;

        v4hdr = (IPV4_HDR *)buf;
        hdrlen = (v4hdr->ip_verlen & 0x0F) * 4;

        // If the header length is greater than the size of the basic IPv4
        //    header then there are options present. Find them and print them.
        if (hdrlen > sizeof(IPV4_HDR))
        {
            v4opt = (IPV4_OPTION_HDR *)(buf + sizeof(IPV4_HDR));
            routes = (v4opt->opt_ptr / sizeof(ULONG)) - 1;
            for(i=0; i < routes ;i++)
            {
                hop.sin_addr.s_addr = v4opt->opt_addr[i];

                // Print the route
                if (i == 0)
                    printf("    Route: ");
                else
                    printf("           ");
                PrintAddress((SOCKADDR *)&hop, sizeof(hop));

                if (i < routes-1)
                    printf(" ->\n");
                else
                    printf("\n");
            }
        }
    }
    return;
}

//
// Function: SetTtl
//
// Description:
//    Sets the TTL on the socket.
//
int SetTtl(SOCKET s, int ttl)
{
    int     optlevel = 0,
        option = 0,
        rc;

    rc = NO_ERROR;
    if (gAddressFamily == AF_INET)
    {
        optlevel = IPPROTO_IP;
        option   = IP_TTL;
    }
    else if (gAddressFamily == AF_INET6)
    {
        optlevel = IPPROTO_IPV6;
        option   = IPV6_UNICAST_HOPS;
    }
    else
    {
        rc = SOCKET_ERROR;
    }
    if (rc == NO_ERROR)
    {
        rc = setsockopt(
            s,
            optlevel,
            option,
            (char *)&ttl,
            sizeof(ttl)
            );
    }
    if (rc == SOCKET_ERROR)
    {
        fprintf(stderr, "SetTtl: setsockopt failed: %d\n", WSAGetLastError());
    }
    return rc;
}

//
// Function: main
//
// Description:
//    Setup the ICMP raw socket and create the ICMP header. Add
//    the appropriate IP option header and start sending ICMP
//    echo requests to the endpoint. For each send and receive we
//    set a timeout value so that we don't wait forever for a 
//    response in case the endpoint is not responding. When we
//    receive a packet decode it.
//
int __cdecl main(int argc, char **argv)
{

    WSADATA            wsd;
    WSAOVERLAPPED      recvol;
    SOCKET             s=INVALID_SOCKET;
    char              *icmpbuf=NULL;
    struct addrinfo   *dest=NULL,
        *local=NULL;
    IPV4_OPTION_HDR    ipopt;
    SOCKADDR_STORAGE   from;
    DWORD              bytes,
        flags;
    int                packetlen=0,
        fromlen,
        time=0,
        rc,
        i,
        status = 0;

    int sockBuff;
    int sockBuffLen = sizeof(int) ;
    u_long iMode = 1;
    BOOL bLinger= 0;


    recvol.hEvent = WSA_INVALID_EVENT;

    // Parse the command line
    if (ValidateArgs(argc, argv) == FALSE)
    {
        // invalid arguments supplied.
        status = -1;
        goto EXIT;
    }

    // Load Winsock
    if ((rc = WSAStartup(MAKEWORD(2,2), &wsd)) != 0)
    {
        printf("WSAStartup() failed: %d\n", rc);
        status = -1;
        goto EXIT;
    }

    // Resolve the destination address
    dest = ResolveAddress(
        gDestination,
        "0",
        gAddressFamily,
        0,
        0
        );

    if (dest == NULL)
    {
        printf("bad name %s\n", gDestination);
        status = -1;
        goto CLEANUP;
    }
    gAddressFamily = dest->ai_family;

    if (gAddressFamily == AF_INET)
        gProtocol = IPPROTO_ICMP;
    else if (gAddressFamily == AF_INET6)
        gProtocol = IPPROTO_ICMP6;

    // Get the bind address
    local = ResolveAddress(
        NULL,
        "0",
        gAddressFamily,
        0,
        0
        );

    if (local == NULL)
    {
        printf("Unable to obtain the bind address!\n");
        status = -1;
        goto CLEANUP;
    }

    // Create the raw socket    
    s = WSASocket(AF_INET, SOCK_RAW,gProtocol,NULL,0, WSA_FLAG_OVERLAPPED);

    if (s == INVALID_SOCKET) 
    {
        printf("socket failed: %d\n", WSAGetLastError());
        status = -1;
        goto CLEANUP;
    }

    ioctlsocket(s, FIONBIO, &iMode);

    setsockopt(s,SOL_SOCKET,SO_DONTLINGER,(char *)&bLinger,sizeof(bLinger));
    SetTtl(s, gTtl);

    // Figure out the size of the ICMP header and payload
    if (gAddressFamily == AF_INET)
        packetlen += sizeof(ICMP_HDR);
    else if (gAddressFamily == AF_INET6)
        packetlen += sizeof(ICMPV6_HDR) + sizeof(ICMPV6_ECHO_REQUEST);

    // Add in the data size
    packetlen += gDataSize;

    // Allocate the buffer that will contain the ICMP request
    icmpbuf = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, packetlen);
    if (icmpbuf == NULL)
    {
        fprintf(stderr, "HeapAlloc failed: %d\n", GetLastError());
        status = -1;
        goto CLEANUP;
    }

    // Initialize the ICMP headers
    if (gAddressFamily == AF_INET)
    {
        if (bRecordRoute)
        {
            // Setup the IP option header to go out on every ICMP packet
            ZeroMemory(&ipopt, sizeof(ipopt));
            ipopt.opt_code = IP_RECORD_ROUTE; // record route option
            ipopt.opt_ptr  = 4;               // point to the first addr offset
            ipopt.opt_len  = 39;              // length of option header

            rc = setsockopt(s, IPPROTO_IP, IP_OPTIONS, (char *)&ipopt, sizeof(ipopt));
            if (rc == SOCKET_ERROR)
            {
                fprintf(stderr, "setsockopt(IP_OPTIONS) failed: %d\n", WSAGetLastError());
                status = -1;
                goto CLEANUP;
            }
        }

        InitIcmpHeader(icmpbuf, gDataSize);
    }
    else if (gAddressFamily == AF_INET6)
    {
        InitIcmp6Header(icmpbuf, gDataSize);
    }

    // Bind the socket -- need to do this since we post a receive first
    rc = bind(s, local->ai_addr, (int)local->ai_addrlen);
    if (rc == SOCKET_ERROR)
    {
        fprintf(stderr, "bind failed: %d\n", WSAGetLastError());
        status = -1;
        goto CLEANUP;
    }

    if (gDataSize > 60000 )
        sockBuff=262144;
    else if(gDataSize > 1000 && gInteval < 10 )
        sockBuff=131072;

    setsockopt(s,SOL_SOCKET,SO_SNDBUF, (char *) &sockBuff,sizeof(sockBuffLen));

    sockBuff=0;
    if (getsockopt(s,SOL_SOCKET,SO_SNDBUF, (char *) &sockBuff,&sockBuffLen) != SOCKET_ERROR)
        printf("\n After setting SO_SNDBUF TX buff size : %d ",sockBuff);

    // Setup the receive operation
    memset(&recvol, 0, sizeof(recvol));
    recvol.hEvent = WSACreateEvent();
    if (recvol.hEvent == WSA_INVALID_EVENT)
    {
        fprintf(stderr, "WSACreateEvent failed: %d\n", WSAGetLastError());
        status = -1;
        goto CLEANUP;
    }

    // Post the first overlapped receive
    fromlen = sizeof(from);
    PostRecvfrom(s, recvbuf, recvbuflen, (SOCKADDR *)&from, &fromlen, &recvol);

    printf("\nPinging ");    

    for(i=0; i < gMaxCount ;i++)
    {
        sendi++;
        // Set the sequence number and compute the checksum
        SetIcmpSequence(icmpbuf);
        ComputeIcmpChecksum(s, icmpbuf, packetlen, dest);

        time = GetTickCount();
        rc = sendto(
            s,
            icmpbuf,
            packetlen,
            0,
            dest->ai_addr,
            (int)dest->ai_addrlen
            );
        if (rc == SOCKET_ERROR)
        {
            if(WSAEWOULDBLOCK == WSAGetLastError())
                printf("Tx buffer full - sendto failed: %d\n", WSAGetLastError());
            else

            {
                fprintf(stderr, "sendto failed: %d\n", WSAGetLastError());
                status = -1;
                goto CLEANUP;
            }
        }

        if(gWaitTime > 10)
            rc = WaitForSingleObject((HANDLE)recvol.hEvent, gWaitTime);
        else
            rc = WaitForSingleObject((HANDLE)recvol.hEvent, 2);        

        if (rc == WAIT_FAILED)
        {
            fprintf(stderr, "WaitForSingleObject failed: %d\n", GetLastError());
            status = -1;
            goto CLEANUP;
        }
        else if (rc == WAIT_TIMEOUT)
        {
            printf("Request timed out.\n");
        }
        else
        {
            rc = WSAGetOverlappedResult(
                s,
                &recvol,
                &bytes,
                FALSE,
                &flags
                );
            if (rc == FALSE)
            {
                fprintf(stderr, "WSAGetOverlappedResult failed: %d\n", WSAGetLastError());
            }
            time = GetTickCount() - time;

            WSAResetEvent(recvol.hEvent);

            printf("Reply from ");
            PrintAddress((SOCKADDR *)&from, fromlen);
            if (time == 0)
            {
                printf(": bytes=%d time<1ms TTL=%d\n", gDataSize, gTtl);
                if(gWaitTime > 10)
                    Sleep(gWaitTime-time); // sleep for the remaining wait time
                else
                    Sleep(1); // sleep for the remaining wait time
            }
            else
            {
                printf(": bytes=%d time=%dms TTL=%d\n", gDataSize, time, gTtl);
                if(gWaitTime > 10)
                    Sleep(gWaitTime-time); // sleep for the remaining wait time
            }
            
            if (0 == CheckIcmpReplyHeader(recvbuf, bytes)) // if reply type is 0 - Echo Reply then count it
            {
                recd++;
            }
            
            if (i < gMaxCount - 1 )				
            {
                fromlen = sizeof(from);
                PostRecvfrom(s, recvbuf, recvbuflen, (SOCKADDR *)&from, &fromlen, &recvol);
            }
        }
        
        printf(" sent %d recieved %d\n", sendi, recd);
        if(resultFile) {
            fprintf(resultFile," sent %d recieved %d\n", sendi, recd);
            fflush(resultFile);
        }
        if(gWaitTime > 10)
            Sleep(gInteval);

    }

    printf(" sent %d recieved %d\n", sendi, recd);
CLEANUP:

    //
    // Cleanup
    //
    if (WSAGetLastError() != WSA_IO_PENDING)
    {
        printf("WSAGetLastError: %d\n", WSAGetLastError());
    }
    else
    {        
        return 0;
    }

    if (dest)
        freeaddrinfo(dest);
    if (local)
        freeaddrinfo(local);
    if (s != INVALID_SOCKET) 
        closesocket(s);
    if (recvol.hEvent != WSA_INVALID_EVENT)
        WSACloseEvent(recvol.hEvent);
    if (icmpbuf)
        HeapFree(GetProcessHeap(), 0, icmpbuf);
    if (resultFile)
        fclose(resultFile);
    WSACleanup();

EXIT:
    return status;
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
    while(n > 0
        && toupper((unsigned char)*s1) == toupper((unsigned char)*s2))
    {
        if(*s1 == '\0')
            return 0;
        s1++;
        s2++;
        n--;
    }
    if(n == 0)
        return 0;
    return toupper((unsigned char)*s1) - toupper((unsigned char)*s2);
}

/* Windows version of strcasecmp */
int strcasecmp(const char *s1, const char *s2)
{
    while(toupper((unsigned char)*s1) == toupper((unsigned char)*s2)) {
        if(*s1 == '\0')
            return 0;
        s1++;
        s2++;
    }
    return toupper((unsigned char)*s1) - toupper((unsigned char)*s2);
}

