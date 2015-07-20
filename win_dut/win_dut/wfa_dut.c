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
 * @file wfa_dut.c
 * @brief File containing the main entry point of Windows DUT
*/

#include "wfa_dut.h"

#ifdef WFA_WMM_WPA2
extern DWORD WINAPI wfa_wmm_thread(void *thr_param);
extern void int2BuffBigEndian(int val, char *buf);
extern void tmout_stop_send(int);
extern tgStream_t *findStreamProfile(int);
extern int wfa_estimate_timer_latency();
#endif

extern xcCommandFuncPtr gWfaCmdFuncTbl[]; /* command process functions */
extern void RefreshTaskbarNotificationArea();
extern void wfa_set_envs();
extern int gettimeofday(struct timeval *tv, void *tz);
extern void rand_gen_sid();
extern void wfa_dut_init(BYTE **tBuf, BYTE **rBuf, BYTE **paBuf, BYTE **cBuf, struct timeval **timerp);

#ifdef WFA_WMM_WPA2
/** 
 * The sleep thread used as a timer for data transfer timeout
 * @param thr_param the sleep duration
 * @return The status of execution
*/
DWORD WINAPI wfa_wpa2_sleep_thread(void *thr_param)
{
	int sleep_prd = *(int *)thr_param;
	Sleep(sleep_prd);
	DPRINT_INFOL(WFA_OUT, "Timer timeout after %i milliseconds\n", sleep_prd);
	tmout_stop_send(0);
	return 0;
}

/** 
 * The thread for receiving data packet in DT3 data transfer
 * @param thr_param the sleep duration
*/
DWORD WINAPI wfa_recv_thread(void *thr_param)
{   
	char recvBuf[32769];
	int newRelease = 0;
	DPRINT_INFOL(WFA_OUT, "Entering wfa_recv_thread...\n");
	while(1)
	{
		if(!wfaTGWMMData.gtgRecv)
		{
			int bw = 0;

			WaitForSingleObject(wfaTGWMMData.recv_mutex, 400);
			if(newRelease == 0)
			{
				ReleaseMutex(wfaTGWMMData.recv_mutex);
				newRelease = 1;
				DPRINT_INFOL(WFA_OUT, "wake up hold object %i...\n", newRelease);
			}
		}
		else
		{
			int ncnt = 0, n=0;
			newRelease = 0;

			if(wfaTGWMMData.btSockfd >= 0)
			{
				while (n!=-1)
				{
					n = wfaRecvFile(wfaTGWMMData.btSockfd, wfaTGWMMData.gtgRecv, (char  *)recvBuf);
					if(wfaTGWMMData.gtgTransac != 0 && n != -1) /* for transaction DT3, need to send a response back */
					{
						char respBuf[WFA_BUFF_4K]; 
						int respLen = 0;

						if(wfaSendShortFile(wfaTGWMMData.btSockfd, wfaTGWMMData.gtgTransac, (BYTE *) recvBuf, n, (BYTE *) respBuf, &respLen) == WFA_SUCCESS)
						{
							if(wfaCtrlSend(wfaDutAgentData.gxcSockfd, (BYTE *)respBuf, respLen)!=respLen)
							{
								DPRINT_WARNING(WFA_WNG, "wfaCtrlSend Error\n");
							}
						}
					}
				}
				continue;  /* this one assumes normal profile will not co-exist with WMM (IPTV) profile */
			}
		}
	} /* while */
}
#endif

/** 
 * Set the time structure used in select()
 * @param secs seconds
 * @param usecs microseconds
 * @return The time strucutre
*/
struct timeval *wfaSetTimer(int secs, int usecs, struct timeval *tv)
{
	struct timeval *mytv;

	if(wfaTGWMMData.gtgTransac != 0)
	{
		tv->tv_sec = secs ;             /* timeout (secs.) */
		tv->tv_usec = usecs;            /* 0 microseconds */
	}
	else
	{
		tv->tv_sec =  0;
		tv->tv_usec = 0;                /* 0 microseconds */
	}

	if(tv->tv_sec == 0 && tv->tv_usec == 0)
	{
		mytv = NULL;
	}
	else
	{
		mytv = tv;
	}
	return mytv; 
}

/** 
 * Initialize variables in the global structs
*/
void init_prog_global()
{
	wfa_defined_debug = WFA_DEBUG_ERR | WFA_DEBUG_WARNING | WFA_DEBUG_INFO;
	wfaDutAgentCAPIData.geSupplicant = 1;    
	wfaDutAgentData.gagtSockfd = -1;
	wfaDutAgentData.gxcSockfd = -1;
	strcpy(wfaDutAgentData.WFA_CLI_CMD_DIR, "C:\\WFA\\CLIs\\Intel");

#ifdef WFA_WMM_WPA2
	wfaTGWMMData.btSockfd = -1;
#endif

#ifdef WFA_VOICE_EXT
	wfaTGVoiceData.gtgPktRTDelay = 0xFFFFFFFF;
	wfaTGVoiceData.gtgStartSync = 0;
#endif

#ifdef WFA_WMM_PS
	wfaTGWMMPSData.msgsize = 256;
#endif
}

/** 
 * Main function of windows DUT agent
*/
int main(int argc, char **argv)
{  
	int       nfds, maxfdn1 = -1, nbytes = 0, cmdLen = 0, isExit = 1;
	int       respLen;
	int       ret = 0;
	WORD      locPortNo = 0;   /* local control port number                  */
	fd_set    sockSet;         /* Set of socket descriptors for select()     */
	BYTE      *xcCmdBuf=NULL, *parmsVal=NULL;
	BYTE      *respBuf=NULL;
	struct timeval *toutvalp=NULL, *tovalp; /* Timeout for select()           */
	WORD      xcCmdTag;
	struct sockfds fds;
	HANDLE hStdOut = NULL;
	SECURITY_ATTRIBUTES sa;
	DWORD dwFlags;
	BYTE *trafficBuf=NULL;

#ifdef WFA_WMM_WPA2    
	struct timeval lstime;
	int asn=1;
	double rttime = 0;
	tgThrData_t tdata[WFA_THREADS_NUM];
	int cntThr = 0;
	int timer_dur=0;
	DWORD thr_id;
#endif /* #ifdef WFA_WMM_WPA2 */

	if (argc < 3)              /* Test for correct number of arguments */
	{
		DPRINT_ERR(WFA_ERR, "Usage:  %s <command interface> <Local Control Port> \n", argv[0]);
		WFA_EXIT(1);
	}

	if(argc > 3)
	{
		sa.nLength              =   sizeof  (SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor =   NULL;
		sa.bInheritHandle       =   TRUE;
		dwFlags =       FILE_ATTRIBUTE_NORMAL;
		hStdOut =   CreateFile  ((LPCWSTR) argv[3], 
			GENERIC_WRITE,
			0,                          
			NULL,                          
			CREATE_ALWAYS,
			dwFlags,
			NULL);

		SetStdHandle(STD_OUTPUT_HANDLE,hStdOut);
		SetStdHandle(STD_ERROR_HANDLE,hStdOut);
		freopen(argv[3],"a",stdout);
		DPRINT_INFOL(WFA_OUT, "Redirected output to %s\n",argv[3]);  
	}

	strncpy(wfaDutAgentData.gnetIf, argv[1], 50);
	DPRINT_INFOL(WFA_OUT, "Interface is %s\n",wfaDutAgentData.gnetIf);
	if(isNumber(argv[2]) == WFA_FAILURE)
	{
		DPRINT_ERR(WFA_ERR, "Incorrect port number\n");
		WFA_EXIT(1);
	}

	locPortNo = atoi(argv[2]);

	init_prog_global();

#if defined(WFA_WMM_WPA2) || defined(WFA_WMM_PS) || defined(WFA_WMM_AC)
	rand_gen_sid();
#endif

#ifdef WFA_WMM_WPA2
	/* raise itself priority class first */
	wfaTGWMMData.adj_latency = wfa_estimate_timer_latency() + 2000; /* four more mini */
	DPRINT_INFOL(WFA_OUT, "\r\nLatency is %d", wfaTGWMMData.adj_latency);
#endif /* #ifdef WFA_WMM_WPA2 */

	/* allocate the traffic stream table */
	wfa_dut_init(&trafficBuf, &respBuf, &parmsVal, &xcCmdBuf, &toutvalp);

	/* 4create listening TCP socket */
	wfaDutAgentData.gagtSockfd = wfaCreateTCPServSock(locPortNo);
	if(wfaDutAgentData.gagtSockfd == -1)
	{
		DPRINT_ERR(WFA_ERR, "Failed to open socket\n");
		WFA_EXIT(1);
	}

	/* initial some external pointers */
	wfa_set_envs();

	if (FALSE == Wow64EnableWow64FsRedirection(FALSE))
	{
		DPRINT_INFOL(WFA_OUT, "Disable WoW64 failed\n");
	}

#ifdef WFA_WMM_WPA2
	/* create a receive thread */
	timer_dur = 1000;

	wfaTGWMMData.recv_mutex = CreateMutex(NULL, FALSE, NULL);
	WaitForSingleObject(wfaTGWMMData.recv_mutex, INFINITE);
	wfaTGWMMData.recvThr = (DWORD) CreateThread(NULL, 0, wfa_recv_thread, (PVOID)&timer_dur, 0,&thr_id);  

	/*
	* Create multiple threads for WMM Stream processing.
	*/
	for(cntThr = 0; cntThr < WFA_THREADS_NUM; cntThr++)
	{ 
		tdata[cntThr].tid = cntThr;
		wfaTGWMMData.wmm_thr[cntThr].thr_flag_mutex = CreateMutex(NULL, FALSE, NULL);
		WaitForSingleObject(wfaTGWMMData.wmm_thr[cntThr].thr_flag_mutex, INFINITE);
		wfaTGWMMData.wmm_thr[cntThr].thr = CreateThread(NULL, 5242880, (LPTHREAD_START_ROUTINE)wfa_wmm_thread, (PVOID)&tdata[cntThr], 0, (LPDWORD)&wfaTGWMMData.wmm_thr[cntThr].thr_id);
	}
#endif /* #ifdef WFA_WMM_WPA2 */

#ifdef WFA_WMM_PS
	/* WMMPS thread  */
	ret = pthread_mutex_init(&wfaTGWMMPSData.wmmps_mutex_info.thr_flag_mutex,NULL);
	if (ret !=0)
	{
		DPRINT_ERR(WFA_ERR, "pthread_mutex_init faile\n");
	}
	ret = pthread_cond_init(&wfaTGWMMPSData.wmmps_mutex_info.thr_flag_cond,NULL);
	if (ret != 0)
	{
		DPRINT_ERR(WFA_ERR, "pthread_cond_init faile\n");
	}
	wfaTGWMMPSData.wmmps_mutex_info.thr_id=pthread_create(&wfaTGWMMPSData.wmmps_mutex_info.thr,NULL /*&ptAttr*/,wfa_wmmps_thread,(void*)&wfaTGWMMPSData.wmmps_mutex_info.thr_id);//Aaron's//calls up the wmmps-thread here
#endif /* #ifdef WFA_WMM_PS */

	maxfdn1 = wfaDutAgentData.gagtSockfd + 1;

	while (isExit) 
	{
		/* set socket file descriptors. For baseline, there are only 
		* three sockets required. They are an agent main socket,
		* Control Agent link socket and Traffic Generator Socket.
		*/
		fds.agtfd = &wfaDutAgentData.gagtSockfd;   
		fds.cafd = &wfaDutAgentData.gxcSockfd;

#ifdef WFA_WMM_PS
		fds.psfd = &wfaTGWMMPSData.psSockfd;
#endif /* #ifdef WFA_WMM_PS */

		wfaSetSockFiDesc(&sockSet, &maxfdn1, &fds);

		/* 
		* The timer will be set for transaction traffic if no echo is back
		* The timeout from the select call force to send a new packet
		*/
		tovalp = NULL;

#ifdef WFA_WMM_WPA2
		if(wfaTGWMMData.gtimeOut != 0)
		{
			/* timeout is set to usec */
			tovalp = wfaSetTimer(0, wfaTGWMMData.gtimeOut*1000, toutvalp);
		}
#endif /* #ifdef WFA_WMM_WPA2 */

		nfds = 0;

		fflush(stdout);

		/* Put a sleep so that the thread gets the mutex then main thread gets it */
		Sleep(1);

		if ((nfds = select(maxfdn1, &sockSet, NULL, NULL, tovalp)) < 0) 
		{
			if (errno == EINTR)
			{
				continue;  /* back to for() */
			}
			else
			{
				int errsv =(int) WSAGetLastError();
				DPRINT_WARNING(WFA_WNG, "Select error: %i", errsv);
			}
		}

		if(nfds == 0)
		{
#ifdef WFA_WMM_WPA2
			/*
			* For transaction test, if it times out without receive a packet,
			* here it should initiate/send the next transac packet and count 
			* a packet loss for the previous one
			*/
			if(wfaTGWMMData.gtgTransac != 0)
			{
				memset(respBuf, 0, WFA_BUFF_4K); 
				respLen = 0;

				/*
				* handle end to end time syc
				*/
				DPRINT_INFOL(WFA_OUT, "Initiate sync message %i\n", asn);
				gettimeofday(&lstime, NULL);
				/*
				* If your device is BIG ENDIAN, you need to 
				* modify the the function calls
				*/
				int2BuffBigEndian(asn++, &((tgHeader_t *)trafficBuf)->hdr[8]);
				int2BuffBigEndian((int)lstime.tv_sec, &((tgHeader_t *)trafficBuf)->hdr[12]);
				int2BuffBigEndian((int)lstime.tv_usec, &((tgHeader_t *)trafficBuf)->hdr[16]);

				if(wfaSendShortFile(wfaTGWMMData.btSockfd, wfaTGWMMData.gtgTransac, trafficBuf, 0, respBuf, &respLen) == WFA_SUCCESS)
				{
					if(wfaCtrlSend(wfaDutAgentData.gxcSockfd, respBuf, respLen) != respLen)
					{
						DPRINT_WARNING(WFA_WNG, "wfaCtrlSend Error\n");
					}
				}
				continue;
			}
#endif /* #ifdef WFA_WMM_WPA2 */
		}

		if (FD_ISSET(wfaDutAgentData.gagtSockfd, &sockSet)) 
		{
			/* Incoming connection request */
			wfaDutAgentData.gxcSockfd = wfaAcceptTCPConn(wfaDutAgentData.gagtSockfd);
			if(wfaDutAgentData.gxcSockfd == -1)
			{
				DPRINT_ERR(WFA_ERR, "Failed to open control link socket\n");
				WFA_EXIT(1);
			}
		}
		/* Control Link port event*/
		if(wfaDutAgentData.gxcSockfd > 0 && FD_ISSET(wfaDutAgentData.gxcSockfd, &sockSet)) 
		{
			memset(xcCmdBuf, '\0', WFA_BUFF_1K);  /* reset the buffer */
			nbytes = wfaCtrlRecv(wfaDutAgentData.gxcSockfd, xcCmdBuf, WFA_BUFF_1K - 4);
			if(nbytes <=0)
			{
				/* errors at the port, close it */
				closesocket(wfaDutAgentData.gxcSockfd);
				wfaDutAgentData.gxcSockfd = -1;
			}
			else
			{
				/*  Clean the tray for any in Active icons */
				RefreshTaskbarNotificationArea();
				DPRINT_INFOL(WFA_OUT, "\nThe command Received after socket: %s\n",xcCmdBuf+4);				
				memset(parmsVal,'\0',MAX_PARMS_BUFF);
				/* command received */
				wfaDecodeTLV(xcCmdBuf, nbytes, &xcCmdTag, &cmdLen, parmsVal);    
				memset(respBuf, 0, WFA_BUFF_4K); 
				respLen = 0;
				DPRINT_INFOL(WFA_OUT, "\nThe command Received after decode: cmdTag %i arguments %s len: %d\n",xcCmdTag, parmsVal,cmdLen);
				
                /* reset two commond storages used by control functions */
				memset(wfaDutAgentData.gCmdStr, '\0', WFA_CMD_STR_SZ);
				memset(&wfaDutAgentData.gGenericResp, '\0', sizeof(dutCmdResponse_t));
				
                /* command process function defined in wfa_ca.c and wfa_tg.c */
				if((xcCmdTag != 0 
					&& xcCmdTag > WFA_STA_NEW_COMMANDS_START 
					&& xcCmdTag < WFA_STA_NEW_COMMANDS_END) 
					&& gWfaCmdFuncTbl[xcCmdTag - WFA_STA_NEW_COMMANDS_START + (WFA_STA_COMMANDS_END - 1)] != NULL)
				{
					DPRINT_INFOL(WFA_OUT, "New command func table used parmsVal=%s\n", parmsVal);
					gWfaCmdFuncTbl[xcCmdTag - WFA_STA_NEW_COMMANDS_START + (WFA_STA_COMMANDS_END - 1)](cmdLen, parmsVal, &respLen, (BYTE *)respBuf);   
				}
				else if((xcCmdTag != 0 && xcCmdTag < WFA_STA_COMMANDS_END) && gWfaCmdFuncTbl[xcCmdTag] != NULL)
				{
					DPRINT_INFOL(WFA_OUT, "Old command func table used parmsVal\n", parmsVal);
					gWfaCmdFuncTbl[xcCmdTag](cmdLen, parmsVal, &respLen, (BYTE *)respBuf);
				}
				else
				{   /* no command defined */
					gWfaCmdFuncTbl[0](cmdLen, parmsVal, &respLen, (BYTE *)respBuf);
					DPRINT_INFOL(WFA_OUT, "Not supported command\n");
				}
				DPRINT_INFOL(WFA_OUT, "Completed control command\n");
#ifdef WFA_WMM_WPA2
				/* for traffic_agent_send command, the wfaSentStatsResp() in wfa_thr.c will send the final status to the ca */
				if(xcCmdTag == WFA_TRAFFIC_AGENT_SEND_RESP_TLV)
				{
					continue;
				}
#endif /* #ifdef WFA_WMM_WPA2 */

				if(wfaCtrlSend(wfaDutAgentData.gxcSockfd, (BYTE *)respBuf, respLen) != respLen)
				{
					DPRINT_WARNING(WFA_WNG, "wfaCtrlSend Error\n");
					/* resend one more time */
					wfaCtrlSend(wfaDutAgentData.gxcSockfd, (BYTE *)respBuf, respLen);
				}      
			}
		}

#ifdef WFA_WMM_PS
		/* Check if there is from Console */
		if(wfaTGWMMPSData.psSockfd > 0 && FD_ISSET(wfaTGWMMPSData.psSockfd, &sockSet))
		{            
			Sleep(100);
			continue;
		}
#endif /* WFA_WMM_PS */

#ifdef WFA_WMM_WPA2
		/*
		* If the profile is set for file transfer, this will run to
		* complete (blocking).
		*/  
		if(wfaTGWMMData.gtgSend != 0 && wfaTGWMMData.gtgTransac == 0)
		{
			tgStream_t *myStream = NULL;
			memset(respBuf, 0, WFA_BUFF_4K); 
			respLen = 0;
			myStream = findStreamProfile(wfaTGWMMData.gtgSend);

			DPRINT_INFOL(WFA_OUT, "\r\n WPA2 -Setting timer for %d ms\n",(myStream->profile.duration)*1000);
			timer_dur = (myStream->profile.duration)*1000;
			CreateThread(NULL, 0, wfa_wpa2_sleep_thread, (PVOID)&timer_dur, 0,&thr_id);    
			wfaSendLongFile(wfaTGWMMData.btSockfd, wfaTGWMMData.gtgSend, respBuf, &respLen );

			if(wfaCtrlSend(wfaDutAgentData.gxcSockfd, (BYTE *)respBuf, respLen) != respLen)
			{
				DPRINT_WARNING(WFA_WNG, "wfaCtrlSend Error\n");
			}
		}
#endif /* WFA_WMM_WPA2 */
	} /* end of while (isExit) */

	free(trafficBuf);
	free(toutvalp);
	free(respBuf);
	free(xcCmdBuf);
	free(parmsVal);

	/* Close sockets */
	closesocket(wfaDutAgentData.gagtSockfd);
	closesocket(wfaDutAgentData.gxcSockfd);

#ifdef WFA_WMM_WPA2
	closesocket(wfaTGWMMData.btSockfd);
#endif /* WFA_WMM_WPA2 */

	DPRINT_INFOL(WFA_OUT, "Exiting MAIN\n");
	return 0;
}
