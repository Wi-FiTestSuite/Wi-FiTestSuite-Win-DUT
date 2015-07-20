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

#ifndef WFA_WMMPS_H
#define WFA_WMMPS_H
/*
* * APTS messages/tests
* */
#define B_D				1
#define	B_H				2	
#define B_B             3
#define B_M             4
#define M_D             5
#define B_Z             6
#define M_Y             7
#define	L_1				8
#define	A_Y				9		// active mode version of M_Y
#define	B_W				10	
#define	A_J				11		// Active test of sending 4 down
#define M_V             12
#define M_U             13
#define A_U             14
#define M_L             15
#define B_K             16
#define M_B             17
#define M_K             18
#define M_W             19

#ifdef WFA_WMM_AC

#define ATC1            20
#define ATC2            21
#define ATC3            22
#define ATC4            23
#define ATC5            24
#define ATC6            25
#define ATC7            26
#define ATC8            27
#define ATC9            28
#define ATC10           29
#define ATC11           30
#define STC1            31
#define STC2            32
#define STC3            33
#define LAST_TEST       STC3

#else

#define LAST_TEST       M_W

#endif

#define	APTS_DEFAULT    (LAST_TEST + 0x01)     // message codes  20
#define	APTS_HELLO      (APTS_DEFAULT + 0x01)
#define	APTS_BCST       (APTS_HELLO + 0x01)
#define	APTS_CONFIRM    (APTS_BCST + 0x01)
#define	APTS_STOP       (APTS_CONFIRM + 0x01)
#define APTS_CK_BE      (APTS_STOP + 0x01)
#define APTS_CK_BK      (APTS_CK_BE + 0x01)
#define APTS_CK_VI      (APTS_CK_BK + 0x01)
#define APTS_CK_VO      (APTS_CK_VI + 0x01)
#define APTS_RESET      (APTS_CK_VO + 0x01)
#define APTS_RESET_RESP (APTS_RESET + 0x01)
#define APTS_RESET_STOP (APTS_RESET_RESP + 0x01)
#define APTS_LAST       99
#define	MAXRETRY       3
#define	MAXHELLO       20
#define MAX_STOPS      10
#define	NTARG          32                     // number of target names
#define EQ(a,b)        (strcmp(a,b)==0)
#define PORT            12345          // use a common port for sending/receiving
#define   LII  2000000

#ifdef WFA_WMM_AC
/*Listen Inteval for station,to be changed to the actual value*/
#define   lis_int  500000
#define   becon_int  100000
#endif

#define NTARG           32
#define WFA_DEFAULT_CODEC_SEC         0
#define WFA_DEFAULT_CODEC_USEC        10000
#define WFA_WMMPS_UDP_PORT            12345        // it must sync with console
#define WMMPS_MSG_BUF_SIZE            1024
/*
* * power management for WMMPS
* */
#define PS_ON    1
#define PS_OFF   0
#define P_ON     1
#define P_OFF    0
/* legacy power save modes as capi: sta_set_power_save  */
#define LegacyPS_OFF        2
#define LegacyPS_PSPoll     3
#define LegacyPS_FAST       4
#define LegacyPS_PSNonPoll  5
#define LegacyPM_ON         6
#define LegacyPM_OFF        7
/*
* * internal table
* */
struct apts_msg {                   //
	char *name;                     // name of test, packet name/type
	int cmd;                        // msg type index
	int param0;                     // number of packet exchanges
	int param1;                     // number of uplink frames
	int param2;                     // number of downlink frames
	int param3;
};

/*
* * Wait/Timer states
* */
typedef enum {
	WFA_WAIT_NEXT_CODEC,
	WFA_WAIT_FOR_AP_RESPONSE,
	WFA_WAIT_STAUT_00,
	WFA_WAIT_STAUT_01,
	WFA_WAIT_STAUT_02,
	WFA_WAIT_STAUT_03,
	WFA_WAIT_STAUT_04,
	WFA_WAIT_STAUT_0E,
	WFA_WAIT_STAUT_VOLOAD,
	WFA_WAIT_STAUT_SEQ,
} WAIT_MODE;

typedef struct wfa_wmmps
{
	int my_sta_id;
	int my_group_cookie;
	int my_cookie;
	int thr_flag;
	int sta_state;
	int sta_test;
	int wait_state;
	int nextsleep;
	int nsent;
	int msgno;
	int rcv_state;
	int dscp;
	int resetWMMPS;
	int streamid;
	int thr_id;
	tgThrData_t *tdata;
	struct sockaddr_in psToAddr;
	int psSendSockFd[5];
	int psRcvSockFd;
	pthread_t thr;
	pthread_cond_t thr_flag_cond;
	pthread_mutex_t thr_flag_mutex;
}wfaWmmPS_t;

/*  WMMPS used sending dirction routines implemented in wfa_thr.c start with Cap W */
int WfaStaSndHello(char,int,int *state);
int WfaStaSndConfirm(char,int,int *state);
int WfaStaSndVO(char,int,int *state);
int WfaStaSndVOCyclic(char,int,int *state);
int WfaStaSnd2VO(char,int,int *state);
int WfaStaWaitStop(char,int,int *state);
int WfaStaSndStop(char,int,int *state);
int WfaStaSndVI(char,int,int *state);
int WfaStaSndBE(char,int,int *state);
int WfaStaSndBK(char,int,int *state);
int WfaStaSndVIE(char,int,int *state);
int WfaStaSndBEE(char,int,int *state);
int WfaStaSnd2VOE(char,int,int *state);
void create_apts_msg(int msg, unsigned int txbuf[],int id);
/*  receive routine : implemented in wfa_wmmps.c */
int WfaRcvStop(unsigned int *,int ,int *);
int WfaRcvVO(unsigned int *,int ,int *);
int WfaRcvProc(unsigned int *,int ,int *);
int WfaRcvVOCyclic(unsigned int *,int ,int *);
int WfaRcvVI(unsigned int *,int ,int *);
int WfaRcvBE(unsigned int *,int ,int *);
int WfaRcvBK(unsigned int *,int ,int *);

void * wfa_wmmps_thread(void* ssSocket);
DWORD WINAPI wfa_wmmps_send_thread(void* input);
void wfaWmmpsInitFlag(void);
void wfaSetDUTPwrMgmt(int mode);
/* missing declaration added   */

void BUILD_APTS_MSG(int msg, unsigned long *txbuf);
void create_apts_msg(int msg, unsigned int txbuf[],int id);
void mpx(char *m, void *buf_v, int len);

// new API for Win 7 only
int wfaCreateUDPSockWmmpsSend(char *ipaddr, int dsc);
int wfaCloseUDPSockWmmpsSend(void);

#endif