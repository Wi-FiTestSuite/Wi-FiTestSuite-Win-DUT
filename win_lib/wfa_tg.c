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
 * @file wfa_tg.c
 * @brief File containing the traffic related CAPI command processsing rountines
*/

#include "wfa_dut.h"

#define CLOSE closesocket
#define DONE 1

extern int gettimeofday(struct timeval *tv, void *tz);

#ifdef WFA_WMM_WPA2
extern int wfaTrafficSendTo(int, char *, int, struct sockaddr *);
extern int wfaTrafficRecv(int, char *, struct sockaddr *, int);
extern DWORD WINAPI wfa_wpa2_sleep_thread(void *thr_param);
#endif

extern void int2BuffBigEndian(int val, char *buf);
extern int bigEndianBuff2Int(char *buff);

#ifdef WFA_WMM_PS
extern void wfaSetDUTPwrMgmt(int mode);
#endif

/*
* wfaTGSendPing(): Instruct Traffic Generator to send ping packets
*
*/
/** 
 * Initially create a TCP socket
 * @param port TCP socket port to listen.
 * @return socket id.
*/
int wfaTGSendPing(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	int totalpkts;
	char cmdStr[128];
	int streamid = ++wfaTGWMMData.streamId;
	int interval;      /* it in milli second */ 
	tgPingStart_t *staPing = (tgPingStart_t *)caCmdBuf;
	dutCmdResponse_t *spresp = &wfaDutAgentData.gGenericResp;

#ifdef WFA_PING_UDP_ECHO_ONLY
	tgStream_t *myStream = NULL;
#endif

	DPRINT_INFOL(WFA_OUT, "Entering wfaTGSendPing ...\n");
	if(staPing->frameSize == 0)
		staPing->frameSize = 100;

	if(staPing->frameRate == 0)
		staPing->frameRate = 1;

	interval = (int)((1/staPing->frameRate)*1000);

	if(staPing->duration == 0)
		staPing->duration = 10;

	switch(staPing->type)
	{
	case WFA_PING_ICMP_ECHO:
#ifndef WFA_PING_UDP_ECHO_ONLY
		totalpkts = (int) (staPing->duration * staPing->frameRate);
		sprintf(cmdStr, "del /F c:\\WFA\\spout.txt");
		system(cmdStr);
		sprintf(cmdStr, "start \"MPING\" cmd /C \"myping %s  -l %d -i %d -n %d -f  c:\\WFA\\spout.txt\"",
			staPing->dipaddr,  staPing->frameSize,interval,totalpkts);
		DPRINT_INFOL(WFA_OUT, "RUN: %s\n", cmdStr);
		DPRINT_INFOL(WFA_OUT, "Executing %s\n",cmdStr);
		system(cmdStr);
		spresp->status = STATUS_COMPLETE;
		spresp->streamId = streamid;
#else
		DPRINT_INFOL(WFA_OUT, "Only support UDP ECHO\n");
#endif
		break;

	case WFA_PING_UDP_ECHO:
		{
#ifdef WFA_PING_UDP_ECHO_ONLY
			/*
			* Make this like a transaction testing
			* Then make it a profile and run it    
			*/
			myStream = &wfaTGWMMData.gStreams[wfaTGWMMData.slotCnt++];
			memset(myStream, 0, sizeof(tgStream_t));
			memcpy(&myStream->profile, caCmdBuf, len);
			myStream->id = streamid; /* the id start from 1 */ 
			myStream->tblidx = wfaTGWMMData.slotCnt-1; 

            if ((wfaTGWMMData.btSockfd = wfaCreateSock(myStream->profile.transProtoType, "127.0.0.1", WFA_UDP_ECHO_PORT)) == WFA_FAILURE)
            {
                spresp->status = STATUS_ERROR;
                wfaEncodeTLV(WFA_TRAFFIC_SEND_PING_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)spresp, respBuf);
                *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

                return WFA_FAILURE;
            }

            if((wfaTGWMMData.btSockfd = wfaConnectToPeer(myStream->profile.transProtoType, wfaTGWMMData.btSockfd, staPing->dipaddr, WFA_UDP_ECHO_PORT)) == WFA_FAILURE)
            {
                spresp->status = STATUS_ERROR;
                wfaEncodeTLV(WFA_TRAFFIC_SEND_PING_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)spresp, respBuf);
                *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

                return WFA_FAILURE;
            }			
			
			wfaTGWMMData.gtgTransac = streamid;        
			wfaTGWMMData.gtgSend = streamid;            

			/*
			* the framerate here is used to derive the timeout 
			* value for waiting transaction echo responses.
			*/
			wfaTGWMMData.gtimeOut = (int) (MINISECONDS/staPing->frameRate);  /* in msec */

			/* set to longest time */
			if(staPing->duration == 0)
				staPing->duration = 3600;
			
#else
			DPRINT_INFOL(WFA_WNG, "Doesn't support UDP Echo\n");
#endif
			break;
		}
	default:
		{
			spresp->status = STATUS_INVALID;
			spresp->streamId = streamid;
		}
	}

	wfaEncodeTLV(WFA_TRAFFIC_SEND_PING_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)spresp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

	return WFA_SUCCESS;
}

/*
* tgStopPing(): Instruct Traffic Generator to stop ping packets
*
*/
/** 
 * Initially create a TCP socket
 * @param port TCP socket port to listen.
 * @return socket id.
*/
int wfaTGStopPing(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	int streamid = (int )*(caCmdBuf+4); 
	dutCmdResponse_t *stpResp = &wfaDutAgentData.gGenericResp;
	char strout[256];
	char _strout[256];
	FILE *tmpfile = NULL;    
	char *str;  //fix: moved to here

	DPRINT_INFOL(WFA_OUT, "Entering StopPing ...");

	stpResp->status = STATUS_COMPLETE;
	DPRINT_INFOL(WFA_OUT, "It is WINDOWS\n");

	system("taskkill /IM myping.exe /F");
	tmpfile = fopen("c:\\WFA\\spout.txt", "r+");

	if(tmpfile!=NULL)
	{
		while (fgets(_strout, 100, tmpfile) != NULL ) 
		{
			DPRINT_INFOL(WFA_OUT, "string in sprout %s\n", _strout);
			strcpy(strout,_strout);

			while((str = strtok(strout, " ")) == NULL)
				;
			DPRINT_INFOL(WFA_OUT, "got word 1: %s\n", str);

			if(strcmp(str,"sent") == 0)
			{
				str = strtok(NULL, " ");
				DPRINT_INFOL(WFA_OUT, "got word 2: %s\n", str);

				if(str)
					stpResp->cmdru.pingStp.sendCnt = atoi(str);
				else
					stpResp->cmdru.pingStp.sendCnt = 0;
			}

			DPRINT_INFOL(WFA_OUT, "sent %i ", stpResp->cmdru.pingStp.sendCnt);

			str = strtok(NULL, " ");
			DPRINT_INFOL(WFA_OUT, "got word 3: %s\n", str);
			if(strcmp(str,"recieved") == 0)
			{
				str = strtok(NULL, " ");
				if(str)
					stpResp->cmdru.pingStp.repliedCnt = atoi(str);
				else
					stpResp->cmdru.pingStp.repliedCnt = 0;   
			}
			DPRINT_INFOL(WFA_OUT, "received %i\n", stpResp->cmdru.pingStp.repliedCnt);
		}
	}
	else
	{
		DPRINT_INFOL(WFA_OUT, "File is empty\n");
	}

	if(tmpfile == NULL)
	{
		DPRINT_ERR(WFA_ERR, "\ninside tmpfile NULL ...\n");
		stpResp->status = STATUS_COMPLETE;

		wfaEncodeTLV(WFA_TRAFFIC_STOP_PING_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)stpResp, respBuf);
		*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

		return WFA_FAILURE;
	}
	else
	{
		DPRINT_INFOL(WFA_OUT, "Sending result back\n");
		fclose(tmpfile);
		wfaEncodeTLV(WFA_TRAFFIC_STOP_PING_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)stpResp, respBuf);
		*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
	}

	return WFA_SUCCESS;
}


#if defined(WFA_WMM_WPA2) || defined(WFA_WMM_PS) || defined(WFA_WMM_AC)

/** 
 * Initially create a TCP socket
 * @param port TCP socket port to listen.
 * @return socket id.
*/
void rand_gen_sid()
{    
	srand((unsigned)time(0));
	wfaTGWMMData.streamId = rand() % 1000;
}

/*
* findStreamProfile(): search existing stream profile by stream id
* input: id - stream id;
* return: matched stream profile
*/
/** 
 * Initially create a TCP socket
 * @param port TCP socket port to listen.
 * @return socket id.
*/
tgStream_t *findStreamProfile(int id)
{
	volatile int i;
	tgStream_t *myStream = wfaTGWMMData.gStreams;

	for(i = 0; i< WFA_MAX_WMM_STREAMS; i++)
	{
		if(myStream->id == id)
			return myStream;

		myStream++;
	}

	return NULL;
}

/** 
 * Initially create a TCP socket
 * @param port TCP socket port to listen.
 * @return socket id.
*/
tgProfile_t *findTGProfile(int streamId)
{
	volatile int i;
	tgStream_t *myStream = wfaTGWMMData.gStreams;

	for(i = 0; i< WFA_MAX_WMM_STREAMS; i++)
	{
		if(myStream->id == streamId)
			return &(myStream->profile);

		myStream++;
	}

	return NULL;
}

/*
* wfaTGConfig: store the traffic profile setting that will be used to
*           instruct traffic generation.
* input: cmd -- not used
* response: send success back to controller
* return: success or fail
* Note: the profile storage is a global space.
*/
/** 
 * Initially create a TCP socket
 * @param port TCP socket port to listen.
 * @return socket id.
*/
int wfaTGConfig(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
	int ret = WFA_SUCCESS;
	tgStream_t *myStream = NULL;
	dutCmdResponse_t *confResp = &wfaDutAgentData.gGenericResp;

	/* if the stream table over maximum, reset it */ 
	if(wfaTGWMMData.slotCnt == WFA_MAX_TRAFFIC_STREAMS)
		wfaTGWMMData.slotCnt = 0;

	if(wfaTGWMMData.slotCnt == 0)
	{
		DPRINT_INFOL(WFA_OUT, "resetting stream table for %d\n",wfaTGWMMData.gStreams);
		memset(wfaTGWMMData.gStreams, 0, WFA_MAX_TRAFFIC_STREAMS*sizeof(tgStream_t));
	}

	DPRINT_INFOL(WFA_OUT, "Entering wfaTGConfig ...\n");
	myStream = &wfaTGWMMData.gStreams[wfaTGWMMData.slotCnt++];
	memset(myStream, 0, sizeof(tgStream_t));
	memcpy(&myStream->profile, caCmdBuf, len);
	myStream->id = ++wfaTGWMMData.streamId; /* the id start from 1 */ 
	myStream->tblidx = wfaTGWMMData.slotCnt-1;

	DPRINT_INFOL(WFA_OUT, "profile %i direction %i dest ip %s dport %i source %s sport %i rate %i duration %i size %i class %i delay %i\n", myStream->profile.profile, myStream->profile.direction, myStream->profile.dipaddr, myStream->profile.dport, myStream->profile.sipaddr, myStream->profile.sport, myStream->profile.rate, myStream->profile.duration, myStream->profile.pksize, myStream->profile.trafficClass, myStream->profile.startdelay);

	confResp->status = STATUS_COMPLETE; 
	confResp->streamId = myStream->id;
	wfaEncodeTLV(WFA_TRAFFIC_AGENT_CONFIG_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)confResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t); 

	return ret;
}

/*
* wfaTGRecvStart: instruct traffic generator to start receiving 
*                 based on a profile
* input:      cmd -- not used
* response:   inform controller for "running"
* return:     success or failed 
*/
/** 
 * Initially create a TCP socket
 * @param port TCP socket port to listen.
 * @return socket id.
*/
int wfaTGRecvStart(int len, BYTE *parms, int *respLen, BYTE *respBuf)
{
	int status = STATUS_COMPLETE, i;
	int numStreams = len/4;
	int streamid, so;
	tgProfile_t *theProfile;
	tgStream_t *myStream;
	int thisSockfd = -1, ret;

	int iOptVal;
	int iOptLen = sizeof(int);

	DPRINT_INFOL(WFA_OUT, "Entering wfaTGRecvStart ...\n");
	for(i=0; i<numStreams; i++)
	{
		memcpy(&streamid, parms+(4*i), 4); /* changed from 2 to 4, bug reported by n.ojanen */
		myStream = findStreamProfile(streamid); 
		if(myStream == NULL)
		{
			status = STATUS_INVALID;
			DPRINT_INFOL(WFA_OUT, "Failed to find the stream\n");
			return status;    
		}

		theProfile = &myStream->profile;
		if(theProfile == NULL)
		{
			status = STATUS_INVALID;
			DPRINT_INFOL(WFA_OUT, "Failed to find the profile\n");
			return status;
		}

		/* calculate the frame interval which is used to derive its jitter */
		if(theProfile->rate != 0 && theProfile->rate < 5000)
			myStream->fmInterval = 1000000/theProfile->rate; /* in ms */
		else
			myStream->fmInterval = 0;

		if(theProfile->direction != DIRECT_RECV)
		{
			status = STATUS_INVALID;
			DPRINT_INFOL(WFA_OUT, "Failed theProfile->direction=%d\n", theProfile->direction);
			return status;
		}

		memset(&myStream->stats, 0, sizeof(tgStats_t));

		DPRINT_INFOL(WFA_OUT, "*******************  Recv Starts here *********************\n");

		myStream->state = WFA_STREAM_ACTIVE;

		switch(theProfile->profile)
		{
		case PROF_MCAST:
			status = STATUS_COMPLETE;
			thisSockfd = wfaTGWMMData.btSockfd = wfaCreateUDPSock(theProfile->dipaddr, theProfile->dport);
			if(wfaTGWMMData.btSockfd >=0 )
			{
				wfaTGWMMData.gtgRecv = streamid;
				DPRINT_INFOL(WFA_OUT, "wfaTGRecvStart wfaCreateUDPSock for MCAST btSockfd=%d addr=%s:%d\n", wfaTGWMMData.btSockfd,theProfile->dipaddr, theProfile->dport);
			}
			else
			{
				status = STATUS_ERROR;
				wfaEncodeTLV(WFA_TRAFFIC_AGENT_RECV_START_RESP_TLV, 4, (BYTE *)&status, respBuf);
				*respLen = WFA_TLV_HDR_LEN + 4;
				DPRINT_INFOL(WFA_OUT, "wfaTGRecvStart wfaCreateUDPSock falied for MCAST %s:%d \n", theProfile->dipaddr, theProfile->dport);
				return WFA_SUCCESS;
				break;
			}

			/* set multicast socket option for receiver */
			so = wfaSetSockMcastRecvOpt(wfaTGWMMData.btSockfd, theProfile->dipaddr);
			if(so < 0)
			{                    
				DPRINT_INFOL(WFA_OUT, "wfaSetSockMcastRecvOpt,  failed setsockopt at %s\n", theProfile->dipaddr);

				CLOSE(wfaTGWMMData.btSockfd);
				wfaTGWMMData.gtgRecv = 0;
				status = STATUS_ERROR;
				wfaEncodeTLV(WFA_TRAFFIC_AGENT_RECV_START_RESP_TLV, 4, (BYTE *)&status, respBuf);
				*respLen = WFA_TLV_HDR_LEN + 4;
				return WFA_SUCCESS;
			}
			break;

		case PROF_TRANSC:
			wfaTGWMMData.gtgTransac = streamid;

			wfaTGWMMData.wmm_thr[wfaTGWMMData.usedThread].thr_flag = streamid;
			DPRINT_INFOL(WFA_OUT, "PROF_TRANSC wfaTGWMMData.usedThread %i\n", wfaTGWMMData.usedThread);
			DPRINT_INFOL(WFA_OUT, "\r\nFired thread %d\n",wfaTGWMMData.usedThread);
			DPRINT_INFOL(WFA_OUT, "\r\nThread %d flag is %d\n",wfaTGWMMData.usedThread,wfaTGWMMData.wmm_thr[wfaTGWMMData.usedThread].thr_flag);
			ret = ReleaseMutex(wfaTGWMMData.wmm_thr[wfaTGWMMData.usedThread].thr_flag_mutex);
			if ( ret == 0)
			{
				Sleep(100);
				DPRINT_INFOL(WFA_OUT, "PROF_TRANSC wfaTGWMMData.usedThread %i streamId is %d, call release mutex ret = %d flag_mutex=%d errCd=%d \n", 
					wfaTGWMMData.usedThread, wfaTGWMMData.wmm_thr[wfaTGWMMData.usedThread].thr_flag, ret, wfaTGWMMData.wmm_thr[wfaTGWMMData.usedThread].thr_flag_mutex, GetLastError());
				ret = ReleaseMutex(wfaTGWMMData.wmm_thr[wfaTGWMMData.usedThread].thr_flag_mutex);
			}
			DPRINT_INFOL(WFA_OUT, "PROF_TRANSC wfaTGWMMData.usedThread %i  flag-streamId is %d, release mutex ret = %d flag_mutex=%d \n", 
				wfaTGWMMData.usedThread, wfaTGWMMData.wmm_thr[wfaTGWMMData.usedThread].thr_flag, ret, wfaTGWMMData.wmm_thr[wfaTGWMMData.usedThread].thr_flag_mutex);

			wfaTGWMMData.usedThread++;
			break;

		case PROF_FILE_TX:
		case PROF_IPTV:
			/*
			* singal the thread to Sending WMM traffic 
			*/         
			wfaTGWMMData.wmm_thr[wfaTGWMMData.usedThread].thr_flag = streamid;

			DPRINT_INFOL(WFA_OUT, "PROF_FILE_TX or PROF_IPTV, wfaTGWMMData.usedThread %i ; Fired thread %d  flag is %d\n", 
				wfaTGWMMData.usedThread,wfaTGWMMData.usedThread,wfaTGWMMData.wmm_thr[wfaTGWMMData.usedThread].thr_flag);            

			ReleaseMutex(wfaTGWMMData.wmm_thr[wfaTGWMMData.usedThread].thr_flag_mutex);
			Sleep(10);
			ReleaseMutex(wfaTGWMMData.wmm_thr[wfaTGWMMData.usedThread].thr_flag_mutex);

			wfaTGWMMData.usedThread++;
			break;

		case PROF_UAPSD:

#ifdef WFA_WMM_PS
			status = STATUS_COMPLETE;
			wfaWmmpsInitFlag();
			theProfile->trafficClass = 0;  // init to no traffic Class set
			// from STA point view, in the WMMPS, source addr is PCEnd also as dest address to send to
			strcpy(theProfile->dipaddr, theProfile->sipaddr);
			theProfile->tgTC[0].Qos = -1;

			memset(&wfaTGWMMPSData.wmmps_info.psToAddr, 0, sizeof(wfaTGWMMPSData.wmmps_info.psToAddr));
			wfaTGWMMPSData.wmmps_info.psToAddr.sin_family = AF_INET;
			wfaTGWMMPSData.wmmps_info.psToAddr.sin_addr.s_addr = inet_addr(theProfile->sipaddr);
			wfaTGWMMPSData.wmmps_info.psToAddr.sin_port = htons(theProfile->sport);
			wfaTGWMMPSData.wmmps_info.streamid = streamid;

			wfaTGWMMPSData.wmmps_mutex_info.thr_flag = streamid;
			wfaTGWMMPSData.gtgWmmPS = streamid;
			pthread_cond_signal(&wfaTGWMMPSData.wmmps_mutex_info.thr_flag_cond); //Wake up the wfa_wmmps_thread
			DPRINT_INFOL(WFA_OUT, "wfaTGRecvStart PROF_UAPSD srcIPAddr=%s desIPAddr=%s streamId=%d\n",
				theProfile->sipaddr, theProfile->dipaddr, wfaTGWMMPSData.wmmps_info.streamid );                
			wfaTGWMMData.gtimeOut = MINISECONDS/10;  /* in msec */
#endif   /* WFA_WMM_PS */

			break;
		default:
			DPRINT_INFOL(WFA_OUT, "wfaTGRecvStart::Default unknow profile  %d\n", theProfile->profile);
		}
	}

	if(thisSockfd >=0)
	{
		if (getsockopt(thisSockfd, SOL_SOCKET, SO_RCVBUF, (char*)&iOptVal, &iOptLen) != SOCKET_ERROR) 
		{
			DPRINT_INFOL(WFA_OUT, "Default SO_RECVBUF Value: %ld\n", iOptVal);
		}
		else
		{
			DPRINT_INFOL(WFA_OUT, "Default SO_RECVBUF= %ld \n", iOptVal); 
		}

		iOptVal = iOptVal*20;
		if (ret =setsockopt(thisSockfd, SOL_SOCKET, SO_RCVBUF, (char*)&iOptVal, iOptLen) == SOCKET_ERROR)
		{
			DPRINT_INFOL(WFA_OUT, "Default SO_RECVBUF= %ld errCd=%i\n", iOptVal, WSAGetLastError()); 
		}


		if (getsockopt(thisSockfd, SOL_SOCKET, SO_RCVBUF, (char*)&iOptVal, &iOptLen) != SOCKET_ERROR) 
		{
			DPRINT_INFOL(WFA_OUT, "New SO_RECVBUF Value: %ld\n", iOptVal);
		}

		SetThreadPriority((HANDLE)wfaTGWMMData.recvThr, THREAD_PRIORITY_ABOVE_NORMAL);
		ReleaseMutex(wfaTGWMMData.recv_mutex);
	}

	DPRINT_INFOL(WFA_OUT, "End of wfaTGRecvStart\n");

	/* encode a TLV for response for "complete/error ..." */
	wfaEncodeTLV(WFA_TRAFFIC_AGENT_RECV_START_RESP_TLV, sizeof(int), 
		(BYTE *)&status, respBuf);
	*respLen = WFA_TLV_HDR_LEN + sizeof(int); 

	return WFA_SUCCESS;

}/* wfaTGRecvStart */

/*
* tgRecvStop: instruct traffic generator to stop receiving based on a profile
* input:      cmd -- not used
* response:   inform controller for "complete"
* return:     success or failed 
*/
/** 
 * Initially create a TCP socket
 * @param port TCP socket port to listen.
 * @return socket id.
*/
int wfaTGRecvStop(int len, BYTE *parms, int *respLen, BYTE *respBuf)
{
	int status = STATUS_COMPLETE;
	int i;
	int numStreams = len/4;
	int streamid;
	tgProfile_t *theProfile;
	tgStream_t *myStream=NULL;
	dutCmdResponse_t statResp;
	BYTE dutRspBuf[WFA_BUFF_4K];
	int id_cnt = 0;

#ifdef WFA_VOICE_EXT
	FILE *e2eoutp = NULL;
#endif

	DPRINT_INFOL(WFA_OUT, "Entering wfaTGRecvStop ... with length %d\n",len);
	memset(dutRspBuf, 0, WFA_BUFF_4K); //fix: use real 4k buffer

	for(i=0; i<numStreams; i++)
	{
		memcpy(&streamid, parms+(4*i), 4);
		myStream = findStreamProfile(streamid); 
		if(myStream == NULL)
		{
			status = STATUS_INVALID;
			wfaEncodeTLV(WFA_TRAFFIC_AGENT_RECV_STOP_RESP_TLV, 4, (BYTE *)&status, respBuf);
			*respLen = WFA_TLV_HDR_LEN + 4;
			DPRINT_INFOL(WFA_OUT,"wfaTGRecvStop:No stremId found id=%d\n", myStream);
			return WFA_FAILURE;    
		}

		theProfile = &myStream->profile;
		if(theProfile == NULL)
		{
			status = STATUS_INVALID;
			wfaEncodeTLV(WFA_TRAFFIC_AGENT_RECV_STOP_RESP_TLV, 4, (BYTE *)&status, respBuf);
			*respLen = WFA_TLV_HDR_LEN + 4;
			DPRINT_INFOL(WFA_OUT,"wfaTGRecvStop:No stremId profile found id=%d\n", myStream);
			return WFA_FAILURE;
		}

		if(theProfile->direction != DIRECT_RECV)
		{
			status = STATUS_INVALID;
			wfaEncodeTLV(WFA_TRAFFIC_AGENT_RECV_STOP_RESP_TLV, 4, (BYTE *)&status, respBuf);
			*respLen = WFA_TLV_HDR_LEN + 4;
			DPRINT_INFOL(WFA_OUT,"wfaTGRecvStop:stremId id=%d direction error\n", myStream);
			return WFA_FAILURE;
		}

		/* reset its flags, close sockets */
		switch(theProfile->profile)
		{
		case PROF_TRANSC:
			wfaTGWMMData.gtgTransac = 0;

		case PROF_FILE_TX:
		case PROF_MCAST:
		case PROF_IPTV:
			DPRINT_INFOL(WFA_OUT, "rec stop %i\n", streamid);
			WaitForSingleObject(wfaTGWMMData.recv_mutex, 50);

			DPRINT_INFOL(WFA_OUT, "acquired mutex after stop\n");
			Sleep(20);
			wfaTGWMMData.gtgRecv = 0;

			DPRINT_INFOL(WFA_OUT, "rec stop %i\n",  streamid);
			if(wfaTGWMMData.tgSockfds[myStream->tblidx] != -1)
			{
				CLOSE(wfaTGWMMData.tgSockfds[myStream->tblidx]);
				wfaTGWMMData.tgSockfds[myStream->tblidx] = -1;
			}
			break;


		case PROF_UAPSD:
#ifdef WFA_WMM_PS
			DPRINT_INFOL(WFA_OUT, "entering tgRecvStop PROF_UAPSD\n");
			if(wfaTGWMMPSData.psSockfd  > 0)
			{
				CLOSE(wfaTGWMMPSData.psSockfd);
				wfaTGWMMPSData.psSockfd = -1;
			}

			memset(&wfaTGWMMPSData.wmmps_info, 0, sizeof(wfaWmmPS_t));

			wfaSetDUTPwrMgmt(PS_OFF);
			Sleep(3000);  
			wfaTGWMMPSData.gtgWmmPS = 0;                
#endif
			break;

		default:
			DPRINT_INFOL(WFA_OUT, "entering tgRecvStop unknown profile\n");                
		}

		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

		/* encode a TLV for response for "complete/error ..." */
		statResp.status = STATUS_COMPLETE; 
		statResp.streamId = streamid; 

		DPRINT_INFOL(WFA_OUT, "Recv stream Id %i rx %lu total %llu \n", streamid, myStream->stats.rxFrames, myStream->stats.rxPayloadBytes);

		memcpy(&statResp.cmdru.stats, &myStream->stats, sizeof(tgStats_t));
		memcpy((dutRspBuf + i * sizeof(dutCmdResponse_t)), (BYTE *)&statResp, sizeof(dutCmdResponse_t));
		id_cnt++;
		DPRINT_INFOL(WFA_OUT, "Size of a stream is %d\n",sizeof(tgStream_t));        
	}

	myStream->state = WFA_STREAM_INACTIVE;
	wfaEncodeTLV(WFA_TRAFFIC_AGENT_RECV_STOP_RESP_TLV, id_cnt * (int) sizeof(dutCmdResponse_t),    dutRspBuf, respBuf);

	/* done here */
	*respLen = WFA_TLV_HDR_LEN + numStreams * sizeof(dutCmdResponse_t); 

	return WFA_SUCCESS;
}

/*
* wfaTGSendStart: instruct traffic generator to start sending based on a profile
* input:      cmd -- not used
* response:   inform controller for "running"
* return:     success or failed 
*/
/** 
 * Initially create a TCP socket
 * @param port TCP socket port to listen.
 * @return socket id.
*/
int wfaTGSendStart(int len, BYTE *parms, int *respLen, BYTE *respBuf)
{
	int i=0, streamid=0, so;
	int numStreams = len/4;
	int ret = WFA_SUCCESS;
	int timer_dur=0;
#ifdef WFA_WMM_WPA2  
	DWORD thr_id;
#endif
	tgProfile_t *theProfile;
	tgStream_t *myStream = NULL;

	int iOptVal;
	int iOptLen = sizeof(int);

	dutCmdResponse_t staSendResp;

	DPRINT_INFOL(WFA_OUT, "Entering wfaTGSendStart ... for %i streams ...\n", numStreams);
	for(i=0; i<numStreams; i++)
	{
		memcpy(&streamid, parms+(4*i), 4);
		DPRINT_INFOL(WFA_OUT, "Streamid to look for is %d\n",streamid);
		myStream = findStreamProfile(streamid); 
		if(myStream == NULL)
		{
			staSendResp.status = STATUS_INVALID;
			wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, 4, (BYTE *)&staSendResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + 4;
			return WFA_SUCCESS;
		}

		theProfile = &myStream->profile;
		if(theProfile == NULL)
		{
			staSendResp.status = STATUS_INVALID;
			wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, 4, (BYTE *)&staSendResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + 4;
			return WFA_FAILURE;
		}

		if(theProfile->direction != DIRECT_SEND)
		{
			staSendResp.status = STATUS_INVALID;
			wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, 4, (BYTE *)&staSendResp, respBuf);
			*respLen = WFA_TLV_HDR_LEN + 4;
			return WFA_FAILURE;
		}

		/*
		* need to reset the stats
		*/
		memset(&myStream->stats, 0, sizeof(tgStats_t));
		myStream->state = WFA_STREAM_ACTIVE;

		switch(theProfile->profile)
		{
		case PROF_FILE_TX:
			if(theProfile->hti == WFA_ON)
			{

			}
			else
			{				
                if ((wfaTGWMMData.btSockfd = wfaCreateSock(theProfile->transProtoType, theProfile->sipaddr, theProfile->sport)) == WFA_FAILURE)
                {
                    staSendResp.status = STATUS_ERROR;
                    wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, 4, (BYTE *)&staSendResp, respBuf);
                    *respLen = WFA_TLV_HDR_LEN + 4;

                    return WFA_FAILURE;
                }

                if((wfaTGWMMData.btSockfd = wfaConnectToPeer(theProfile->transProtoType, wfaTGWMMData.btSockfd, theProfile->dipaddr, theProfile->dport)) == WFA_FAILURE)
                {
                    staSendResp.status = STATUS_ERROR;
                    wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, 4, (BYTE *)&staSendResp, respBuf);
                    *respLen = WFA_TLV_HDR_LEN + 4;

                    return WFA_FAILURE;
                }
				
				wfaTGWMMData.gtgSend = streamid;				
			}               
			break;

		case PROF_MCAST:			
            if ((wfaTGWMMData.btSockfd = wfaCreateSock(SOCK_TYPE_UDP, theProfile->sipaddr, theProfile->sport)) == WFA_FAILURE)			
			{
                DPRINT_INFOL(WFA_OUT, "socket creation failed\r\n");
                staSendResp.status = STATUS_ERROR;

                wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, 4, (BYTE *)&staSendResp, respBuf);
                *respLen = WFA_TLV_HDR_LEN + 4;

                return WFA_FAILURE;				
			}

			wfaTGWMMData.gtgSend = streamid;

			so = wfaSetSockMcastSendOpt(wfaTGWMMData.btSockfd);
			if (so < 0)
			{
				DPRINT_ERR(WFA_ERR, "setsockopt for mcast: ");
				wfaTGWMMData.gtgSend = 0;
				CLOSE(wfaTGWMMData.btSockfd);
				wfaTGWMMData.btSockfd = -1;
				staSendResp.status = STATUS_ERROR;

				wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, 4, (BYTE *)&staSendResp, respBuf);
				*respLen = WFA_TLV_HDR_LEN + 4;

				return WFA_FAILURE;
			}
			//wfaSetProcPriority(60)
			break;

		case PROF_IPTV:
			/*
			* singal the thread to Sending WMM traffic 
			*/         
			wfaTGWMMData.wmm_thr[wfaTGWMMData.usedThread].thr_flag = streamid;
			wfaTGWMMData.mainSendThread = wfaTGWMMData.usedThread;

			DPRINT_INFOL(WFA_OUT, "wfaTGWMMData.usedThread %i\n", wfaTGWMMData.usedThread);
			DPRINT_INFOL(WFA_OUT, "\r\nFired thread %d\n", wfaTGWMMData.usedThread);
			DPRINT_INFOL(WFA_OUT, "\r\nThread %d flag is %d\n", wfaTGWMMData.usedThread, wfaTGWMMData.wmm_thr[wfaTGWMMData.usedThread].thr_flag);
			ReleaseMutex(wfaTGWMMData.wmm_thr[wfaTGWMMData.usedThread].thr_flag_mutex);
			Sleep(10);
			ReleaseMutex(wfaTGWMMData.wmm_thr[wfaTGWMMData.usedThread].thr_flag_mutex);

			wfaTGWMMData.usedThread++;
			break;

		case PROF_TRANSC:
#ifdef WFA_WMM_WPA2   
            if ((wfaTGWMMData.btSockfd = wfaCreateSock(theProfile->transProtoType, theProfile->sipaddr, theProfile->sport)) == WFA_FAILURE)
            {
                staSendResp.status = STATUS_ERROR;
                wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, 4, (BYTE *)&staSendResp, respBuf);
                *respLen = WFA_TLV_HDR_LEN + 4;

                return WFA_FAILURE;
            }

            if((wfaTGWMMData.btSockfd = wfaConnectToPeer(theProfile->transProtoType, wfaTGWMMData.btSockfd, theProfile->dipaddr, theProfile->dport)) == WFA_FAILURE)
            {
                staSendResp.status = STATUS_ERROR;
                wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, 4, (BYTE *)&staSendResp, respBuf);
                *respLen = WFA_TLV_HDR_LEN + 4;

                return WFA_FAILURE;
            }
			
			wfaTGWMMData.gtgTransac = streamid;        
			wfaTGWMMData.gtgSend = streamid;                   

			/*
			* the framerate here is used to derive the timeout 
			* value for waiting transaction echo responses.
			*/
			wfaTGWMMData.gtimeOut = MINISECONDS/theProfile->rate;  /* in msec */			

			/* set duration for the test */
			DPRINT_INFOL(WFA_OUT, "TRANSFIX -Setting timer for %d ms\n",(theProfile->duration)*1000);
			timer_dur = (theProfile->duration)*1000;
			CreateThread(NULL, 0, wfa_wpa2_sleep_thread, (PVOID)&timer_dur, 0,&thr_id);
#endif          
			break;

		case PROF_START_SYNC:
#ifdef WFA_WMM_WPA2         
			DPRINT_INFOL(WFA_OUT, "profile port %i\n", theProfile->sport);
            if ((wfaTGWMMData.btSockfd = wfaCreateSock(theProfile->transProtoType, theProfile->sipaddr, theProfile->sport)) == WFA_FAILURE)               
            {
                DPRINT_INFOL(WFA_OUT, "socket creation error");
                staSendResp.status = STATUS_ERROR;
                wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, 4, (BYTE *)&staSendResp, respBuf);
                *respLen = WFA_TLV_HDR_LEN + 4;

                return WFA_FAILURE;
            }
            if((wfaTGWMMData.btSockfd = wfaConnectToPeer(theProfile->transProtoType, wfaTGWMMData.btSockfd, theProfile->dipaddr, theProfile->dport)) == WFA_FAILURE)
            {
                DPRINT_INFOL(WFA_OUT, "connection failed\r\n");
                staSendResp.status = STATUS_ERROR;
                wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, 4, (BYTE *)&staSendResp, respBuf);
                *respLen = WFA_TLV_HDR_LEN + 4;

                return WFA_FAILURE;
            }
			
			wfaTGWMMData.gtgTransac = streamid;        
			wfaTGWMMData.gtgSend = streamid;

#ifdef WFA_VOICE_EXT
			if(theProfile->profile == PROF_START_SYNC)
				wfaTGVoiceData.gtgStartSync = streamid;
#endif
			/*
			* the framerate here is used to derive the timeout 
			* value for waiting transaction echo responses.
			*/
			wfaTGWMMData.gtimeOut = MINISECONDS/theProfile->rate;  /* in msec */
			

			/* set duration for the test */            
			DPRINT_INFOL(WFA_OUT, "\r\n SYNCFIX -Setting timer for %d ms\n",(theProfile->duration)*1000);
			timer_dur = (theProfile->duration)*1000;
			CreateThread(NULL, 0, wfa_wpa2_sleep_thread, (PVOID)&timer_dur, 0,&thr_id);          
#endif
			break;
		} // switch

		if(wfaTGWMMData.btSockfd != -1)
		{
			if (getsockopt(wfaTGWMMData.btSockfd, SOL_SOCKET, SO_SNDBUF, (char*)&iOptVal, &iOptLen) != SOCKET_ERROR) 
			{
				DPRINT_INFOL(WFA_OUT, "SO_SNDBUF Value: %ld\n", iOptVal);
			}

			iOptVal = iOptVal*40;            

			ret = setsockopt(wfaTGWMMData.btSockfd, SOL_SOCKET, SO_SNDBUF, (char*)&iOptVal, iOptLen);
			if ( ret == SOCKET_ERROR )
			{
				DPRINT_INFOL(WFA_OUT, "setsockopt failed, SO_SNDBUF=%ld errCd=%i\n",iOptVal*40, WSAGetLastError());
			}

			if (getsockopt(wfaTGWMMData.btSockfd, SOL_SOCKET, SO_SNDBUF, (char*)&iOptVal, &iOptLen) != SOCKET_ERROR) 
			{
				DPRINT_INFOL(WFA_OUT, "SO_SNDBUF Value: %ld\n", iOptVal);
			}
			else
			{
				DPRINT_INFOL(WFA_OUT, "getsockopt failed, SO_SNDBUF=%ld\n", WSAGetLastError());
			}
		}
	} // for

	return WFA_SUCCESS;
}


/** 
 * Initially create a TCP socket
 * @param port TCP socket port to listen.
 * @return socket id.
*/
int wfaTGReset(int len, BYTE *parms, int *respLen, BYTE *respBuf)
{
	dutCmdResponse_t *resetResp = &wfaDutAgentData.gGenericResp;
	int i;
	DPRINT_INFOL(WFA_OUT, "Entering tgreset for btsok %d ...\n",wfaTGWMMData.btSockfd);
	/* need to reset all traffic socket fds */
	//    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
	if(wfaTGWMMData.btSockfd != -1)
	{
		CLOSE(wfaTGWMMData.btSockfd);
		wfaTGWMMData.btSockfd = -1;
	}

	if(wfaTGWMMData.txSockfd != -1)
	{
		CLOSE(wfaTGWMMData.txSockfd);
		wfaTGWMMData.txSockfd = -1;
	}

    for (i = 0; i < WFA_MAX_WMM_STREAMS; i++)
	{
		if (wfaTGWMMData.svrSock[i] != -1)
		{
			CLOSE(wfaTGWMMData.svrSock[i]);
			wfaTGWMMData.svrSock[i] = -1;
		}
	}

	for(i=0;i<WFA_MAX_TRAFFIC_STREAMS;i++)
	{
		if(wfaTGWMMData.tgSockfds[i] > 0)
		{
			DPRINT_INFOL(WFA_WNG, "Closing a socket which should have been closed earlier\n");
			CLOSE(wfaTGWMMData.tgSockfds[i]);
			wfaTGWMMData.tgSockfds[i] = -1;
		}
	}

	/* just reset the flags for the command */
	wfaTGWMMData.gtgRecv = 0;
	wfaTGWMMData.gtgSend = 0;
	wfaTGWMMData.gtgTransac = 0;

	wfaTGWMMData.gtimeOut = 0;

	wfaTGWMMData.runLoop = 0;

	wfaTGWMMData.usedThread = 0;

#ifdef WFA_WMM_PS
	wfaTGWMMPSData.gtgWmmPS = 0;   

	if(wfaTGWMMPSData.psSockfd != -1)
	{
		closesocket(wfaTGWMMPSData.psSockfd);
		wfaTGWMMPSData.psSockfd = -1;
	}

	memset(&wfaTGWMMPSData.wmmps_info, 0, sizeof(wfaWmmPS_t));

	wfaSetDUTPwrMgmt(PS_OFF);
#endif

	DPRINT_INFOL(WFA_OUT, "before memset...\n");

#ifdef WFA_VOICE_EXT
	wfaTGVoiceData.gtgStartSync = 0;
	wfaTGVoiceData.e2eCnt = 0;
	wfaTGVoiceData.e2eResults[0] = '\0';
	memset(wfaTGVoiceData.e2eStats, 0, 6000*sizeof(tgE2EStats_t));
#endif

	/* Also need to clean up WMM streams NOT DONE YET!*/
	wfaTGWMMData.slotCnt = 0;             /* reset stream profile container */
	memset(wfaTGWMMData.gStreams, 0, WFA_MAX_TRAFFIC_STREAMS*sizeof(tgStream_t));
	DPRINT_INFOL(WFA_OUT, "after memset streams...\n");
	/*
	* After be asked to reset, it should lower itself back to 
	* normal level. It is optional implementation if it is not called 
	* while it starts sending/receiving for raising priority level.
	*/
	//wfaSetProcPriority(20);

	/* encode a TLV for response for "complete ..." */
	resetResp->status = STATUS_COMPLETE; 
	wfaEncodeTLV(WFA_TRAFFIC_AGENT_RESET_RESP_TLV, 4, 
		(BYTE *)resetResp, respBuf);
	*respLen = WFA_TLV_HDR_LEN + 4; 
	DPRINT_INFOL(WFA_OUT, "Exiting tgreset for ...\n");
	return WFA_SUCCESS;
}

#ifdef WFA_WMM_WPA2

/* this is to stop sending packets by timer       */ 
/** 
 * Initially create a TCP socket
 * @param port TCP socket port to listen.
 * @return socket id.
*/
void tmout_stop_send(int num)
{
	struct timeval af;
	int i =0;

	gettimeofday(&af,0);
	DPRINT_INFOL(WFA_OUT, "Exiting sendLongFile  at sec %d usec %d\n", af.tv_sec,af.tv_usec);
	DPRINT_INFOL(WFA_OUT, "Timer fired, stop sending traffic\n");

	/*
	*  After runLoop reset, all sendLong will stop
	*/
	wfaTGWMMData.runLoop = 0;

	/*
	* once wfaTGWMMData.usedThread is reset, WMM tests using multithread is ended
	* the threads will be reused for the next test.
	*/
	wfaTGWMMData.usedThread = 0;

	/*
	* once the stream table slot count is reset, it implies that the test
	* is done. When the next set profile command comes in, it will reset/clean
	* the stream table. 
	*/
	wfaTGWMMData.slotCnt = 0;

	/*
	* The test is for DT3 transaction test.
	* Timeout to stop it.
	*/
	if(wfaTGWMMData.gtgTransac != 0)
	{
		wfaTGWMMData.gtgSend = 0;
		wfaTGWMMData.gtgRecv = 0;
		CLOSE(wfaTGWMMData.btSockfd);
		wfaTGWMMData.btSockfd = -1;

#ifdef WFA_VOICE_EXT
		/* Voice End 2 End Sync */      
		if(wfaTGVoiceData.gtgStartSync != 0)
		{
			wfaTGVoiceData.gtgStartSync = 0;
			DPRINT_INFOL(WFA_OUT, "stopping StartSync\n");
		}
#endif /* WFA_VOICE_EXT */
	}

	/*
	* all WMM streams also stop
	*/   
	for(i=0; i<WFA_THREADS_NUM; i++)
	{
		wfaTGWMMData.wmm_thr[i].thr_flag = 0;   
	}
}

/** 
 * Calculate the sleep time for different frame rate
 * @param profile TCP socket port to listen.
 * @param rate The frame rate.
 * @param sleepTime Used for time period to send packets in the throttled Rate
 * @param throttledRate The rate actually for sending data based on framerate.
*/
void wfaTxSleepTime(int profile, int rate, int *sleepTime, int *throttledRate)
{
	*sleepTime=0;     /* in microseconds */

	/*
	* Framerate is still required for Multicast traffic
	* Sleep and hold for a timeout.
	*
	* For WMM traffic, the framerate must also need for VO and VI.
	* the framerate 500, OS may not handle it precisely.
	*/
	switch(profile)
	{
		/*
		* make it a fix rate
		*/
	case PROF_MCAST:
		if(rate < 500 && rate >= 50)
		{
			*sleepTime = 100000;  /* sleep for 100 ms */
			*throttledRate = WFA_MCAST_FRATE;
		}
		else
		{
			*sleepTime = 100000;
			*throttledRate = rate;
		}
		break;

		/*
		* Vendor must find ways to better adjust the speed for their own device
		*/
	case PROF_IPTV:
	case PROF_FILE_TX:

		if (rate == 0)
		{
			*sleepTime = 20000;
			*throttledRate = 10000/50;
		}
		else if(rate > 50)
		{
			/* 
			* this sleepTime indeed is now being used for time period  
			* to send packets in the throttled Rate.  
			* The idea here is that in each fixed 20 minisecond period,
			* The device will send rate/50 (rate = packets / second),
			* then go sleep for rest of time.
			*/
			*sleepTime = 50000; /* fixed 20 miniseconds, changed to 50 */
			*throttledRate = rate/20;

			DPRINT_INFOL(WFA_OUT, "Sleep time %i, throttledRate %i\n", *sleepTime, *throttledRate);
		}
		else if (rate > 0 && rate <= 50) /* typically for voice */
		{
			*throttledRate = 1;
			*sleepTime = 1000*1000/rate;
		}
		break;
	default:
		DPRINT_ERR(WFA_ERR, "Incorrect profile\n");            
	} 
}

#define WFA_TIME_DIFF(before, after, rtime, dtime) \
	dtime = rtime + (after.tv_sec*1000000 + after.tv_usec) - (before.tv_sec*1000000 + before.tv_usec);

/** 
 * Consume the remaining time based on the delay value
 * @param delay The time delay.
 */
void buzz_time(int delay)
{
	struct timeval now, stop;
	int diff;
	int remain_time = 0;

	gettimeofday(&stop, 0);

	stop.tv_usec += delay;
	if(stop.tv_usec > 1000000)
	{
		stop.tv_usec -=1000000;
		stop.tv_sec +=1;
	}

	do
	{
		gettimeofday(&now, 0);
		WFA_TIME_DIFF(now, stop, remain_time, diff);
	} while (diff > 0);
}

/** 
 * This is going to be a blocking SEND till it finishes
 * @param port TCP socket port to listen.
 * @return socket id.
*/
int wfaSendLongFile(int mySockfd, int streamid, BYTE *aRespBuf, int *aRespLen)
{
	static int becnt = 0;

	tgProfile_t           *theProf = NULL;
	tgStream_t            *myStream = NULL;
	struct sockaddr_in    toAddr;
	char                  *packBuf; 
	int  packLen;
	int  bytesSent;
	dutCmdResponse_t sendResp;
	int sleepTime = 0;
	int throttledRate = 0;
	struct timeval before, after,after1,af; 
	int difftime = 0,x;
	int counter = 0;
    struct timeval stime;
	int throttled_est_cost;
	int act_sleep_time;
	gettimeofday(&af,0);

	DPRINT_INFOL(WFA_OUT, "Entering sendLongFile %i at sec %d usec %d\n", streamid,af.tv_sec,af.tv_usec);
	/* find the profile */
	myStream = findStreamProfile(streamid);
	if(myStream == NULL)
	{
		return WFA_FAILURE;
	}

	theProf = &myStream->profile;

	if(theProf == NULL)
	{
		return WFA_FAILURE;
	}

	if(theProf->rate == 0)
	{
		//DPRINT_INFOL(WFA_OUT, "Input parameters control - Rate %i and progset %i and hti %i\n", theProf->duration, wfaDutAgentCAPIData.progSet, theProf->hti);
		if(wfaDutAgentCAPIData.progSet == eDEF_VHT)  // only for VHT or other high through programs
		{
			if(theProf->hti == WFA_ON)
				packLen = MAX_UDP_LEN;
			else if(theProf->hti == WFA_OFF)
				packLen = MAX_ETH_PAYLOAD_LEN;
			else 
				packLen = MAX_LEGACY_PAYLOAD_LEN;
		}
		else
		{
			if (theProf->pksize <= 0)
            {
                packLen = MAX_LEGACY_PAYLOAD_LEN;
            }
            else
            {               
                packLen = theProf->pksize; 
            }
		}
		DPRINT_INFOL(WFA_OUT, "Input parameters control - duration %i and progset %i and PacketLength %i\n", theProf->duration,wfaDutAgentCAPIData.progSet,packLen);
	}
	else
	{
		packLen = theProf->pksize;
	}

	/* allocate a buf */
	packBuf = (char *)malloc(packLen+1);
	memset(packBuf, 0, packLen);

	/* fill in the header */
	strncpy(packBuf, "1345678", sizeof(tgHeader_t));

	/* initialize the destination address */
	memset(&toAddr, 0, sizeof(toAddr));
	toAddr.sin_family = AF_INET;
	toAddr.sin_addr.s_addr = inet_addr(theProf->dipaddr);
	toAddr.sin_port = htons(theProf->dport); 

	/* if a frame rate and duration are defined, then we know
	* interval for each packet and how many packets it needs to
	* send.
	*/
	if(theProf->duration != 0)
	{ 
		DPRINT_INFOL(WFA_OUT, "duration %i\n", theProf->duration);

		if (theProf->hti == WFA_ON && theProf->rate == 0)
		{
			/* experiment for VHT */
			sleepTime = 50;
			//sleepTime = wfaTGWMMData.sleep_time;
			if(becnt != 0)
				throttledRate = 700/20;
			else
				throttledRate = 800/20;
			
			//throttledRate = wfaTGWMMData.throttle_rate;
			throttledRate = 800/20;
			becnt++;

			DPRINT_INFOL(WFA_OUT, "running with HTI and rate 0\n");
		}
		else
		{
			/* 
			*  use this to decide periodical interval sleep time and frames to send
			*  int the each interval.
			*  Each device should adopt a own algorithm for better performance
			*/
			wfaTxSleepTime(theProf->profile, theProf->rate, &sleepTime, &throttledRate);
		}
		// Here assumes it takes 20 usec to send a packet
		throttled_est_cost = throttledRate * 20;  // MUST estimate the cost per ppk
		act_sleep_time = sleepTime - wfaTGWMMData.adj_latency;
		if (act_sleep_time <= 0)
			act_sleep_time = sleepTime;  

		DPRINT_INFOL(WFA_OUT, "sleep time %i act_sleep_time %i\n", sleepTime, act_sleep_time);

		wfaTGWMMData.runLoop=1;

		while(wfaTGWMMData.runLoop)
		{
			/*if(packLen == 1001)
				x=1;*/

			counter++;
			/* fill in the counter */
			int2BuffBigEndian(counter, &((tgHeader_t *)packBuf)->hdr[8]);

			/*if(theProf->transProtoType == SOCK_TYPE_UDP && counter%throttledRate == 0)
			{
				Sleep(1);
			}*/
			/*
			* the following code is only used to slow down
			* over fast traffic flooding the buffer and cause
			* packet drop or the other end not able to receive due to
			* some limitations, purely for experiment purpose.
			* each implementation needs some fine tune to it.
			*/
            //In 60GHz, the throttle control has to be skipped since Intel device won't work properly without non-blocking socket
            if (theProf->transProtoType == SOCK_TYPE_UDP)
            {
			    if(counter ==1)
			    {
				    gettimeofday(&before, NULL);

				    before.tv_usec += sleepTime;
				    if(before.tv_usec > 1000000)
				    {
					    before.tv_usec -= 1000000;
					    before.tv_sec +=1;
				    }
			    }

			    if(throttledRate != 0)
			    {
				    if(counter%throttledRate == 0)
				    {
					    gettimeofday(&after, NULL);
					    difftime = wfa_itime_diff(&after, &before);

					    if(difftime > wfaTGWMMData.adj_latency)
					    {
						    // too much time left, go sleep
						    //DPRINT_INFOL(WFA_OUT, "\r\n Sleeping for %d time", (difftime-wfaTGWMMData.adj_latency)/1000);
						    Sleep((difftime-wfaTGWMMData.adj_latency)/10000);

						    gettimeofday(&after1, NULL);
						    difftime = wfa_itime_diff(&after1, &before);
					    }

					    // burn the rest to absort latency
					    if(difftime >0)
						{
							//DPRINT_INFOL(WFA_OUT, "\r\n diff time %d ", difftime);
						    buzz_time(difftime);
						}

					    before.tv_usec += sleepTime;
					    if(before.tv_usec > 1000000)
					    {
						    before.tv_usec -= 1000000;
						    before.tv_sec +=1;
					    }
				    }
			    } // otherwise, it floods   
            } // end of if (theProf->transProtoType == SOCK_TYPE_UDP)

            /*
             * Fill the timestamp to the header.
             */
            gettimeofday(&stime, NULL);
            int2BuffBigEndian(stime.tv_sec, &((tgHeader_t *)packBuf)->hdr[12]);
            int2BuffBigEndian(stime.tv_usec, &((tgHeader_t *)packBuf)->hdr[16]);

			bytesSent = wfaTrafficSendTo(mySockfd, packBuf, packLen, (struct sockaddr *)&toAddr);
            myStream->stats.txActFrames++;
			if(bytesSent != -1)
			{
				myStream->stats.txPayloadBytes += bytesSent; 
				myStream->stats.txFrames++ ;
			}
			else
			{
				int errsv = WSAGetLastError();
				switch(errsv)
				{
				case WSAENOBUFS:
					DPRINT_ERR(WFA_ERR, "send error\n");
					Sleep(1);             /* hold for 1 ms */
					counter-- ;
					myStream->stats.txFrames--;
					break;

				case WSAECONNRESET:
					DPRINT_ERR(WFA_ERR, "send error RESET\n");
					wfaTGWMMData.runLoop = 0;
					break;

				case WSAENOTCONN:
					DPRINT_ERR(WFA_ERR, "send error not connected\n");
					wfaTGWMMData.runLoop = 0;
					break;

				case WSANOTINITIALISED:
					DPRINT_ERR(WFA_ERR, "WSAStartup not yet called\n");
					break;

                case WSAEWOULDBLOCK:
                    ////DPRINT_ERR(WFA_ERR, "WSAStartup not yet called\n");
                    ////Sleep(1);             /* hold for 1 ms */
                    break;

				default:
					DPRINT_ERR(WFA_ERR, "Packet sent error %d\n",errsv);
					Sleep(20);
				}
			}

		} /* while */
	} /* if ->duration */
	else /* invalid parameters */
	{
		/* encode a TLV for response for "invalid ..." */
		sendResp.status = STATUS_INVALID;
		wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, 4, 
			(BYTE *)&sendResp, (BYTE *)aRespBuf);
		*aRespLen = WFA_TLV_HDR_LEN + 4; 
		free(packBuf);
		return DONE;
	}

	wfaTGWMMData.gtgSend = 0;

	/* free the buffer */
	free(packBuf);

	/* return statistics */
	sendResp.status = STATUS_COMPLETE;
	sendResp.streamId = myStream->id;

    DPRINT_INFOL(WFA_OUT, "Sent actutal frames are %d\r\n",myStream->stats.txActFrames);
	DPRINT_INFOL(WFA_OUT, "\r\n Sent frames are %d\n",myStream->stats.txFrames);

	memcpy(&sendResp.cmdru.stats, &myStream->stats, sizeof(tgStats_t)); 

	wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, sizeof(dutCmdResponse_t), 
		(BYTE *)&sendResp, (BYTE *)aRespBuf);
	*aRespLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
	return DONE;
}

/* this only sends one packet a time */
/** 
 * Send one packet at a time
 * @param port TCP socket port to listen.
 * @return socket id.
*/
int wfaSendShortFile(int mySockfd, int streamid, BYTE *sendBuf, int pksize, BYTE *aRespBuf, int *aRespLen)
{
	BYTE *packBuf = sendBuf; 
	struct sockaddr_in toAddr;
	tgProfile_t *theProf;
	tgStream_t *myStream;
	int packLen, bytesSent=-1;
	dutCmdResponse_t sendResp;

	if(mySockfd == -1)
	{
		/* stop */ 
		wfaTGWMMData.gtgTransac = 0;
		wfaTGWMMData.gtimeOut = 0; 
		wfaTGWMMData.gtgRecv = 0;
		wfaTGWMMData.gtgSend = 0;
		DPRINT_INFOL(WFA_OUT, "stop short traffic\n");

		myStream = findStreamProfile(streamid);
		if(myStream != NULL)
		{
			sendResp.status = STATUS_COMPLETE;
			sendResp.streamId = streamid;
			memcpy(&sendResp.cmdru.stats, &myStream->stats, sizeof(tgStats_t)); 

			wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)&sendResp, aRespBuf);

			*aRespLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
		}

		return WFA_ERROR;
	}

	/* find the profile */
	myStream = findStreamProfile(streamid);

	theProf = &myStream->profile;
	if(theProf == NULL)
	{
		return WFA_ERROR;
	}

	if(pksize == 0)
		packLen = theProf->pksize;
	else
		packLen = pksize;

	memset(&toAddr, 0, sizeof(toAddr));
	toAddr.sin_family = AF_INET;
	toAddr.sin_addr.s_addr = inet_addr(theProf->sipaddr);
	toAddr.sin_port = htons(theProf->sport); 

	if(wfaTGWMMData.gtgRecv && wfaTGWMMData.gtgTransac)
	{
		toAddr.sin_addr.s_addr = inet_addr(theProf->sipaddr);
		toAddr.sin_port = htons(theProf->sport); 
	}
	else if(wfaTGWMMData.gtgSend && wfaTGWMMData.gtgTransac)
	{
		toAddr.sin_addr.s_addr = inet_addr(theProf->dipaddr);
		toAddr.sin_port = htons(theProf->dport); 
	}

    int2BuffBigEndian(myStream->stats.txActFrames, &((tgHeader_t *)packBuf)->hdr[8]);
	
	if(mySockfd != -1)
		bytesSent = wfaTrafficSendTo(mySockfd, (char *)packBuf, packLen, (struct sockaddr *)&toAddr);

    myStream->stats.txActFrames++;

	if(bytesSent != -1)
	{
		myStream->stats.txFrames++;
		myStream->stats.txPayloadBytes += bytesSent;
	}
	else
	{
		int errsv = WSAGetLastError();
		switch(errsv)
		{
		case WSAENOBUFS:
			DPRINT_ERR(WFA_ERR, "send error\n");
			Sleep(1);             /* hold for 1 ms */
			myStream->stats.txFrames--;
			break;

		case WSAECONNRESET:
			wfaTGWMMData.runLoop = 0;
			break;

		case WSAENOTCONN:
			wfaTGWMMData.runLoop = 0;
			break;

		case WSANOTINITIALISED:
			DPRINT_ERR(WFA_ERR, "WSAStartup not yet called\n");
			break;

		default:
			DPRINT_ERR(WFA_ERR, "Packet sent error\n");
		}
	}

	return WFA_SUCCESS;
}

/** 
 * Receive from a specified IP address and Port
 * @param mySockfd The socket for receiving packets.
 * @param streamid The id used to find the stream profile.
 * @param recvBuf The buffer allocated to hold received packets.
 * @return The number of bytes received.
*/
int wfaRecvFile(int mySockfd, int streamid, char *recvBuf, int bufLen)
{
	/* how many packets are received */  
	char *packBuf = recvBuf; 
	struct sockaddr_in fromAddr;
	tgProfile_t *theProf;
	tgStream_t *myStream;
	unsigned int bytesRecvd;
	int lostPkts;
	u_long NonBlock=0; /* make it block */

	/* find the profile */
	myStream = findStreamProfile(streamid);

	theProf = &myStream->profile;
	if(theProf == NULL)
	{
		return WFA_ERROR;
	}

	memset(&fromAddr, 0, sizeof(fromAddr));
	fromAddr.sin_family = AF_INET;
	fromAddr.sin_addr.s_addr = inet_addr(theProf->dipaddr);
	fromAddr.sin_port = htons(theProf->dport); 

	if(wfaTGWMMData.gtgRecv && wfaTGWMMData.gtgTransac)
	{
		fromAddr.sin_addr.s_addr = inet_addr(theProf->sipaddr);
		fromAddr.sin_port = htons(theProf->sport); 
	}
	else if(wfaTGWMMData.gtgSend && wfaTGWMMData.gtgTransac)
	{
		fromAddr.sin_addr.s_addr = inet_addr(theProf->dipaddr);
		fromAddr.sin_port = htons(theProf->dport); 
	}

	if (ioctlsocket(mySockfd, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		return WFA_ERROR;
	}
	/* it is always to receive at least one packet, in case more in the queue, just pick them up */
	bytesRecvd = wfaTrafficRecv(mySockfd, packBuf, (struct sockaddr *)&fromAddr, bufLen);

	if(!wfaTGWMMData.gtgTransac)
	{
		while(bytesRecvd != -1)
		{
#ifdef WFA_VOICE_EXT
			int le2eCnt = myStream->stats.rxFrames;
			tgE2EStats_t *e2esp = myStream->e2ebuf;

			int totalE2Cnt = 6000;
#endif
			myStream->stats.rxFrames++;    
			myStream->stats.rxPayloadBytes +=bytesRecvd;

			/*
			*  Get the lost packet count
			*/
			lostPkts =bigEndianBuff2Int(&((tgHeader_t *)packBuf)->hdr[8]) - 1 - myStream->lastPktSN; 
			myStream->stats.lostPkts += lostPkts; 
			myStream->lastPktSN = bigEndianBuff2Int(&((tgHeader_t *)packBuf)->hdr[8]); 

#ifdef WFA_VOICE_EXT
			if(theProf->profile == PROF_IPTV)
			{
				struct timeval ttval, currTimeVal;

				int sn = bigEndianBuff2Int(&((tgHeader_t *)recvBuf)->hdr[8]);
				ttval.tv_sec = bigEndianBuff2Int(&((tgHeader_t *)recvBuf)->hdr[12]);
				ttval.tv_usec = bigEndianBuff2Int(&((tgHeader_t *)recvBuf)->hdr[16]);
				gettimeofday(&currTimeVal, NULL);

				/*
				* take the end2end stats, limit to the max voice pkt number
				*/
				if(le2eCnt < totalE2Cnt)
				{
					tgE2EStats_t *ep = e2esp + le2eCnt;
					ep->seqnum = sn;
					ep->rsec = ttval.tv_sec;
					ep->rusec = ttval.tv_usec;

					ep->lsec = currTimeVal.tv_sec;
					ep->lusec = currTimeVal.tv_usec;

					if(ep->lusec  < 0)
					{
						ep->lsec -=1;
						ep->lusec += 1000000;
					}
					else if(ep->lusec >= 1000000)
					{
						ep->lsec += 1;
						ep->lusec -= 1000000;
					}
				}
			}
#endif
			bytesRecvd = wfaTrafficRecv(mySockfd, packBuf, (struct sockaddr *)&fromAddr, bufLen);

			if (bytesRecvd == 0)
				SwitchToThread();
		}  
	} 

	if(bytesRecvd != -1)
	{
		myStream->stats.rxFrames++;    
		myStream->stats.rxPayloadBytes +=bytesRecvd;

		/*
		*  Get the lost packet count
		*/
		lostPkts =bigEndianBuff2Int(&((tgHeader_t *)packBuf)->hdr[8]) - 1 - myStream->lastPktSN; 
		myStream->stats.lostPkts += lostPkts; 
		myStream->lastPktSN = bigEndianBuff2Int(&((tgHeader_t *)packBuf)->hdr[8]); 
	}

	return (bytesRecvd);
}

/** 
 * The wrapper of windows sleep
 * @param milSec The number of millisecs to sleep.
*/
void wfaSleepMilsec(int milSec)
{
	if (milSec >0)
		Sleep(milSec);
}

/** 
 * Send fixed bitrate data only 
 * @param mySockfd The local TCP socket to send data.
 * @param streamId The stream id used to find the stream and its config profile.
 * @param pRespBuf TLV response buffer.
 * @param pRespLen TLV response buffer length.
 * @return The status of execution.
*/
int wfaSendBitrateData(int mySockfd, int streamId, BYTE *pRespBuf, int *pRespLen)
{
	tgProfile_t           *theProf = findTGProfile(streamId);
	tgStream_t            *myStream =findStreamProfile(streamId);
	int                   ret = WFA_SUCCESS;
	struct sockaddr_in    toAddr;
	char                  *packBuf=NULL; 
	int                   packLen, bytesSent, rate;
	int                   sleepTimePerSent = 0, nOverTimeCount = 0, nDuration=0, nOverSend=0;
	unsigned long long int sleepTotal=0, extraTimeSpendOnSending=0;   /* sleep mil-sec per sending  */
	int                   counter = 0, i;     /*  frame data sending count */
	unsigned long         difftime;
	dutCmdResponse_t      sendResp;
	struct timeval        before, after, stime; 

	DPRINT_INFOL(WFA_OUT, "wfaSendBitrateData entering\n");

	/* error check section  */
	if ((mySockfd <= 0) || (streamId < 0) || ( pRespBuf == NULL) || (pRespLen == NULL))
	{
		DPRINT_INFOL(WFA_OUT, "wfaSendBitrateData pass-in parameter err mySockfd=%i streamId=%i pRespBuf=0x%x pRespLen=0x%x, *pRespLen=%i\n",
			mySockfd,streamId,pRespBuf,pRespLen,*pRespLen );
		ret= WFA_FAILURE;
		goto errcleanup;
	}

	if (theProf == NULL || myStream == NULL)
	{
		DPRINT_INFOL(WFA_OUT, "wfaSendBitrateData parameter err in NULL pt theProf=%l myStream=%l \n",
			theProf, myStream);
		ret= WFA_FAILURE;
		goto errcleanup;
	}
	if (theProf->rate == 0 || theProf->duration == 0)
	{  /*  rate == 0 means unlimited to push data out which is not this routine to do */
		DPRINT_INFOL(WFA_OUT, "wfaSendBitrateData usage error, bitrate=%i or duration=%i \n" , 
			theProf->rate, theProf->duration);
		ret= WFA_FAILURE;
		goto errcleanup;
	}

	if ((theProf->rate > 3500) || (theProf->pksize > 1000))
	{
		DPRINT_INFOL(WFA_OUT, "wfaSendBitrateData Warning, must set total streams rate<=10000 and payload<=1000; stream bitrate may not meet\n");
	}

	/* calculate bitrate asked */
	if ((rate = theProf->pksize * theProf->rate * 8) > WFA_SEND_FIX_BITRATE_MAX)
	{
		DPRINT_INFOL(WFA_OUT, "wfaSendBitrateData over birate can do in the routine, req bitrate=%l \n",rate);
		ret= WFA_FAILURE;
		goto errcleanup;
	}

	/* alloc sending buff  */
	packLen = theProf->pksize;
	packBuf = (char *)malloc(packLen+4);
	if (packBuf == NULL)
	{
		DPRINT_INFOL(WFA_OUT, "wfaSendBitrateData malloc err \n");
		ret= WFA_FAILURE;
		goto errcleanup;
	}
	memset(packBuf, 0, packLen + 4);
    /* fill in the header */
    strncpy(packBuf, "1345678", sizeof(tgHeader_t));

	/*  initialize the destination address  */
	memset(&toAddr, 0, sizeof(toAddr));
	toAddr.sin_family = AF_INET;
	toAddr.sin_addr.s_addr = inet_addr(theProf->dipaddr);
	toAddr.sin_port = htons(theProf->dport); 

	/* set sleep time per sending */
	if (theProf->rate < 100)
		sleepTimePerSent =  WFA_SEND_FIX_BITRATE_SLEEP_PER_SEND  + 1; 
	else sleepTimePerSent = WFA_SEND_FIX_BITRATE_SLEEP_PER_SEND;

	wfaTGWMMData.runLoop=1; /* global defined share with thread routine, should remove it later  */
	while (wfaTGWMMData.runLoop)
	{
		int iSleep = 0;
		gettimeofday(&before, NULL);
		/* send data per second loop */
		for (i=0; i<= (theProf->rate); i++)
		{
			counter++;
			iSleep++;
			/* fill in the counter */
			int2BuffBigEndian(counter, &((tgHeader_t *)packBuf)->hdr[8]);
            /*           
            * Fill the timestamp to the header.           
            */
            gettimeofday(&stime, NULL);
            int2BuffBigEndian(stime.tv_sec, &((tgHeader_t *)packBuf)->hdr[12]);
            int2BuffBigEndian(stime.tv_usec, &((tgHeader_t *)packBuf)->hdr[16]);

			bytesSent = wfaTrafficSendTo(mySockfd, packBuf, packLen, (struct sockaddr *)&toAddr);
			myStream->stats.txActFrames++;
			if(bytesSent != -1)
			{
				myStream->stats.txPayloadBytes += bytesSent; 
				myStream->stats.txFrames++;
			}
			else
			{
				counter--;
				wfaSleepMilsec(1);
				sleepTotal = sleepTotal + (unsigned long long int) 1;
				DPRINT_INFOL(WFA_OUT, "wfaSendBitrateData wfaTrafficSendTo call ERR counter=%i i=%i; send busy, sleep %i MilSec then send\n", counter, i, sleepTimePerSent);
				i--;
			}
			/*  sleep per batch sending */
			if (i == (theProf->rate/10) * iSleep)
			{
				wfaSleepMilsec(5);
				sleepTotal = sleepTotal + (unsigned long long int) 5;
				iSleep++;
			}
		} // for loop per second sending

		iSleep = 0;
		nDuration++;

		/*calculate second rest part need to sleep  */
		gettimeofday(&after, NULL);
		difftime = wfa_itime_diff(&before, &after);
		if ( difftime < 0 || difftime >= 1000000 )
		{/* over time used, no sleep, go back to send */
			nOverTimeCount++;
			if (difftime > 1000000)
			{
				extraTimeSpendOnSending += (difftime - 1000000);
			}
			wfaSleepMilsec(1);
			sleepTotal++;
			continue;
		}

		/* difftime < 1 sec case, use sleep to catchup time as 1 sec per sending  */
		/*  check with accumulated extra time spend on previous sending, difftime < 1 sec case */
		if (extraTimeSpendOnSending > 0)
		{
			if (extraTimeSpendOnSending > 1000000 - difftime)
			{   
				/* reduce sleep time to catch up over all on time sending   */
				extraTimeSpendOnSending = (extraTimeSpendOnSending - (1000000 - difftime));
				wfaSleepMilsec(5);
				sleepTotal = sleepTotal + (unsigned long long int) 5;
				continue;
			}
			else
			{   /* usec used to */
				difftime =(unsigned long)( difftime - extraTimeSpendOnSending);
				extraTimeSpendOnSending = 0; 
			}
		}

		difftime = difftime/1000; // convert to mil-sec

		if(1000 - difftime > 0)
		{   
			/*  only sleep if there is extrac time with in the second did not spend on sending */
			wfaSleepMilsec(1000 - difftime);
			sleepTotal = sleepTotal + (unsigned long long int)(1000 - difftime);
		}

		if (theProf->duration < nDuration)
		{
			nOverSend++;
		}

	}// while loop

	if (packBuf) 
	{
		free(packBuf);
	}
	/* return statistics */
	sendResp.status = STATUS_COMPLETE;
	sendResp.streamId = myStream->id;

	memcpy(&sendResp.cmdru.stats, &myStream->stats, sizeof(tgStats_t)); 

	wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, sizeof(dutCmdResponse_t), 
		(BYTE *)&sendResp, (BYTE *)pRespBuf);

	*pRespLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

	extraTimeSpendOnSending = extraTimeSpendOnSending/1000;
	DPRINT_INFOL(WFA_OUT, "*** wfg_tg.cpp wfaSendBitrateData Count=%i txActFrames=%i txFrames=%i totalByteSent=%i sleepTotal=%llu milSec extraTimeSpendOnSending=%llu nOverTimeCount=%d nOverSend=%i rate=%d nDuration=%d ***\n", 
		counter,(myStream->stats.txActFrames), (myStream->stats.txFrames),(unsigned int) (myStream->stats.txPayloadBytes), sleepTotal,extraTimeSpendOnSending, nOverTimeCount, nOverSend, theProf->rate , nDuration);
	wfaSleepMilsec(1000);
	return ret;

errcleanup:
	/* encode a TLV for response for "invalid ..." */
	if (packBuf) free(packBuf);

	sendResp.status = STATUS_INVALID;
	wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, 4, (BYTE *)&sendResp, (BYTE *)pRespBuf);
	/* done here */
	*pRespLen = WFA_TLV_HDR_LEN + 4; 

	return ret;
}/*  wfaSendBitrateData  */
#endif
#endif