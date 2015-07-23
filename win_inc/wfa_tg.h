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

/*
* wfa_tg.h:
*   Definitions used in Traffic Generator Modules
*/
#ifndef WFA_TG_H
#define WFA_TG_H

#include <traffic.h> // fix: move the <windows.h> and <traffic.h> after the stdafx.h and mment out <windows.h>
#include <pthread.h>

/* IP Address String length */
#define IPV4_ADDRESS_STRING_LEN    16
#define IPV6_ADDRESS_STRING_LEN    64

#define  MAX_PROFILE_NUM        6
#define  MINISECONDS            1000
#define  MICROSECONDS           1000000
#define  NANOSECONDS            1000000000
#define  LINUX_TIMER_RES        20000000      /* 20 MINSECONDS */
#define  CA_RESPONSE_BUF_SIZE   128

/* Limited bit rate generator related constant or threshold values defined here   */
#define WFA_SEND_FIX_BITRATE_MAX             25*1024*1024  /* 25 Mbits per sec per stream */
#define WFA_SEND_FIX_BITRATE_SLEEP_PER_SEND  1             /* mil-sec per sending sending*/

/* Profile Key words */
#define KW_PROFILE                 1 
#define KW_DIRECTION               2
#define KW_DIPADDR                 3
#define KW_DPORT                   4
#define KW_SIPADDR                 5
#define KW_SPORT                   6
#define KW_FRATE                   7
#define KW_DURATION                8
#define KW_PLOAD                   9
#define KW_TCLASS                  10
#define KW_STREAMID                11
#define KW_COMPLETE                12
#define KW_ERROR                   13
#define KW_STARTDELAY              14
#define KW_NUMFRAME                15
#define KW_USESYNCCLOCK            16
#define KW_USERPRIORITY            17
#define KW_MAXCNT                  18
#define KW_HTI                     19

/* Profile Types */
#define PROF_FILE_TX               1
#define PROF_MCAST                 2
#define PROF_IPTV                  3
#define PROF_TRANSC                4
#define PROF_START_SYNC            5
#define PROF_UAPSD                 6
#define PROF_LAST                  7

/* stream state */
#define WFA_STREAM_INACTIVE        0
#define WFA_STREAM_ACTIVE          1

/* Traffic Directions */
#define DIRECT_SEND                1 
#define DIRECT_RECV                2 

#define TG_PROTO_TCP               0 
#define TG_PROTO_UDP               1

#define TG_WMM_AC_BE          1      /* Best Effort */
#define TG_WMM_AC_BK          2      /* Background  */
#define TG_WMM_AC_VI          3      /* Video       */
#define TG_WMM_AC_VO          4      /* Voice       */
#define TG_WMM_AC_UAPSD       5      /* UAPSD        */

/* For wireless station, 802.11 defines 8 priority queues and 2 for each
*  four catergories
*/
#define TG_WMM_AC_BE2       0x0a
#define TG_WMM_AC_BK2       0x0b
#define TG_WMM_AC_VI2       0x0c
#define TG_WMM_AC_VO2       0x0d

#define TG_WMM_AC_UP0         12     /* User Priority 0 */
#define TG_WMM_AC_UP1         13     /*               1 */
#define TG_WMM_AC_UP2         14     /*               2 */
#define TG_WMM_AC_UP3         15     /*               3 */
#define TG_WMM_AC_UP4         16     /*               4 */
#define TG_WMM_AC_UP5         17     /*               5 */
#define TG_WMM_AC_UP6         18     /*               6 */
#define TG_WMM_AC_UP7         19     /*               7 */

/* wmm defs */
#define TOS_VO7     0xE0         // 111 0  0000 (7)  AC_VO tos/dscp values
#define TOS_VO      0xC0         // 110 0  0000 (6)  AC_VO tos/dscp values
#define TOS_VO6     0xC0         // 110 0  0000 (6)  AC_VO tos/dscp values benz
#define TOS_VO2     0x88         // 110 0  0000 (6)  AC_VO tos/dscp values

#define TOS_VI      0xA0         // 101 0  0000 (5)  AC_VI
#define TOS_VI4     0x80         // 100 0  0000 (4)  AC_VI
#define TOS_VI5     0x88         // 100 0  1000 (4)  AC_VI 

#define TOS_BE      0x00         // 000 0  0000 (0)  AC_BE
#define TOS_EE      0x60         // 011 0  0000 (3)  AC_BE
#define TOS_BK      0x20         // 001 0  0000 (1)  AC_BK
#define TOS_LE      0x40         // 010 0  0000 (2)  AC_BK

#define AC_CLASS_MAX_THREAD        5

#define WFA_UDP_ECHO_STREAM_ID    0xFFFF
#define WFA_UDP_ECHO_PORT         07

#define WFA_PING_ICMP_ECHO         0
#define WFA_PING_UDP_ECHO          1

#define WFA_UPLOAD_VHSO_RPT        1

#define WFA_MCAST_FRATE            50

#define NOT_SPECIFIED   0xFFFF  // used in traffic control API

/* traffic control related struct to keep  */
typedef struct _tg_tc
{
	PTC_GEN_FLOW         pTcFlow;
	PTC_GEN_FILTER       pTcFilter;
	TCI_CLIENT_FUNC_LIST ClientHandlerList;
	HANDLE               hClient;
	IFC_LIST             IfcList;
	int                  Qos;
	int                  tosVal;
	int                  trafficClass;      /* VO, VI, BK, BE */
}tgTC_t;

typedef struct _tg_profile
{
	int  profile;                           /* profile id                    */
	int  direction;
	char dipaddr[IPV4_ADDRESS_STRING_LEN];  /* destination/remote ip address */
	int  dport;
	char sipaddr[IPV4_ADDRESS_STRING_LEN];  /* source/local ip address       */
	int  sport;
	int  rate;
	int  duration;
	int  pksize;
	short trafficClass;      /* VO, VI, BK, BE */
	int  startdelay;
	int maxcnt;
	int hti;                 /* High Throughput Injection */
	tgTC_t      tgTC[5];
} tgProfile_t;

typedef struct _tg_stats
{
	unsigned int txFrames;
	unsigned int rxFrames;
	unsigned long long txPayloadBytes;
	unsigned long long rxPayloadBytes;
	int outOfSequenceFrames; 
	int lostPkts;                 /* voice over wi-fi */
	unsigned long jitter;         /* voice over wi-fi */
} tgStats_t;

typedef struct _e2e_stats
{
	int seqnum;
	int lsec;
	int lusec;
	int rsec;
	int rusec;
} tgE2EStats_t;

typedef struct _tg_stream
{
	int id;
	int sockfd;
	int tblidx;
	int lastPktSN;        /* use for Jitter calculation */
	int fmInterval;       
	int rxTimeLast;       /* use for pkLost             */
	int state;            /* indicate if the stream being active */
	tgProfile_t profile;
	tgStats_t stats;
	tgE2EStats_t *e2ebuf;
} tgStream_t;

typedef struct _traffic_header
{
	char hdr[20];   /* always wfa */
} tgHeader_t;

typedef struct _tg_wmm
{
	int thr_flag;    /* this is used to indicate stream id */
	int stop_flag;    /* this is used to indicate stream id */
	HANDLE thr;
	DWORD thr_id;
	int timerid;
	HANDLE thr_flag_mutex;
	HANDLE thr_stop_mutex;
}tgWMM_t;

typedef struct _tg_wmmps
{
	int thr_flag;    /* this is used to indicate stream id */
	int stop_flag;    /* this is used to indicate stream id */
	int thr_id;
	pthread_t thr;
	pthread_cond_t thr_flag_cond;
	pthread_cond_t thr_stop_cond;
	pthread_mutex_t thr_flag_mutex;
	pthread_mutex_t thr_stop_mutex;
}tgWMMPS_t;

typedef int (*StationStateFunctionPtr)( char, int,int *); //PS,sleep period,state

typedef struct station_state_table
{
	StationStateFunctionPtr statefunc;
	char                    pw_offon;
	int                     sleep_period;
} StationProcStatetbl_t;

typedef int (*stationRecvStateFunctionPtr)(unsigned int *, int,int * ); //Recieved message buffer, length,state

typedef struct console_rcv_state_table
{
	stationRecvStateFunctionPtr statefunc;
} StationRecvProcStatetbl_t;

typedef struct _tg_thr_data
{
	int tid;
	StationProcStatetbl_t  *state;
	int state_num;
} tgThrData_t;

extern int wfaTGConfig(int len, BYTE *buf, int *respLen, BYTE *respBuf);
extern int wfaSendLongFile(int fromSockfd, int streamId, BYTE *respBuf, int *respLen);
extern int wfaRecvFile(int mySockfi, int profId, char *buf);
extern int wfaTGRecvStart(int len, BYTE *parms, int *respLen, BYTE *respBuf);
extern int wfaTGRecvStop(int len, BYTE *parms, int *respLen, BYTE *respBuf);
extern int wfaTGSendStart(int len, BYTE *parms, int *respLen, BYTE *respBuf);
extern int wfaTGReset(int len, BYTE *parms, int *respLen, BYTE *respBuf);
extern int wfaSendShortFile(int fromSockfd, int profId, BYTE *buf, int size, BYTE *respBuf, int *respLen);
extern int wfaFlushSockQueue(int profId);
extern int wfaTGSendPing(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaTGStopPing(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int agtCmdProcGetVersion(int len, BYTE *parms, int *respLen, BYTE *respBuf);
/*  open tc flow with specific AC class */
extern int wfaOpenTrafficControlFlow(SOCKET sockfd, int tgClass, tgProfile_t *pTGProfile,  tgTC_t *pTC);
extern int wfaCloseTrafficControlFlow(tgTC_t *pTC);
extern int wfaSendBitrateData(int mySockfd, int streamId, BYTE *pRespBuf, int *aRespLen);
extern int wfaACClassToQos(int tgClass, int *pQos, int *pTosVal, int *pThreadPriority);
tgStream_t *findStreamProfile(int streamId);
tgProfile_t *findTGProfile(int streamId);

#endif
