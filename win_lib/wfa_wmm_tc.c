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
 * @file wfa_wmm_tc.c
 * @brief File containing the various rountines used for where setting packet's user priority is needed
*/

#include "wfa_dut.h"

#if defined(WFA_WMM_WPA2) || defined(WFA_WMM_PS)
extern struct addrinfo *ResolveAddress(char *addr, char *port, int af, int type, int proto);
extern BOOL DeleteFlow (IN PTC_GEN_FLOW *pFlow);
extern BOOL CreateFlow( IN OUT PTC_GEN_FLOW *_ppTcFlowObj, 
	IN  USHORT   DSCPValue, 
	IN  USHORT   OnePValue,
	IN  ULONG   ThrottleRate);
extern BOOL DeleteFilter(PTC_GEN_FILTER *ppFilter);
extern BOOL CreateFilter( IN OUT PTC_GEN_FILTER  *ppFilter,
	IN  SOCKADDR_STORAGE Address,
	IN  USHORT    Port,
	IN  UCHAR    ProtocolId);
extern void ClNotifyHandler( IN HANDLE ClRegCtx,
	IN HANDLE ClIfcCtx,
	IN ULONG Event,
	IN HANDLE SubCode,
	IN ULONG BufSize,
	IN PVOID Buffer);
extern BOOL ClearIfcList( IN PIFC_LIST pIfcList);
extern BOOL MakeIfcList( IN HANDLE  hClient,
	IN PIFC_LIST pIfcList);
extern BOOL AddTcFlows( IN IFC_LIST  IfcList,
	IN PTC_GEN_FLOW pTcFlow);

extern BOOL AddTcFilters( IN IFC_LIST  IfcList,
	IN PTC_GEN_FILTER pTcFilter);

/*
Declared in wfa_tg.h for following three basic API
*/
int wfaACClassToQos(int tgClass, int *pQos, int *pTosVal, int *pThreadPriority)
{
	int Qos = 0, tosVal=0, threadPriority=0;

	if ( pQos == NULL || pTosVal==NULL || pThreadPriority==NULL)
	{
		DPRINT_ERR(WFA_ERR, "wfaACClassToQos pass in err param or NULL pt tgClass=%d pQos=0x%x pTosVal=0x%x pThreadPriority=0x%x\n",tgClass, pQos,  pTosVal, pThreadPriority);
		return WFA_FAILURE;
	}


	switch(tgClass)
	{
	case TG_WMM_AC_BK:
		/*Change this value to the ported device*/
		Qos = 0x08;
		//Qos = 1;
		DPRINT_INFOL(WFA_OUT, "wfaACClassToQos: Setting QoS BK/UP1 tag: 0x%x\n", TOS_BK);
		tosVal = TOS_BK;
		threadPriority = THREAD_PRIORITY_BELOW_NORMAL;
		break;

	case TG_WMM_AC_BK2:
	case TG_WMM_AC_UP2:
        /*Change this value to the ported device*/
        Qos = 0x10;
		//Qos = 2;
        DPRINT_INFOL(WFA_OUT, "wfaACClassToQos: Setting QoS BK/UP2 tag: 0x%x\n", TOS_LE);
        tosVal = TOS_LE;
        threadPriority = THREAD_PRIORITY_BELOW_NORMAL;
        break;

	case TG_WMM_AC_VI:
	case TG_WMM_AC_UP5:
		Qos = 0x28;
		//Qos = 5;
		DPRINT_INFOL(WFA_OUT, "wfaACClassToQos: Setting QoS VI/UP5 tag: 0x%x\n",TOS_VI);
		tosVal = TOS_VI;
		threadPriority = THREAD_PRIORITY_ABOVE_NORMAL;       
		break;

	/*case TG_WMM_AC_UP5:
        Qos = 0x28;
        DPRINT_INFOL(WFA_OUT, "wfaACClassToQos: Setting QoS UP5 tag: 0x%x\n",TOS_VI);
        tosVal = TOS_VI;
        threadPriority = THREAD_PRIORITY_ABOVE_NORMAL;       
        break;*/

	case TG_WMM_AC_UAPSD:
		tosVal = 0x88;
		threadPriority = THREAD_PRIORITY_BELOW_NORMAL;
		break;

	case TG_WMM_AC_VO:
		/*Change this value to the ported device*/
		Qos = 0x30;
		//Qos = 6;
		DPRINT_INFOL(WFA_OUT, "wfaACClassToQos: Setting QoS VO/UP6 Tag: 0x%x\n",TOS_VO);
		tosVal = TOS_VO;
		threadPriority = THREAD_PRIORITY_HIGHEST;       
		break;

	/*case TG_WMM_AC_UP7:
        Qos = 0x38;
        DPRINT_INFOL(WFA_OUT, "wfaACClassToQos: Setting QoS UP7 Tag: 0x%x\n", TOS_VO7);
        tosVal = TOS_VO7;
        threadPriority = THREAD_PRIORITY_HIGHEST;       
        break;*/

	case TG_WMM_AC_BE:
		Qos = 0x00;
		//Qos = 0;
		DPRINT_INFOL(WFA_OUT, "wfaACClassToQos: Setting QoS BE/UP0 tag: 0x%x\n", TOS_BE);
		tosVal = TOS_BE;
		threadPriority = THREAD_PRIORITY_NORMAL;
		break;

	/*case TG_WMM_AC_UP3:
        Qos = 0x18;
        DPRINT_INFOL(WFA_OUT, "wfaACClassToQos: Setting QoS UP3 tag: 0x%x\n", TOS_BE);
        tosVal = TOS_EE;
        threadPriority = THREAD_PRIORITY_NORMAL;
        break;*/

	/*case TG_WMM_AC_BK2:
		Qos = 0x10;
		//Qos = 2;
		DPRINT_INFOL(WFA_OUT, "wfaACClassToQos: Setting QoS BE2 tag: 0x%x\n", TOS_LE);
        tosVal = TOS_LE;
        threadPriority = THREAD_PRIORITY_BELOW_NORMAL;       
        break;*/

    case TG_WMM_AC_VI2:
        /* Change this value to the ported device */
		Qos = 0x20;
		//Qos = 4;
		DPRINT_INFOL(WFA_OUT, "wfaACClassToQos: Setting QoS VI/UP4 tag: 0x%x\n", TOS_VI4);
        tosVal = TOS_VI4;
        threadPriority = THREAD_PRIORITY_NORMAL;
        break;

    case TG_WMM_AC_VO2:
	case TG_WMM_AC_UP7:
		Qos = 0x38;
		//Qos = 7;
		DPRINT_INFOL(WFA_OUT, "wfaACClassToQos: Setting QoS VO/UP7 tag: 0x%x\n", TOS_VO7);
        /*Change this value to the ported device*/
        tosVal = TOS_VO7;
        threadPriority = THREAD_PRIORITY_ABOVE_NORMAL;       
        break;

    case TG_WMM_AC_BE2:
	case TG_WMM_AC_UP3:
		Qos = 0x18;
		//Qos = 3;
		DPRINT_INFOL(WFA_OUT, "wfaACClassToQos: Setting QoS BE/UP3 tag: 0x%x\n", TOS_EE);
        tosVal = TOS_EE;
        threadPriority = THREAD_PRIORITY_NORMAL;
        break;

	default:
		DPRINT_WARNING(WFA_WNG, "wfaACClassToQos: ERR Unknown AC class value 0x%x\n", tgClass);
		tosVal = TOS_BE;
		Qos = 0x00;
		//Qos = 0;
		threadPriority = threadPriority = THREAD_PRIORITY_BELOW_NORMAL;
	}
	*pQos = Qos;
	*pTosVal = tosVal;
	*pThreadPriority = threadPriority;

	return WFA_SUCCESS;
}

/*  We assume tgClass set in  tgProfile_t 
Before call this routine, must set trafficClass value due to call wfaACClassToQos
declared in wfa_tg.h on following basic API
*/
int wfaOpenTrafficControlFlow(SOCKET sockfd, int tgClass, tgProfile_t *pTGProfile, tgTC_t *pTC)
{
	int ret = WFA_SUCCESS;
	int Qos=0, tosVal=0, threadPriority=0;
	char *stport=NULL;
	ULONG    err;

	SOCKADDR_STORAGE Address = { 0 };
	struct addrinfo  *dest = NULL;

	TCI_CLIENT_FUNC_LIST ClientHandlerList;
    HANDLE    hClient = TC_INVALID_HANDLE;
    IFC_LIST   IfcList = {0};
	PTC_GEN_FLOW  pTcFlow = NULL;
	PTC_GEN_FILTER  pTcFilter = NULL;

	if ( pTC == NULL)
	{
		DPRINT_ERR(WFA_ERR, "wfaOpenTrafficControlFlow, ERR pTC is NULL\n");
		return WFA_FAILURE;
	}
	if (pTGProfile == NULL)
	{
		DPRINT_ERR(WFA_ERR, "wfaOpenTrafficControlFlow, ERR pTGProfile is NULL\n");
		return WFA_FAILURE;
	}

	////ret = wfaACClassToQos(tgClass, &(pTC->Qos), &(pTC->tosVal), &threadPriority);
	if(!CreateFlow(&pTcFlow, pTC->Qos, NOT_SPECIFIED, QOS_NOT_SPECIFIED))
	//if(!CreateFlow(&(pTC->pTcFlow), pTC->Qos, NOT_SPECIFIED, QOS_NOT_SPECIFIED))
	{
		ret = WFA_FAILURE;
		DPRINT_ERR(WFA_ERR, "wfaOpenTrafficControlFlow:TC flow creation - FAILED\n");
		goto CLEANUP;
	}

	stport = (char *) malloc(16); //fix: defined at the beginning of function
	sprintf(stport, "%i", pTGProfile->dport);
	DPRINT_INFOL(WFA_OUT, "wfaOpenTrafficControlFlow:The port string is %s\n", stport);

	// Resolve the destination address
	dest = ResolveAddress( pTGProfile->dipaddr, stport, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(dest == NULL)
	{
		DPRINT_INFOL(WFA_OUT, "ERR wfaOpenTrafficControlFlow:ResolveAddress FAILED");
		ret = WFA_FAILURE;
		goto CLEANUP;
	}

	Address.ss_family = AF_INET;
	memcpy(&Address, dest->ai_addr, (int)dest->ai_addrlen);
	DPRINT_INFOL(WFA_OUT, "wfaOpenTrafficControlFlow creating a filter start for QoS %i\n", (pTC->Qos));

	// Create the TC Filter with the parameters
	if (!CreateFilter(&pTcFilter, Address, pTGProfile->dport, IPPROTO_UDP))
	//if (!CreateFilter(&(pTC->pTcFilter), Address, pTGProfile->dport, IPPROTO_UDP))
	{
		ret = WFA_FAILURE;
		DPRINT_ERR(WFA_ERR, "wfaOpenTrafficControlFlow:CreateFilter - FAILED\n");
		goto CLEANUP;
	}
	// Register TC client
	//memset(&(pTC->ClientHandlerList), 0, sizeof(pTC->ClientHandlerList));
	//pTC->ClientHandlerList.ClNotifyHandler = (TCI_NOTIFY_HANDLER)ClNotifyHandler;
	//ret = TcRegisterClient(CURRENT_TCI_VERSION, 0, &(pTC->ClientHandlerList), &(pTC->hClient));
	memset(&ClientHandlerList, 0, sizeof(ClientHandlerList));
	ClientHandlerList.ClNotifyHandler = (TCI_NOTIFY_HANDLER)ClNotifyHandler;
	err = TcRegisterClient(CURRENT_TCI_VERSION, 0, &ClientHandlerList, &hClient);
	if(err != NO_ERROR)
	{
		DPRINT_ERR(WFA_OUT, "wfaOpenTrafficControlFlow TcRegisterClient Failed %d\n", ret);

		if(err == ERROR_OPEN_FAILED)
		{
			DPRINT_ERR(WFA_OUT, "Please make sure you are running with admin credentials\n");
		}
		ret = WFA_FAILURE;
		goto CLEANUP;
	}
	// Enumerate All TC enabled Interfaces and store the information in IfcList
	//if(!MakeIfcList(pTC->hClient, &(pTC->IfcList)))
	if(!MakeIfcList(hClient, &IfcList))
	{
		DPRINT_ERR(WFA_ERR, "wfaOpenTrafficControlFlow::MakeIfcList falied, make sure QoS Packet Scheduler is active for this interface\n");
		ret = WFA_FAILURE;
		goto CLEANUP;
	}
	// Add pTcFlow on all the Ifcs in the IfcList
	if(!AddTcFlows(IfcList, pTcFlow))
	//if(!AddTcFlows(pTC->IfcList, pTC->pTcFlow))
	{
		DPRINT_ERR(WFA_ERR, "wfaOpenTrafficControlFlow::AddTcFlows FAILED\n");
		ret = WFA_FAILURE;
		goto CLEANUP;
	}
	// Add pTcFilter to all the corresponding TcFlows on all the Ifcs in the IfcList
	if (!AddTcFilters(IfcList, pTcFilter))
	//if (!AddTcFilters(pTC->IfcList, pTC->pTcFilter))
	{
		DPRINT_INFOL(WFA_OUT, "ERR wfaOpenTrafficControlFlow::AddTcFilters\n");
		ret = WFA_FAILURE;
		goto CLEANUP;
	}

//#ifndef QOS_SUPPORT
//	if(setsockopt (sockfd, IPPROTO_IP, IP_TOS, (char *)&(pTC->tosVal), sizeof(pTC->tosVal)) < 0)
//	{
//		DPRINT_INFOL(WFA_OUT,"ERR wfaTGSetPrio, setsockopt failed to set tosVal=0x%x\n", pTC->tosVal);
//		goto CLEANUP;
//	}
//	else
//	{
//		DPRINT_INFOL(WFA_OUT,"wfaTGSetPrio call wfaOpenTrafficControlFlow +++, TOS set OK tosVal=0x%x sockFd=%d\n", pTC->tosVal, sockfd);
//	} 
//#endif

	pTC->trafficClass = tgClass;
CLEANUP:
	//if (ret != WFA_SUCCESS)
	{
		pTC->hClient = hClient;
		//pTC->ClientHandlerList = ClientHandlerList;
		pTC->IfcList = IfcList;
		pTC->pTcFilter = pTcFilter;
		pTC->pTcFlow = pTcFlow;
		//wfaCloseTrafficControlFlow(pTC);
	}
	return ret;
}/* wfaOpenTrafficControlFlow  */


int wfaCloseTrafficControlFlow(tgTC_t *pTC)
{
	if ( pTC == NULL)
	{
		DPRINT_ERR(WFA_ERR, " wfaCloseTrafficControlFlow::ERR,pTC is NULL\n");
		return WFA_FAILURE;
	}
	/* err check inside of each following call  */
	ClearIfcList(&(pTC->IfcList));
	DeleteFilter(&(pTC->pTcFilter));
	DeleteFlow(&(pTC->pTcFlow));

	TcDeregisterClient(pTC->hClient);// must call to unreg
	//memset(&(pTC->ClientHandlerList), 0, sizeof(pTC->ClientHandlerList));

	//ZeroMemory(&(pTC->IfcList), sizeof(IFC_LIST));
	pTC->pTcFilter = NULL;
	pTC->pTcFlow = NULL;
	pTC->Qos     = -1;
	pTC->tosVal  = -1;
	pTC->trafficClass = 0;
	//pTC->hClient = TC_INVALID_HANDLE;
	//pTC->IfcList = {0};

	DPRINT_INFOL(WFA_OUT, " wfaCloseTrafficControlFlow::---\n");
	return WFA_SUCCESS; 
}/* wfaCloseTrafficControlFlow  */

#ifdef WFA_WMM_PS
int wfaCreateUDPSockWmmpsSend(char *ipaddr, int dsc)
{
	struct sockaddr_in servAddr;        /* Local address */
	tgProfile_t *pTGProfile = findTGProfile(wfaTGWMMPSData.wmmps_info.streamid);
	WSADATA wsadata;
	int ret=WSAStartup(MAKEWORD(2,2),&wsadata);
	int errsv = 0, i=0;

	DPRINT_INFOL(WFA_OUT, "wfaCreateUDPSockWmmpsSend, open send socket Begin\n");
	if(ret!=0)
	{
		errsv = WSAGetLastError();
		DPRINT_INFOL(WFA_OUT,  "ERR wfaCreateUDPSockWmmpsSend WSAStartup falled with error %d",errsv);
		return WFA_FAILURE;
	}
	if (pTGProfile == NULL)
	{
		DPRINT_INFOL(WFA_OUT, "ERR wfaCreateUDPSockWmmpsSend pTGProfile NULL streamid=%d", wfaTGWMMPSData.wmmps_info.streamid);
		return WFA_FAILURE;
	}
	servAddr.sin_family      = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	for (i=1; i <2; i++)
	{
		if((wfaTGWMMPSData.wmmps_info.psSendSockFd[i] = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		{
			errsv = WSAGetLastError();
			DPRINT_INFOL(WFA_OUT,  "ERR wfaCreateUDPSockWmmpsSend socket() failed with error %d for port %d",errsv, i + 1 + WFA_WMMPS_UDP_PORT);
			return WFA_FAILURE;
		}
		servAddr.sin_port  = htons((unsigned short)( i + 1 + WFA_WMMPS_UDP_PORT));
		ret =  bind(wfaTGWMMPSData.wmmps_info.psSendSockFd[i], (struct sockaddr *) &servAddr, sizeof(servAddr)); 
		if (ret == SOCKET_ERROR)
		{
			errsv = WSAGetLastError();
			DPRINT_INFOL(WFA_OUT, "ERR wfaCreateUDPSockWmmpsSend bind failed with error %d for port %d",errsv,i + 1 + WFA_WMMPS_UDP_PORT);
			return WFA_FAILURE;
		}
		// here sipaddr is target-PCEnd address
		ret = wfaConnectUDPPeer(wfaTGWMMPSData.wmmps_info.psSendSockFd[i], pTGProfile->dipaddr, pTGProfile->dport);
		if (ret <0)
		{
			DPRINT_INFOL(WFA_OUT, "ERR wfaCreateUDPSockWmmpsSend wfaConnectUDPPeer failed with error %d for port %d",ret,i + 1 + WFA_WMMPS_UDP_PORT);
			return WFA_FAILURE;
		}
	}

	ret = wfaOpenTrafficControlFlow(wfaTGWMMPSData.wmmps_info.psSendSockFd[1], dsc, pTGProfile, &(pTGProfile->tgTC[1]));
	if ( ret == WFA_SUCCESS)
	{
		DPRINT_INFOL(WFA_OUT, "wfaCreateUDPSockWmmpsSend, open  send socket OK\n");
	}
	return ret;
}

int wfaCloseUDPSockWmmpsSend(void)
{    
	int errCd = 0, i=0;
	tgProfile_t *pTGProfile = findTGProfile(wfaTGWMMPSData.wmmps_info.streamid);

	for ( i=1; i<2; i++)
	{
		if (wfaTGWMMPSData.wmmps_info.psSendSockFd[i] >0)
		{
			closesocket(wfaTGWMMPSData.wmmps_info.psSendSockFd[i]);
			wfaTGWMMPSData.wmmps_info.psSendSockFd[i] = -1;
		}
		wfaCloseTrafficControlFlow(&(pTGProfile->tgTC[i]));  // clean up last one used on tc flow
	}
	// following cleanup for all socket opend related to create routine, reduce 
	WSACleanup();
	DPRINT_INFOL(WFA_OUT, "wfaCreateUDPSockWmmpsSend, Close  send socket ok\n");
	return  WFA_SUCCESS;
}
#endif
#endif