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
Licensed under the Microsoft Limited Public License (the $(D@<(Bicense?; you may not
use this file except in compliance with the License.
You may obtain a copy of the License at http://msdn.microsoft.com/en-us/cc300389.

THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS
OF TITLE, FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.

See the Microsoft Limited Public License (Ms-LPL) for specific language governing
permissions and limitations under the License.
*****************************************************************************/

/**
 * @file wfa_thr.c
 * @brief File containing the TC flow handling routines and the ones used of sending file or specific packet
*/

#include "wfa_dut.h"

#if defined(WFA_WMM_WPA2) || defined(WFA_WMM_PS) || defined(WFA_WMM_AC)

#ifdef _IA64_
#pragma warning (disable: 4267)
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <ws2tcpip.h>
#include "resolve.h"

#define NOT_SPECIFIED   0xFFFF

#ifdef WFA_WMM_PS
extern void mpx(char *m, void *buf_v, int len);
#endif /* WFA_WMM_PS */

extern int gettimeofday(struct timeval *tv, void *tz);
extern tgStream_t *findStreamProfile(int id);
extern void tmout_stop_send(int);


//
// Function: ResolveAddress
//
// Description:
//    This routine resolves the specified address and returns a list of addrinfo
//    structure containing SOCKADDR structures representing the resolved addresses.
//    Note that if 'addr' is non-NULL, then getaddrinfo will resolve it whether
//    it is a string listeral address or a hostname.
//
struct addrinfo *ResolveAddress(char *addr, char *port, int af, int type, int proto)
{
    struct addrinfo hints,
    *res = NULL;
    int rc;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags  = ((addr) ? 0 : AI_PASSIVE);
    hints.ai_family = af;
    hints.ai_socktype = type;
    hints.ai_protocol = proto;

    ////DPRINT_INFOL(WFA_OUT, "receiving port %s\n", port);
    //zlog_info(zc, "receiving port %s\r\n", port);
    rc = getaddrinfo(addr, port, &hints, &res);
    if (rc != 0)
    {
        ////DPRINT_ERR(WFA_ERR, "Invalid address %s\n", addr);
        //zlog_error(zc, "Invalid address %s\r\n", addr);
        return NULL;
    }

    return res;
}


//
// Routine: 
// DeleteFlow
// Description:
// Deletes the flow and its member variables
// Arguments:
// [in] pFlow -
//  ptr to the existing Flow struct.
//            
//******************************************************************************
BOOL DeleteFlow (IN PTC_GEN_FLOW *pFlow)
{
    if(pFlow == NULL || *pFlow == NULL)
    {
        return TRUE;
    }

    free(*pFlow);
    *pFlow = NULL;

    return TRUE;
}

//
// Routine: 
// CreateFlow
// Description:
// The function returns a tc flow in ppTcFlowObj on success 
// Arguments:
// [in,out] ppTcFlowObj -
//  double ptr to Flow struct in which the function returns the flow.
// [in] DSCPValue -
//  dscp value for the flow
// [in] OnePValue - 
//  802.1p value for the flow
// [in] ThrottleRate -
//  throttle rate for the flow
// Return:
// TRUE if file creating is successful
// FALSE if creating file failed.
//            
//******************************************************************************
BOOL CreateFlow( IN OUT PTC_GEN_FLOW *_ppTcFlowObj, 
                 IN  USHORT   DSCPValue, 
                 IN  USHORT   OnePValue,
                 IN  ULONG   ThrottleRate)
{
    BOOL status = FALSE;
 //
 // Flow Parameters
 //
    ULONG   TokenRate = QOS_NOT_SPECIFIED;
    ULONG   TokenBucketSize = QOS_NOT_SPECIFIED;
    ULONG   PeakBandwidth = QOS_NOT_SPECIFIED;
    ULONG   Latency = QOS_NOT_SPECIFIED;
    ULONG   DelayVariation = QOS_NOT_SPECIFIED;
    SERVICETYPE  ServiceType = SERVICETYPE_BESTEFFORT;
    ULONG   MaxSduSize = QOS_NOT_SPECIFIED;
    ULONG   MinimumPolicedSize = QOS_NOT_SPECIFIED;

    PVOID   pCurrentObject;
    PTC_GEN_FLOW _pTcFlowObj = NULL;

    int    Length = 0;
  
   //
   // Calculate the memory size required for the optional TC objects
   //
    Length += (OnePValue == NOT_SPECIFIED ? 0 : sizeof(QOS_TRAFFIC_CLASS)) + 
          (DSCPValue == NOT_SPECIFIED ? 0 : sizeof(QOS_DS_CLASS));

   //
   // Print the Flow parameters
   //
    if(ThrottleRate == QOS_NOT_SPECIFIED) 
    {
        // DPRINT_INFO(WFA_OUT, "\tThrottleRate: *\n");
        // DPRINT_INFO(WFA_OUT, "\tServiceType: Best effort\n");
    }
    else 
    {
        ServiceType = SERVICETYPE_GUARANTEED;
    }

    TokenRate = TokenBucketSize = ThrottleRate;

    //
    // Allocate the flow descriptor
    //
    _pTcFlowObj = (PTC_GEN_FLOW)malloc(FIELD_OFFSET(TC_GEN_FLOW, TcObjects) + Length);
    if (!_pTcFlowObj) 
    {
        ////DPRINT_ERR(WFA_ERR, "Flow Allocation Failed\n");
        //zlog_error(zc, "Flow Allocation Failed\r\n");
        goto Exit;
    }

    _pTcFlowObj->SendingFlowspec.TokenRate = TokenRate;
    _pTcFlowObj->SendingFlowspec.TokenBucketSize = TokenBucketSize;
    _pTcFlowObj->SendingFlowspec.PeakBandwidth = PeakBandwidth;
    _pTcFlowObj->SendingFlowspec.Latency = Latency;
    _pTcFlowObj->SendingFlowspec.DelayVariation = DelayVariation;
    _pTcFlowObj->SendingFlowspec.ServiceType = ServiceType;
    _pTcFlowObj->SendingFlowspec.MaxSduSize = MaxSduSize;
    _pTcFlowObj->SendingFlowspec.MinimumPolicedSize = MinimumPolicedSize;

    memcpy(&(_pTcFlowObj->ReceivingFlowspec), &(_pTcFlowObj->SendingFlowspec), sizeof(_pTcFlowObj->ReceivingFlowspec));

    _pTcFlowObj->TcObjectsLength = Length;

   //
   // Add any requested objects
   //
    pCurrentObject = (PVOID)_pTcFlowObj->TcObjects;

    if(OnePValue != NOT_SPECIFIED)
    {
        QOS_TRAFFIC_CLASS *pTClassObject = (QOS_TRAFFIC_CLASS*)pCurrentObject;
        pTClassObject->ObjectHdr.ObjectType = QOS_OBJECT_TRAFFIC_CLASS;
        pTClassObject->ObjectHdr.ObjectLength = sizeof(QOS_TRAFFIC_CLASS);
        pTClassObject->TrafficClass = OnePValue; //802.1p tag to be used

        pCurrentObject = (PVOID)(pTClassObject + 1);
    }

    if(DSCPValue != NOT_SPECIFIED)
    {
        QOS_DS_CLASS *pDSClassObject = (QOS_DS_CLASS*)pCurrentObject;
        pDSClassObject->ObjectHdr.ObjectType = QOS_OBJECT_DS_CLASS;
        pDSClassObject->ObjectHdr.ObjectLength = sizeof(QOS_DS_CLASS);
        ////pDSClassObject->DSField = (DSCPValue << 3); //Services Type
		pDSClassObject->DSField = DSCPValue;
    }

    DeleteFlow(_ppTcFlowObj);
    *_ppTcFlowObj = _pTcFlowObj;

    status = TRUE;

Exit:
    if(!status)
    {
        ////DPRINT_INFO(WFA_OUT, "Flow Creation Failed\n");
        //zlog_info(zc, "Flow Creation Failed\n");
        DeleteFlow(&_pTcFlowObj);
    }
    else
    {
        ////DPRINT_INFO(WFA_OUT, "Flow Creation Succeeded\n");
        //zlog_info(zc, "Flow Creation Succeeded\r\n");
    }

    return status;
}

//******************************************************************************
// Routine: 
//      DeleteFilter
//
// Description:
//      Deletes the filter and its member variables
//            
//******************************************************************************
BOOL DeleteFilter(PTC_GEN_FILTER *ppFilter)
{
    PTC_GEN_FILTER pFilter;

    if (ppFilter == NULL || *ppFilter == NULL)
    {
        return TRUE;
    }

    pFilter = (*ppFilter);

    if (pFilter->Pattern)
    {
        free(pFilter->Pattern);
    }

    if (pFilter->Mask)
    {
        free(pFilter->Mask);
    }

    *ppFilter = NULL;

    return TRUE;
}

//
// Routine: 
// CreateFilter
// Description:
// The function returns a tc filter in ppFilter on success 
// Arguments:
// [in, out] ppFilter - 
//  double ptr to Filter struct in which the function returns the filter
// [in] Address -
//  destination address of the outgoing packets of interest.
// [in] Port -
//  destination port of the outgoing packets of interest.
// [in] ProtocolId -
//  protocol of the outgoing packets of interest.
// Return:
// TRUE if filter creating successes.
// FALSE if failed.
//
BOOL CreateFilter( IN OUT PTC_GEN_FILTER  *ppFilter,
                   IN  SOCKADDR_STORAGE Address,
                   IN  USHORT    Port,
                   IN  UCHAR    ProtocolId)
{  
    BOOL status = FALSE;
    USHORT AddressFamily = Address.ss_family;
    PTC_GEN_FILTER pFilter = NULL;
    PIP_PATTERN pPattern = NULL;
    PIP_PATTERN pMask = NULL;

    if(AddressFamily != AF_INET)
    {
        ////DPRINT_INFO(WFA_OUT, "Address family is not AF_INET");
        //zlog_info(zc, "Address family is not AF_INET\r\n");
        goto Exit;
    }
  
    //
    // Allocate memory for the filter
    //
    pFilter = (PTC_GEN_FILTER)malloc(sizeof(TC_GEN_FILTER));
    if(!pFilter)
    {
        ////DPRINT_INFO(WFA_OUT, "Error, No memory for filter\n");
        //zlog_info(zc, "Error, No memory for filter\r\n");
        goto Exit;
    }
    ZeroMemory(pFilter, sizeof(TC_GEN_FILTER));
      
    //
    // Allocate memory for the pattern and mask
    //
    pPattern = (PIP_PATTERN)malloc(sizeof(IP_PATTERN));
    pMask    = (PIP_PATTERN)malloc(sizeof(IP_PATTERN));

    if(!pPattern || !pMask)
    {
        ////DPRINT_INFO(WFA_OUT, "pPattern or pMask is null");
        //zlog_info(zc, "pPattern or pMask is null\r\n");
        goto Exit;
    }
  
    memset(pPattern, 0, sizeof(IP_PATTERN));

    pPattern->DstAddr = ((SOCKADDR_IN *)&Address)->sin_addr.s_addr;
    pPattern->tcDstPort = htons(Port);
    pPattern->ProtocolId = ProtocolId;

    memset(pMask, (ULONG)-1, sizeof(IP_PATTERN));
    //
    // Set the source address and port to wildcard
    // 0 -> wildcard, 0xFF-> exact match 
    //
    pMask->SrcAddr = 0;
    pMask->tcSrcPort = 0;

    //
    // If the user specified 0 for dest port, dest address or protocol
    // set the appropriate mask as wildcard
    // 0 -> wildcard, 0xFF-> exact match 
    //

    if(pPattern->tcDstPort == 0)
    {
        pMask->tcDstPort = 0;
    }

    if(pPattern->ProtocolId == 0)
    {
        pMask->ProtocolId = 0;
    }

    if(pPattern->DstAddr == 0)
    {
        pMask->DstAddr = 0;
    }

    pFilter->AddressType = NDIS_PROTOCOL_ID_TCP_IP;
    pFilter->PatternSize = sizeof(IP_PATTERN);
    pFilter->Pattern = pPattern;
    pFilter->Mask = pMask;

    //
    // Delete any previous instances of the Filter
    //
    DeleteFilter(ppFilter);
    *ppFilter = pFilter;

    status = TRUE;

Exit:
    if(!status)
    {
        ////DPRINT_INFO(WFA_OUT, "Filter Creation Failed\n");
        //zlog_info(zc, "Filter Creation Failed\r\n");
        DeleteFilter(&pFilter);
    }
    else
    {
        ////DPRINT_INFO(WFA_OUT, "Filter Creation Succeeded\n");
        //zlog_info(zc, "Filter Creation Succeeded\r\n");
    }

    return status; 
}

//
// Routine: 
// ClNotifyHandler
// Description:
// Empty notification handler.
// The ClNotifyHandler function is used by traffic control to notify the client of various 
// traffic control$(Dx7(Bspecific events, including the deletion of flows, changes in filter parameters,
// or the closing of an interface.
// Arguments:
// [in] ClRegCtx -
//  Client registration context, provided to traffic control by the client with the client's call
//  to the TcRegisterClient function.
// [in] ClIfcCtx -
//  Client interface context, provided to traffic control by the client with the client's call to
//  the TcOpenInterface function. Note that during a TC_NOTIFY_IFC_UP event, ClIfcCtx is not
//  available and will be set to NULL.
// [in] Event -
//  Describes the notification event. See the Remarks section for a list of notification events.
// [in] SubCode -
//  Handle used to further qualify a notification event.
// [in] BufSize -
//  Size of the buffer included with the notification event, in bytes.
// [in] Buffer -
//  Buffer containing the detailed event information associated with Event and SubCode.
// Return:
// None
//
void ClNotifyHandler( IN HANDLE ClRegCtx,
                      IN HANDLE ClIfcCtx,
                      IN ULONG Event,
                      IN HANDLE SubCode,
                      IN ULONG BufSize,
                      IN PVOID Buffer)
{
    UNREFERENCED_PARAMETER(ClRegCtx);
    UNREFERENCED_PARAMETER(ClIfcCtx);
    //
    // Notification was unexpected
    //
    ////DPRINT_INFO(WFA_OUT, "Unexpected notification: Event=%d, SubCode=%p, BufSize=%d, Buffer=%p", 
    ////                            (int)Event, (void *)SubCode, (int)BufSize, Buffer);
    //zlog_info(zc, "Unexpected notification: Event=%d, SubCode=%p, BufSize=%d, Buffer=%p\r\n", (int)Event, (void *)SubCode, (int)BufSize, Buffer);
}

//
// Routine: 
// ClearIfcList
// Description:
// Clears the IfcList and its member variables
// Arguments:
// [in] pIfcList -
//  The interface list.
// Return:
// TRUE if successes or FALSE if fails.
//
BOOL ClearIfcList( IN PIFC_LIST pIfcList)
{
    ULONG i;

    if(!pIfcList)
    {
        return TRUE;
    }

    if(pIfcList->pIfcInfo)
    {
        //
        // Delete filter, flow and interface
        //
        PIFC_INFO pCurrentIfcInfo = pIfcList->pIfcInfo;

        for(i = 0; i < pIfcList->IfcCount; i ++)
        {
            if(pCurrentIfcInfo->hFilter)
            {
            TcDeleteFilter(pCurrentIfcInfo->hFilter);
            }
            if(pCurrentIfcInfo->hFlow)
            {
            TcDeleteFlow(pCurrentIfcInfo->hFlow);
            }
            if(pCurrentIfcInfo->hIfc)
            {
            TcCloseInterface(pCurrentIfcInfo->hIfc);
            }

            pCurrentIfcInfo++;
        }

        free(pIfcList->pIfcInfo);
    }

    ZeroMemory(pIfcList, sizeof(IFC_LIST));

    return TRUE;
}


//
// Routine: 
// MakeIfcList
// Description:
// The function enumerates all TC enabled interfaces. 
// opens each TC enabled interface and stores each ifc handle in IFC_LIST struct
// pointed to by pIfcList.
// Arguments:
// [in] hClient -
//  Handle returned by TcRegisterClient
// [in] pIfcList -
//  ptr to IfcList structure which will be populated by the function
// Return:
// TRUE if successes or FALSE if fails.
//
BOOL
MakeIfcList( IN HANDLE  hClient,
             IN PIFC_LIST pIfcList)
{
    BOOL    status = FALSE;
    ULONG    err = ERROR_INVALID_PARAMETER;

    ULONG     BufferSize = 1, ActualBufferSize, RemainingBufferSize = 0;
    PTC_IFC_DESCRIPTOR pIfcBuffer = NULL, pCurrentIfc;
    PIFC_INFO   pIfcInfo = NULL, pCurrentIfcInfo;
    ULONG    nIfcs = 0;

    //
    // Enumerate the TC enabled interfaces
    //
    while(TRUE)
    {
        ActualBufferSize = BufferSize;
        pIfcBuffer = (PTC_IFC_DESCRIPTOR)malloc(ActualBufferSize);
        if(pIfcBuffer == NULL)
        {
            break;
        }

        err = TcEnumerateInterfaces(hClient, &ActualBufferSize, pIfcBuffer);
        if(err == ERROR_INSUFFICIENT_BUFFER)
        {
            free(pIfcBuffer);
            BufferSize *= 2;
        }
        else
        {
            break;
        }
    }

    if(err != NO_ERROR)
    {
        goto Exit;
    }

    //
    // Count the number of interfaces
    //

    pCurrentIfc = pIfcBuffer;
    RemainingBufferSize = ActualBufferSize;
    while(RemainingBufferSize)
    {
        nIfcs ++;

        RemainingBufferSize -= pCurrentIfc->Length;
        pCurrentIfc = (PTC_IFC_DESCRIPTOR)(((PBYTE)pCurrentIfc) + pCurrentIfc->Length);
    }
  
    if(nIfcs == 0)
    {
        goto Exit;
    }

    //
    // Allocate memory for the size(IFC_INFO) X nIfcs
    // 
    pIfcInfo = (PIFC_INFO)malloc(sizeof(IFC_INFO) * nIfcs);
    if(!pIfcInfo)
    {
        goto Exit;
    }
  
    ZeroMemory(pIfcInfo, sizeof(IFC_INFO) * nIfcs);

    ClearIfcList(pIfcList);
    pIfcList->IfcCount = nIfcs;
    pIfcList->pIfcInfo = pIfcInfo;

    //
    // Open Each interface and store the ifc handle in ifcList
    //
    pCurrentIfc = pIfcBuffer;
    pCurrentIfcInfo = pIfcInfo;

    RemainingBufferSize = ActualBufferSize;
    while(RemainingBufferSize)
    {
        HANDLE hIfc;

        err = TcOpenInterfaceW(pCurrentIfc->pInterfaceName, 
                    hClient,
                    0,
                    &hIfc);
  
        if(err != NO_ERROR)
        {
            ////DPRINT_ERR(WFA_ERR, "TcOpenInterface Failed %d\n", err);
            //zlog_info(zc, "TcOpenInterface Failed %d\r\n", err);
            break;
        }

        pCurrentIfcInfo->hIfc = hIfc;

        RemainingBufferSize -= pCurrentIfc->Length;
        pCurrentIfc = (PTC_IFC_DESCRIPTOR)(((PBYTE)pCurrentIfc) + pCurrentIfc->Length);
        pCurrentIfcInfo ++;
    }
  
    if(err != NO_ERROR)
    {
        goto Exit;
    }

    status = TRUE;

Exit:
    if(!status)
    {
        ClearIfcList(pIfcList);
    }
   
    //
    // Cleanup the IfcBuffer
    //
    if(pIfcBuffer)
    {
        free(pIfcBuffer);
    }

    return status;
}

//
// Routine: 
// AddTcFlows
// Description:
// Add Tc Flow in pTcFlow to each interface in IfcList.
// Arguments:
// [in] IfcList -
//  The interface list.
// [in] pTcFlow -
//  The TC flow to add.
// Return:
// TRUE if successes. Otherwise return FALSE.
//
BOOL AddTcFlows( IN IFC_LIST  IfcList,
                 IN PTC_GEN_FLOW pTcFlow)
{
    UINT   i;
    ULONG  err;
    BOOL  status = FALSE;
    PIFC_INFO pCurrentIfcInfo = IfcList.pIfcInfo;

    //
    // For each interface in the list, add a TC flow
    //
    for(i = 0; i < IfcList.IfcCount; i++)
    {
        HANDLE hFlow;

        err = TcAddFlow(pCurrentIfcInfo->hIfc, 0, 0, pTcFlow, &hFlow);		
        if(err != NO_ERROR)
        {
            ////DPRINT_ERR(WFA_ERR, "TcAddFlow Failed %d\n", err);
            //zlog_error(zc, "TcAddFlow Failed %d\r\n", err);
            goto Exit;
        }

        pCurrentIfcInfo->hFlow = hFlow;
        pCurrentIfcInfo++;
    }

   status = TRUE;

Exit:
   return status;
}

//
// Routine: 
// AddTcFilters
// Description:
// Add Tc Filter in pTcFilter to each interface in IfcList
// Arguments:
// [in] IfcList -
//  The interface list.
// [in] pTcFilter -
//  The filter to apply.
// Return:
// TRUE if successes. Otherwise return FALSE.
//
BOOL AddTcFilters( IN IFC_LIST  IfcList,
                   IN PTC_GEN_FILTER pTcFilter)
{
    UINT  i;
    ULONG  err;
    BOOL  status = FALSE;
    PIFC_INFO pCurrentIfcInfo = IfcList.pIfcInfo;

    //
    // For each interface in the list, add TC filter on the corresponding TcFlow
    //
    for(i = 0; i < IfcList.IfcCount; i++)
    {
        HANDLE hFilter;

        err = TcAddFilter(pCurrentIfcInfo->hFlow, pTcFilter, &hFilter);
        if(err != NO_ERROR)
        {
            ////DPRINT_ERR(WFA_ERR, "TcAddFilter Failed %d\n", err);
            //zlog_error(zc, "TcAddFilter Failed %d\r\n", err);
            goto Exit;
        }

        pCurrentIfcInfo->hFilter = hFilter;
        pCurrentIfcInfo++;      
    }

    status = TRUE;

Exit:
    return status;
}


/* NOTE, to run this version of code, win_dut have to run under administrator mode
* wfaTGSetPrio_Win7(): This depends on the network interface card.
*               So you might want to remap according to the driver
*               provided.
*               The current implementation is to set the TOS/DSCP bits
*               in the IP header
* All related info see doc from MS, key word; traffic control API
*  http://msdn.microsoft.com/en-us/library/windows/desktop/aa373391(v=vs.85).aspx
* sample code 
* http://blogs.msdn.com/b/wndp/archive/2007/11/14/throttling-dscp-and-802-1p-with-the-traffic-control-api.aspx
* Go to the Microsoft Connect website, choose Available Connections on the left-hand
* side of the page, and select Windows Networking from the available connections 
* (bottom half of the page). On the left-hand side of the Windows Networking page, 
* choose Downloads, and select TCMonLite.
* reference:
*  http://blogs.msdn.com/b/wndp/archive/2007/10/09/introduction-to-windows-qos-traffic-control.aspx
*  http://blogs.msdn.com/b/wndp/archive/2006/07/05/657196.aspx   TC approach will be removed in the future.
*  http://technet.microsoft.com/en-us/magazine/2007.02.cableguy.aspx   QoS Support in Windows

*  http://www.nessoft.com/kb/article/setting-dscp-qos-byte-on-packets-with-windows-7-8-95.html
*/

/*
* wfaTGSetPrio(): This depends on the network interface card.
*               So you might want to remap according to the driver
*               provided.
*               The current implementation is to set the TOS/DSCP bits
*               in the IP header
*/
int wfaTGSetPrio(int sockfd, int tgClass, int streamid)
{
	int tosval =0, ret = 0, qos=0, threadPri=0;
	int threadPrio = THREAD_PRIORITY_NORMAL;
	tgProfile_t *pTGProfile = findTGProfile(streamid);
	int size = sizeof(tosval);
	getsockopt(sockfd, IPPROTO_IP, IP_TOS, (char *)&tosval, &size);

	if (pTGProfile == NULL)
	{
		return WFA_FAILURE;
	}

	ret = wfaACClassToQos(tgClass, &qos, &(tosval), &threadPri);

	if (pTGProfile->tgTC[0].pTcFilter != NULL && pTGProfile->tgTC[0].Qos == qos)
	//if (pTGProfile->tgTC[0].Qos == qos)
	{
		DPRINT_INFOL(WFA_OUT,"wfaTGSetPrio, same QoS do nothing QoS=0x%x\n", qos);
	}
	else
	{
		//wfaCloseTrafficControlFlow(&(pTGProfile->tgTC[0]));
		//Sleep(50);
		pTGProfile->tgTC[0].Qos = qos;
		pTGProfile->tgTC[0].tosVal = tosval;

		ret = wfaOpenTrafficControlFlow(sockfd, tgClass, pTGProfile, &(pTGProfile->tgTC[0]));
		if (ret == WFA_SUCCESS)
		{
			DPRINT_INFOL(WFA_OUT,"wfaTGSetPrio, wfaOpenTrafficControlFlow OK sockfd=%d tosVal=0x%x\n",sockfd, pTGProfile->tgTC[0].tosVal);
			tosval = pTGProfile->tgTC[0].tosVal;
		}
		else
		{
			DPRINT_INFOL(WFA_OUT,"ERR wfaTGSetPrio, wfaOpenTrafficControlFlow faile sockfg=%d tosVal=0x%x\n",sockfd, pTGProfile->tgTC[0].tosVal);
		}
	}

#ifdef WFA_WMM_PS
	wfaTGWMMPSData.psTxMsg[1] = tosval;
#endif

	//DPRINT_INFOL(WFA_OUT, "wfaTGSetPrio set the TOS as 0x%x\n",tosval);
	return (tosval == 0xE0)?0xD8:tosval;
}/*  wfaTGSetPrio  */

#ifdef WFA_WMM_WPA2
/* 
* collects the traffic statistics from other threads and 
* sends the collected information to CA
*/
void  wfaSentStatsResp(int sock, BYTE *buf)
{
	int i, total=0, pkLen;
	tgStream_t *allStreams = wfaTGWMMData.gStreams;
	dutCmdResponse_t *sendStatsResp = (dutCmdResponse_t *)buf, *first;
	char buff[WFA_BUFF_4K];
	DPRINT_INFOL(WFA_OUT, "wfaSentStatsResp: gstream is %d\n",wfaTGWMMData.gStreams);
	if(sendStatsResp == NULL)
		return;

	first = sendStatsResp;

	for(i = 0; i < WFA_MAX_TRAFFIC_STREAMS; i++)
	{
		if((allStreams->id != 0) && (allStreams->profile.direction == DIRECT_SEND) && (allStreams->state == WFA_STREAM_ACTIVE))
		{
			sendStatsResp->status = STATUS_COMPLETE;
			sendStatsResp->streamId = allStreams->id;
			memcpy(&sendStatsResp->cmdru.stats, &allStreams->stats, sizeof(tgStats_t));          

			/*DPRINT_INFOL(WFA_OUT, "id=%i rxFrames=%i txFrames=%i rxPayLoadBytes=%i txPayloadBytes=%i lastPktSN=%i\n", allStreams->id,
				allStreams->stats.rxFrames,
				allStreams->stats.txFrames,
				allStreams->stats.rxPayloadBytes,
				allStreams->stats.txPayloadBytes,
				allStreams->lastPktSN); */

            DPRINT_INFOL(WFA_OUT, "id=%i rxFrames=%i txActFrames=%i txFrames=%i rxPayLoadBytes=%llu txPayloadBytes=%llu lastPktSN=%i\r\n", allStreams->id,allStreams->stats.rxFrames,allStreams->stats.txActFrames,allStreams->stats.txFrames,allStreams->stats.rxPayloadBytes,allStreams->stats.txPayloadBytes,allStreams->lastPktSN); 

			sendStatsResp++;
			total++;
		} 
		allStreams++;
	} 

	wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, total*sizeof(dutCmdResponse_t),
		(BYTE *)first, (BYTE *)buff);

	pkLen = WFA_TLV_HDR_LEN + total*sizeof(dutCmdResponse_t); 

	if(wfaCtrlSend(sock, (BYTE *)buff, pkLen) != pkLen)
	{
		DPRINT_WARNING(WFA_WNG, "wfaCtrlSend Error\n");
		// reSend one more time
		wfaCtrlSend(sock, (BYTE *)buff, pkLen);
	}
	DPRINT_INFOL(WFA_OUT, "wfaSentStatsResp exit: gstream is %d\n",wfaTGWMMData.gStreams);
	return;
}
#endif

#if defined(WFA_WMM_PS) || defined(WFA_WMM_AC)
/*
* sender(): This is a generic function to send a packed for the given dsc 
*               (ac:VI/VO/BE/BK), before sending the packet the function
*               puts the station into the PS mode indicated by psave and 
*               sends the packet after sleeping for sllep_period
*/
int sender(char psave,int sleep_period,int dsc)
{
	int r=0;
	tgProfile_t *pTGProfile = findTGProfile(wfaTGWMMPSData.wmmps_info.streamid);

	DPRINT_INFOL(WFA_OUT,"\nWMMPS sender Sleeping for ms %d\n",sleep_period/1000);
	wfaSetDUTPwrMgmt(psave);
	wfaCreateUDPSockWmmpsSend(pTGProfile->dipaddr, dsc);
	if (wfaTGWMMPSData.wmmps_info.psSendSockFd[1]  > 0)
	{
		create_apts_msg(APTS_DEFAULT, wfaTGWMMPSData.psTxMsg, wfaTGWMMPSData.wmmps_info.my_sta_id);
		wfaTGWMMPSData.psTxMsg[1] = pTGProfile->tgTC[1].tosVal; // must set to let PCEnd know it
		Sleep(sleep_period/1000);
		DPRINT_INFOL(WFA_OUT,"\nAfter create msg tos type=%d, sendto, socketFd=%d\n",dsc, wfaTGWMMPSData.wmmps_info.psSendSockFd[1]);
		r = sendto(wfaTGWMMPSData.wmmps_info.psSendSockFd[1], (char*)wfaTGWMMPSData.psTxMsg, /*msgsize*/ WMMPS_MSG_BUF_SIZE/4, 0, (struct sockaddr *)&wfaTGWMMPSData.wmmps_info.psToAddr, sizeof(struct sockaddr));
		wfaCloseUDPSockWmmpsSend();
	}
	else
	{
		DPRINT_INFOL(WFA_OUT,"\nERR WMMPS sender socket fd err fd=%d\n", wfaTGWMMPSData.wmmps_info.psSendSockFd[1]);
	}
	return r;
}

/*
* wfaStaSndHello(): This function sends a Hello packet 
*                and sleeps for sleep_period, the idea is
*                to keep sending hello packets till the console
*                responds, the function keeps a check on the MAX
*                Hellos and if number of Hellos exceed that it quits
*/
int WfaStaSndHello(char psave,int sleep_period,int *state)
{
	int r;
	tgWMMPS_t *my_wmm=&wfaTGWMMPSData.wmmps_mutex_info;

	if(!(wfaTGWMMPSData.num_hello++))
	{
		wfaSetDUTPwrMgmt(psave);
		create_apts_msg(APTS_HELLO, wfaTGWMMPSData.psTxMsg,0);
		r = sendto(wfaTGWMMPSData.psSockfd, (char*)wfaTGWMMPSData.psTxMsg, sizeof(wfaTGWMMPSData.psTxMsg), 0, (struct sockaddr *)&wfaTGWMMPSData.wmmps_info.psToAddr, sizeof(struct sockaddr));
		DPRINT_INFOL(WFA_OUT, "WfaStaSndHello sent.\n");
	}

	Sleep(sleep_period/1000);

	pthread_mutex_lock(&my_wmm->thr_flag_mutex);
	if(my_wmm->thr_flag && wfaTGWMMPSData.wmmps_info.rcv_state==1)
	{
		(*state)++;
		wfaTGWMMPSData.num_hello=0;
		my_wmm->thr_flag=0;
		DPRINT_INFOL(WFA_OUT, "WfaStaSndHello rcv_state=%d *state=%d\n", wfaTGWMMPSData.wmmps_info.rcv_state, *state );
	}

	pthread_mutex_unlock(&my_wmm->thr_flag_mutex);
	if(wfaTGWMMPSData.num_hello > MAXHELLO)
	{
		DPRINT_ERR(WFA_ERR, "Too many Hellos sent;close UDP socket.\n");
		wfaTGWMMPSData.gtgWmmPS = 0;
		wfaTGWMMPSData.num_hello=0;
		closesocket(wfaTGWMMPSData.psSockfd);
		wfaTGWMMPSData.psSockfd = -1;
	} 

	return 0;
}

/*
* wfaStaSndConfirm(): This function sends the confirm packet
*                which is sent after the console sends the
*                test name to the station
*/
int WfaStaSndConfirm(char psave,int sleep_period,int *state)
{
	int r;
	static int num_hello=0; // reset counter as original code, will update later
	DPRINT_INFOL(WFA_OUT, "WfaStaSndConfirm psave=%d *state=%d\n",psave,*state);
	wfaSetDUTPwrMgmt(psave);// tmp removed 06/02/2014 benz
	Sleep(800);
	{
		create_apts_msg(APTS_CONFIRM, wfaTGWMMPSData.psTxMsg,0);
		r = sendto(wfaTGWMMPSData.psSockfd, (char*)wfaTGWMMPSData.psTxMsg, wfaTGWMMPSData.msgsize, 0, (struct sockaddr *)&wfaTGWMMPSData.wmmps_info.psToAddr, sizeof(struct sockaddr));
		(*state)++;
	}

	return 0;
}

/*
* WfaStaSndVO(): This function sends a AC_VO packet
*                after the time specified by sleep_period
*                and advances to the next state for the given test case
*/
int WfaStaSndVO(char psave,int sleep_period,int *state)
{
	int r;
	static int en=1;
	DPRINT_INFOL(WFA_OUT, "\r\nEnterring WfaStaSndVO %d",en++);
	if ((r=sender(psave,sleep_period,TG_WMM_AC_VO))>=0)
		(*state)++;
	else
		DPRINT_INFOL(WFA_OUT, "\r\nError\n");

	return 0;
}

/*
* WfaStaSnd2VO(): This function sends two AC_VO packets
*                after the time specified by sleep_period
*                and advances to the next state for the given test case
*/
int WfaStaSnd2VO(char psave,int sleep_period,int *state)
{
	int r;
	static int en=1;

	DPRINT_INFOL(WFA_OUT,"\r\nEnterring WfaStaSnd2VO %d",en++);
	if ((r=sender(psave,sleep_period,TG_WMM_AC_VO))>=0)
	{
		r = sendto(wfaTGWMMPSData.psSockfd, (const char*)wfaTGWMMPSData.psTxMsg, wfaTGWMMPSData.msgsize, 0, (struct sockaddr *)&wfaTGWMMPSData.wmmps_info.psToAddr, sizeof(struct sockaddr));
		mpx("STA msg",wfaTGWMMPSData.psTxMsg,64);
		(*state)++;
	}
	else
		DPRINT_INFOL(WFA_OUT, "\r\nError\n");

	return 0;
}

/*
* WfaStaSndVI(): This function sends a AC_VI packet
*                after the time specified by sleep_period
*                and advances to the next state for the given test case
*/
int WfaStaSndVI(char psave,int sleep_period,int *state)
{
	int r;
	static int en=1;

	DPRINT_INFOL(WFA_OUT, "\r\nEnterring WfaStaSndVI %d",en++);
	if ((r=sender(psave,sleep_period,TG_WMM_AC_VI))>=0)
		(*state)++;
	else
	{
		DPRINT_ERR(WFA_ERR, "\r\nEnterring WfaStaSndVI send err r=%d",r);
	}
	return 0;
}

/*
* WfaStaSndBE(): This function sends a AC_BE packet
*                after the time specified by sleep_period
*                and advances to the next state for the given test case
*/
int WfaStaSndBE(char psave,int sleep_period,int *state)
{
	int r;
	static int en=1;

	DPRINT_INFOL(WFA_OUT,"\r\nEnterring  WfaStaSndBE %d",en++);
	if ((r=sender(psave,sleep_period,TG_WMM_AC_BE))>=0)
		(*state)++;

	return 0;
}
/*
* WfaStaSndBK(): This function sends a AC_BK packet
*                after the time specified by sleep_period
*                and advances to the next state for the given test case
*/
int WfaStaSndBK(char psave,int sleep_period,int *state)
{
	int r;
	static int en=1;

	DPRINT_INFOL(WFA_OUT,"\r\nEnterring WfaStaSndBK %d",en++);
	if ((r=sender(psave,sleep_period,TG_WMM_AC_BK))>=0)
		(*state)++;

	return 0;
}

/*
* WfaStaSndVOCyclic(): The function is the traffic generator for the L.1 test
*                      caseThis function sends 3000 AC_VO packet
*                      after the time specified by sleep_period (20ms)
*/
int WfaStaSndVOCyclic(char psave,int sleep_period,int *state)
{
	int i;
	static int en=1;

	for(i=0;i<3000;i++)
	{
		DPRINT_INFOL(WFA_OUT,"\r\nEnterring WfaStaSndVOCyclic %d",en++);
		sender(psave,sleep_period,TG_WMM_AC_VO);
		if(!(i%50))
		{
			DPRINT_INFOL(WFA_OUT,".");
			fflush(stdout);
		}
	}
	en  = 1;
	(*state)++;

	return 0;
}

int WfaStaSndStop(char psave,int sleep_period,int *state)
{
	int r=0;
	DPRINT_INFOL(WFA_OUT,"\nEntering WfaStaSndStop, sleep ms:%d\n",sleep_period/1000);

	Sleep(sleep_period/1000);

	create_apts_msg(APTS_STOP, wfaTGWMMPSData.psTxMsg, wfaTGWMMPSData.wmmps_info.my_sta_id);
	if (wfaTGWMMPSData.psSockfd > 0)
	{
		wfaSetDUTPwrMgmt(psave);
		wfaTGSetPrio(wfaTGWMMPSData.psSockfd, TG_WMM_AC_BE, wfaTGWMMPSData.wmmps_info.streamid );
		r = sendto(wfaTGWMMPSData.psSockfd, (char*)wfaTGWMMPSData.psTxMsg, wfaTGWMMPSData.msgsize, 0, (struct sockaddr *)&wfaTGWMMPSData.wmmps_info.psToAddr, sizeof(struct sockaddr));
		mpx("STA msg sent STOP", wfaTGWMMPSData.psTxMsg,64);
	}
	else
	{
		DPRINT_INFOL(WFA_OUT,"WfaStaSndStop, socket already closed psSockfd=%d num_stops=%d\n", wfaTGWMMPSData.psSockfd, wfaTGWMMPSData.num_stops);
		wfaTGWMMPSData.gtgWmmPS = 0;
		wfaTGWMMPSData.psSockfd = -1;
		wfaTGWMMPSData.num_hello=0;
		wfaTGWMMPSData.resetsnd=1;
		wfaTGWMMPSData.resetrcv=1;
	}
	wfaTGWMMPSData.num_stops++;
	if((wfaTGWMMPSData.num_stops > MAX_STOPS) || (wfaTGWMMPSData.psSockfd == -1))
		(*state)++;
	return 0;
}

#endif 

#ifdef WFA_WMM_WPA2
DWORD WINAPI wfa_wmm_sleep_thread(void *thr_param)
{
	int sleep_prd = *(int *)thr_param;
	Sleep(sleep_prd);
	tmout_stop_send(0);
	return 0;
}

DWORD WINAPI wfa_wmm_thread(void *thr_param)
{
	int myId = ((tgThrData_t *)thr_param)->tid;
	tgThrData_t *tdata =(tgThrData_t *) thr_param;
	tgWMM_t *my_wmm = &wfaTGWMMData.wmm_thr[myId];
	tgStream_t *myStream = NULL;
	int myStreamId;
	int mySock, status, respLen;
	tgProfile_t *myProfile;
	BYTE respBuf[WFA_BUFF_4K];
	int resendcnt = 0;
    DWORD thr_id;
	int timer_dur;
	int iOptVal;
	int iOptLen = sizeof(int);
    int bufActualSize = 0;
    ULONG iResult = 0L;
    u_long iMode = 1;

	while(1)
	{
		DPRINT_INFOL(WFA_OUT, "wfa_wmm_thread::Prestart %d ...\n",myId);
		fflush(stdout);
		while(!my_wmm->thr_flag)
		{
			WaitForSingleObject(my_wmm->thr_flag_mutex, INFINITE);            
			Sleep(20);
		}

		ReleaseMutex(my_wmm->thr_flag_mutex);  
		myStreamId = my_wmm->thr_flag;
		my_wmm->thr_flag = 0;
		DPRINT_INFOL(WFA_OUT, "wfa_wmm_thread::lock met %d ...\n",myId);

		/* use the flag as a stream id to file the profile */ 
		myStream = findStreamProfile(myStreamId); 
		myProfile = &myStream->profile;

		if(myProfile == NULL)
		{
			status = STATUS_INVALID;
			wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, 4, (BYTE *)&status, respBuf);
			respLen = WFA_TLV_HDR_LEN+4;
			/*
			* send it back to control agent.
			*/
			continue;
		}

		DPRINT_INFOL(WFA_OUT, "wfa_wmm_thread::start new ...\n");
		switch(myProfile->direction)
		{
		case DIRECT_SEND:
			DPRINT_INFOL(WFA_OUT, "creating send socket with port %d ...\n",myProfile->sport);
            if ((mySock = wfaCreateSock(myProfile->transProtoType, myProfile->sipaddr, myProfile->sport)) == WFA_FAILURE)
            {
                DPRINT_ERR(WFA_ERR, "can't creat socket\n");
                continue;
            }

            DPRINT_INFOL(WFA_OUT, "Current transProtoProfile is %i and connecting to %s and port %i\n", myProfile->transProtoType, myProfile->dipaddr, myProfile->dport);

			if (wfaConnectToPeer(myProfile->transProtoType, mySock, myProfile->dipaddr, myProfile->dport) == WFA_FAILURE)
            {
                DPRINT_INFOL(WFA_OUT, "can't connect to %s and port %i\n", myProfile->dipaddr, myProfile->dport);
                continue;
            }

			//if (myProfile->transProtoType == SOCK_TYPE_TCP)
			{
			iResult = ioctlsocket(mySock, FIONBIO, &iMode);
			DPRINT_INFOL(WFA_OUT, "ioctlsocket result: %d\n", iResult);
			if (iResult != NO_ERROR)
				DPRINT_INFOL(WFA_OUT, "ioctlsocket failed with error: %ld\n", iResult);
			}

			/*
			* Set packet/socket priority TOS field
			*/
			DPRINT_INFOL(WFA_OUT, "To Set QoS Traffic Class=%i\n", myProfile->trafficClass);
			wfaTGSetPrio(mySock, myProfile->trafficClass,myStreamId);
			//wfaTGSetPrio_Win7(mySock, myProfile->trafficClass, myProfile);
			/* if delay is too long, it must be something wrong */
			if(myProfile->startdelay > 0 && myProfile->startdelay<100)
			{
				Sleep(1000 * myProfile->startdelay);
			}

			//memset(respBuf, 0, WFA_BUFF_4K);      // benz added 

			/*
			* set timer fire up
			*/
			DPRINT_INFOL(WFA_OUT, "Thread %d Setting timer for %d ms\n",myId,1000*myProfile->duration);
			timer_dur = 1000*(myProfile->duration + 1);  /* add one missing second   */          
            CreateThread(NULL, 0, wfa_wmm_sleep_thread, (PVOID)&timer_dur, 0, &thr_id);

			if (getsockopt(mySock, SOL_SOCKET, SO_SNDBUF, (char*)&iOptVal, &iOptLen) != SOCKET_ERROR) 
			{
				DPRINT_INFOL(WFA_OUT, "SO_SNDBUF Value: %ld\n", iOptVal);
			}
			else
			{
				DPRINT_INFOL(WFA_OUT, "Error to get Default SO_RECVBUF= %ld \n", iOptVal); 
			}		   
			iOptVal = 128 * 1024;//iOptVal*40;  /* one mbyte buffer size or 128 * 1024? */

			if ( setsockopt(mySock, SOL_SOCKET, SO_SNDBUF, (char*)&iOptVal, iOptLen) != SOCKET_ERROR) 
			{
				DPRINT_INFOL(WFA_OUT, "Set SO_SNDBUF Value: %ld  OK\n", iOptVal);
			}
			else
			{
				DPRINT_INFOL(WFA_OUT, "Error to set SO_SNDBUF Value: %ld\n", iOptVal);
			}
			if (getsockopt(mySock, SOL_SOCKET, SO_SNDBUF, (char*)&iOptVal, &iOptLen) != SOCKET_ERROR) 
			{
				DPRINT_INFOL(WFA_OUT, "get resetted SO_SNDBUF Value: %ld\n", iOptVal);
			}			


			/* ----------detect some fixed bit rate cases on Sending ----------------  */
			if ((myProfile->rate != 0 ) /* WFA_SEND_FIX_BITRATE_MAX_FRAME_RATE)*/ && 
				(myProfile->pksize * myProfile->rate * 8 < WFA_SEND_FIX_BITRATE_MAX))
			{
				wfaSendBitrateData(mySock, myStreamId, respBuf, &respLen);
			}
			else
			{
				wfaSendLongFile(mySock, myStreamId, respBuf, &respLen);
			}

			wfaCloseTrafficControlFlow(&(myProfile->tgTC[0]));

			DPRINT_INFOL(WFA_OUT, "Closing socket for thread %d sockId=%i\n",myId, mySock);
			closesocket(mySock);
			Sleep(1000);

			/*
			* uses thread 0 to pack the items and ships it to CA.
			*/
			if(myId == wfaTGWMMData.mainSendThread) 
			{
				wfaSentStatsResp(wfaDutAgentData.gxcSockfd, respBuf);  
				Sleep(3000);
				DPRINT_INFOL(WFA_OUT, "finish resendsn %i", resendcnt);
			}

			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
			break;

		case DIRECT_RECV:
			if(myProfile->profile == PROF_IPTV || myProfile->profile == PROF_FILE_TX || myProfile->profile == PROF_MCAST)
			{
				int nbytes = 0;
				char *recvBuf;
				int iOptVal;//, iOptLen;
				int iOptLen = sizeof(int);

#ifdef WFA_VOICE_EXT
				struct timeval currtime;
				FILE *e2eoutp = NULL;
				char e2eResults[256];
				int le2eCnt = 0;
				tgE2EStats_t *e2esp = NULL;
				int totalE2Cnt = 6000;
#endif
				if (myProfile->transProtoType == SOCK_TYPE_TCP)
				{
					if ((wfaTGWMMData.svrSock[myStream->tblidx] = wfaCreateTCPServSockImpl(myProfile->dipaddr, myProfile->dport)) == WFA_FAILURE)
					{
						DPRINT_ERR(WFA_ERR, "can't create server socket to %s and port %i\r\n", myProfile->dipaddr, myProfile->dport);                       
						continue;
					}
				}
				else
				{
					if ((mySock = wfaCreateSock(myProfile->transProtoType, myProfile->sipaddr, myProfile->sport)) == WFA_FAILURE)                  
					{
						DPRINT_ERR(WFA_ERR, "can't create socket at %s and port %i\r\n", myProfile->dipaddr, myProfile->dport);                   
						continue;
					}
				}

				if (myProfile->transProtoType == SOCK_TYPE_TCP)
				{
					DPRINT_INFOL(WFA_OUT, "Before accepting connection\r\n");
					if ((mySock = wfaAcceptTCPConn(wfaTGWMMData.svrSock[myStream->tblidx])) == WFA_FAILURE)
					{
						DPRINT_ERR(WFA_ERR, "can't accept the connection\r\n");
						closesocket(wfaTGWMMData.svrSock[myStream->tblidx]);
						continue;
					}
					DPRINT_INFOL(WFA_OUT, "After accepting connection\r\n");
				}
				else
				{
					if (wfaConnectToPeer(myProfile->transProtoType, mySock, myProfile->dipaddr, myProfile->dport) == WFA_FAILURE)
					{
                       DPRINT_ERR(WFA_ERR, "can't connect to %s and port %i\r\n", myProfile->dipaddr, myProfile->dport);
                       closesocket(mySock);
                       continue;
					}
				}

				wfaTGWMMData.tgSockfds[myStream->tblidx] = mySock; 

#ifdef WFA_VOICE_EXT
				if(myProfile->profile == PROF_IPTV)
				{
					e2esp = (tgE2EStats_t *)malloc(totalE2Cnt * sizeof(tgE2EStats_t));
					if(e2esp == NULL)
					{
						DPRINT_INFOL(WFA_OUT, "Failed to malloc e2esp\n");
					}
				}
				myStream->e2ebuf = e2esp;
#endif
				if (getsockopt(mySock, SOL_SOCKET, SO_RCVBUF, (char*)&iOptVal, &iOptLen) != SOCKET_ERROR) 
				{
					DPRINT_INFOL(WFA_OUT, "Current SO_RCVBUF Value: %ld\n", iOptVal);                  
				}
				else
				{
					DPRINT_ERR(WFA_OUT, "before setting SO_RECVBUF, getsockopt failed with error %d\n", WSAGetLastError());
				}
				iOptVal = 128 * 1024; // 128 K
				if (setsockopt(mySock, SOL_SOCKET, SO_RCVBUF, (char*)&iOptVal, iOptLen) != SOCKET_ERROR)
				{
					DPRINT_INFOL(WFA_OUT, "set SO_RCVBUF Value OK: %ld\n", iOptVal);  
				}
				else
				{
					DPRINT_ERR(WFA_OUT, "set SO_RECVBUF failed with error %d\n", WSAGetLastError());
				}

				if (getsockopt(mySock, SOL_SOCKET, SO_RCVBUF, (char*)&iOptVal, &iOptLen) != SOCKET_ERROR) 
				{
					DPRINT_INFOL(WFA_OUT, "set SO_RCVBUF Value: %ld\n", iOptVal);
				}
				else
				{
					DPRINT_ERR(WFA_OUT, "after setting SO_RECVBUF, getsockopt failed with error %d\n", WSAGetLastError());
				}

                if (myProfile->pksize <= 0)
                {
                    DPRINT_INFOL(WFA_OUT, "Receiver zero packet size detected\r\n");
                    if(myProfile->rate == 0 && wfaDutAgentCAPIData.progSet == eDEF_VHT && myProfile->hti == WFA_OFF)
                    {
                        bufActualSize = MAX_ETH_PAYLOAD_LEN;
                        DPRINT_INFOL(WFA_OUT, "use default ethernet payload size\r\n");
                    }
                    else 
                    {
                        // the sender can send any size of packet, using the configurable receiving buffer size makes it able to 
                        // adjust the receving buffer size
                        bufActualSize = MAX_UDP_LEN;
                        DPRINT_INFOL(WFA_OUT, "use maximum buffer size\r\n");
                    }
                    recvBuf = (char*)malloc((bufActualSize + 1) * sizeof(char));
                }
                else
                {
                    recvBuf = (char*)malloc((myProfile->pksize + 1) * sizeof(char));
                    bufActualSize = myProfile->pksize;                  
                }
              
                DPRINT_INFOL(WFA_OUT, "The packet size in profile is %d\r\n", bufActualSize); 
				
                for (;;)
				{
                    memset(recvBuf, 0, bufActualSize + 1);
					nbytes = wfaRecvFile(mySock, myStreamId, (char  *)recvBuf, bufActualSize);					
					if (nbytes==0)
					{
						DPRINT_ERR(WFA_OUT, "The connection has been gracefully closed, the return value is zero");
						break;
					}

					if(nbytes== -1)
					{
						DPRINT_ERR(WFA_OUT, "Packet recving with error %d\n", WSAGetLastError());
						break;
					}
				} /* for */

                free(recvBuf);

				my_wmm->thr_flag = 0;

#ifdef WFA_VOICE_EXT
				if(myProfile->profile == PROF_IPTV)
				{
					int j;

					gettimeofday(&currtime, NULL);

					sprintf(e2eResults, "c:\\windows\\temp\\e2e%u.txt", (unsigned int) currtime.tv_sec); 

					DPRINT_INFOL(WFA_OUT, "storage file %s to have cnt %i records\n", e2eResults, myStream->stats.rxFrames);
					e2eoutp = fopen(e2eResults, "w+");
					if(e2eoutp != NULL)
					{
						fprintf(e2eoutp, "roundtrip delay: %i\n", (int) (1000000*gtgPktRTDelay));

						for(j = 0; j<totalE2Cnt && j < (int) myStream->stats.rxFrames; j++)
						{
							tgE2EStats_t *ep = e2esp+j;
							fprintf(e2eoutp, "%i:%i:%i:%i:%i\n", ep->seqnum, ep->lsec, ep->lusec, ep->rsec, ep->rusec);
						}
						fclose(e2eoutp);
					}

					if(e2esp!= NULL)
						free(e2esp);
				}
#endif
			}
			else if (myProfile->profile == PROF_TRANSC)
			{
				int sendCount=0, rcvCount=0;
				DPRINT_INFOL(WFA_OUT, "RCV:PROF_TRANSC::Packet receiving and then send back\n");

                if (myProfile->transProtoType == SOCK_TYPE_TCP)
                {
                    if ((mySock = wfaCreateTCPServSockImpl(myProfile->dipaddr, myProfile->dport)) == WFA_FAILURE)
                    {
                        DPRINT_ERR(WFA_ERR, "can't create server socket to %s and port %i\r\n", myProfile->dipaddr, myProfile->dport);                       
                        continue;
                    }
                }
                else
                {
                    if ((mySock = wfaCreateSock(myProfile->transProtoType, myProfile->sipaddr, myProfile->sport)) == WFA_FAILURE)
                    {
                        my_wmm->thr_flag = 0;
					    DPRINT_ERR(WFA_ERR, "PROF_TRANSC::Packet receiving and then send back; ERR on wfaCreateUDPSock \n");
					    continue;
                    }
                }

                if (myProfile->transProtoType == SOCK_TYPE_TCP)
                {
                    if (wfaAcceptTCPConn(mySock) == WFA_FAILURE)
                    {
                        DPRINT_ERR(WFA_ERR, "can't accept the connection\r\n");
                        closesocket(mySock);
                        continue;
                    }
                }
                else
                {
                    if (wfaConnectToPeer(myProfile->transProtoType, mySock, myProfile->dipaddr, myProfile->dport) == WFA_FAILURE)
                    {
                       DPRINT_ERR(WFA_ERR, "can't connect to %s and port %i\r\n", myProfile->dipaddr, myProfile->dport);
                       closesocket(mySock);
                       continue;
                    }
                }

				wfaTGWMMData.tgSockfds[myStream->tblidx] = mySock;              
				wfaTGWMMData.gtgTransac = myStreamId;

				if (getsockopt(mySock, SOL_SOCKET, SO_RCVBUF, (char*)&iOptVal, &iOptLen) != SOCKET_ERROR) 
				{
					DPRINT_INFO(WFA_OUT, "Current SO_RCVBUF Value: %ld\n", iOptVal);                  
				}
				else
				{
					DPRINT_INFOL(WFA_OUT, "before setting SO_RECVBUF, getsockopt failed with error %d\n", WSAGetLastError());
				}

				iOptVal = iOptVal*40;

				if (setsockopt(mySock, SOL_SOCKET, SO_RCVBUF, (char*)&iOptVal, iOptLen) != SOCKET_ERROR)
				{
					DPRINT_INFO(WFA_OUT, "setsockopt SO_RCVBUF Value OK: %ld\n", iOptVal);  
				}
				else
				{
					DPRINT_INFOL(WFA_OUT, "setsockopt SO_RCVBUF failed with error %d  Value: %ld\n", WSAGetLastError(), iOptVal);
				}

				while(wfaTGWMMData.gtgTransac != 0)
				{
					BYTE recvBuf[MAX_RCV_BUF_LEN+1];
					int nbytes = 0, respLen = 0;
					memset(recvBuf,0, sizeof(recvBuf));

					if(mySock != -1)
					{
						int i = 0;

						nbytes = wfaRecvFile(mySock, wfaTGWMMData.gtgTransac, (char *)recvBuf, sizeof(recvBuf));
						if(nbytes <= 0)
						{
							Sleep(3);
							i++;
							if (i < 10)
								continue;
							else
							{
								DPRINT_INFO(WFA_OUT, "RCV:PROF_TRANSC::ERR on wfaRecvFile, try 10 times failed\n");
								break;
							}
						}
						else
						{   
                            rcvCount++;
						}
					}
					else
					{
						DPRINT_INFO(WFA_OUT, "RCV:PROF_TRANSC: wfaRecvFile Error on socket open\n");
						break;
					}

					if(wfaSendShortFile(mySock, wfaTGWMMData.gtgTransac, recvBuf, nbytes, respBuf, &respLen) == -1)
					{
						if(wfaCtrlSend(wfaDutAgentData.gxcSockfd, (BYTE *)respBuf, respLen) != respLen)
						{
							DPRINT_WARNING(WFA_WNG, "RCV;PROF_TRANSC::wfaCtrlSend Error for wfaSendShortFile \n");
						}
					}
					else
					{
						sendCount++;
					}
				} /*   while(gtgTransac != 0) */
				DPRINT_INFOL(WFA_OUT, "wfa_wmm_thread::RCV;PROF_TRANSC::while loop done sendCount=%d rcvCount=%d\n",sendCount, rcvCount);
			}

			break;
		default:
			DPRINT_ERR(WFA_ERR, "Unknown covered case\n");
		}
	}
}

#endif

#endif
