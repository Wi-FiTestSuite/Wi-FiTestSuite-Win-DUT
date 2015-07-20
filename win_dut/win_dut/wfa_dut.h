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
#ifndef WFA_DUT_H
#define WFA_DUT_H

#include "wfa_main.h"
#include "wfa_wmmps.h"

struct wfa_dutagt_data
{
	/* the agent local Socket, Agent Control socket and baseline test socket*/
	int gagtSockfd;
	int gxcSockfd;    
	int  vend;
	char gCmdStr[WFA_CMD_STR_SZ];
	char intfname[64];
	char WFA_CLI_CMD_DIR[64];
	char WFA_CLI_VERSION[32];
	char gnetIf[WFA_BUFF_32];        /* specify the interface to use */
	dutCmdResponse_t gGenericResp;
};

struct wfa_tg_wmm_data 
{
	/* Global flags for synchronizing the TG functions */
	BOOL gtgSend;         /* flag to sync Send traffic */
	BOOL gtgRecv;         /* flag to sync Recv traffic */
	BOOL gtgTransac;      /* flag to sync Transaction traffic */
	int adj_latency;      /* adjust sleep time due to latency */

	/* stream table */
	tgStream_t gStreams[WFA_MAX_TRAFFIC_STREAMS];         /* streams' buffers */ 
	int txSockfd;
	int btSockfd;
	/* the WMM traffic streams socket fds - Socket Handler table */
	int tgSockfds[WFA_MAX_WMM_STREAMS];

	/* Thread Synchronize flags */
	tgWMM_t wmm_thr[WFA_THREADS_NUM];
	HANDLE recv_mutex;
	DWORD recvThr;
	int streamId;
	int slotCnt;
	int mainSendThread;
	int usedThread;
	int runLoop;
	int gtimeOut;
};

struct wfa_tg_voice_data 
{
	int e2eCnt;
	tgE2EStats_t *e2eStats;
	FILE *e2efp;
	double gtgPktRTDelay;
	BOOL gtgStartSync;
	char e2eResults[124];
};

struct wfa_dutagt_capi_data
{
	int geSupplicant;  /* specifies the supplicant, default is zeroconfig */
	char gStaSSID[WFA_SSID_NAME_LEN];  /* For Marvell supplicant */
	int progSet;
	WORD g11nChnlWidth;
};

struct wfa_tg_wmmps_data 
{
	int psSockfd;
	tgWMMPS_t wmmps_mutex_info;
	wfaWmmPS_t wmmps_info;
	int gtgWmmPS;

	unsigned int psTxMsg[WMMPS_MSG_BUF_SIZE];
	unsigned int psRxMsg[WMMPS_MSG_BUF_SIZE];

	int resetsnd;
	int resetrcv;
	int reset_recd;

	int num_retry;
	int state_num;
	int num_stops;
	int num_hello;

	int msgsize;
};


struct wfa_dutagt_data wfaDutAgentData;
struct wfa_dutagt_capi_data wfaDutAgentCAPIData;

#if defined(WFA_WMM_WPA2) || defined(WFA_WMM_PS) || defined(WFA_WMM_AC)
struct wfa_tg_wmm_data wfaTGWMMData;
#endif

#ifdef WFA_WMM_PS
struct wfa_tg_wmmps_data wfaTGWMMPSData;
#endif

#ifdef WFA_VOICE_EXT
struct wfa_tg_voice_data wfaTGVoiceData;
#endif


#define WFA_STAUT_IF           "{E1F2A722-C270-403B-9D0E-496958EE2C9C}"          /* NOTE: THIS MUST BE CHANGED FOR YOUR DEVICE */

extern int wfaStaAssociate(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaIsConnected(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaGetIpConfig(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetIpConfig(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaVerifyIpConnection(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaGetMacAddress(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetMacAddress(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);

extern int wfaStaGetBSSID(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaGetStats(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaSetEncryption(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetEapTLS(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetPSK(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaGetInfo(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaDeviceGetInfo(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaDeviceListIF(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetEapTTLS(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetEapSim(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetPEAP(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetEapSIM(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetPEAP(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetUAPSD(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetIBSS(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaDebugSet(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetMode(int, BYTE *, int *, BYTE *);
extern int wfaStaUpload(int, BYTE *, int *, BYTE *);
extern int wfaStaSetPwrSave(int, BYTE *, int *, BYTE *);   /* WMMPS   */
extern int wfaStaSetPowerSave(int, BYTE *, int *, BYTE *); /* legacy power save  */
extern int wfaStaSetWMM(int, BYTE *, int *, BYTE *);
extern int wfaStaPresetParameters(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetEapFAST(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetEapAKA(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetSystime(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);

extern int wfaStaSet11n(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetWireless(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSendADDBA(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetRIFS(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSendCoExistMGMT(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaResetDefault(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaDisconnect(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaReassociate(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaCliCommand(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);

#ifdef WFA_P2P
extern int  wfaStaGetP2pDevAddress(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf); 	
extern int wfaStaSetP2p(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);			
extern int wfaStaP2pConnect(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);		
extern int wfaStaP2pJoin(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);		
extern int wfaStaP2pStartGrpFormation(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);		

extern int wfaStaP2pDissolve(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);			
extern int wfaStaSendP2pInvReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);		
extern int wfaStaAcceptP2pReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);		
extern int wfaStaSendP2pProvDisReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);		
extern int wfaStaSetWpsPbc(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);

extern int wfaStaWpsReadPin(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);						
extern int wfaStaWpsEnterPin(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);			 
extern int wfaStaGetPsk(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);			 
#endif

#endif

